// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    CurrentPhase = EGamePhase::Lobby;
    PhaseTimeRemaining = 0.0f;
    CurrentRoundNumber = 1;

    RoomName = TEXT("BridgeRun Room");
    MaxPlayersCount = 12;
    CurrentPlayersCount = 0;
    bGameStarted = false;

    TeamVictoryPoints.Empty();
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 공통 게임 상태
    DOREPLIFETIME(ABridgeRunGameState, CurrentPhase);
    DOREPLIFETIME(ABridgeRunGameState, PhaseTimeRemaining);
    DOREPLIFETIME(ABridgeRunGameState, CurrentRoundNumber);

    // 로비 시스템
    DOREPLIFETIME(ABridgeRunGameState, RoomName);
    DOREPLIFETIME(ABridgeRunGameState, MaxPlayersCount);
    DOREPLIFETIME(ABridgeRunGameState, CurrentPlayersCount);

    // 게임플레이 시스템
    DOREPLIFETIME(ABridgeRunGameState, TeamVictoryPoints);
    DOREPLIFETIME(ABridgeRunGameState, bGameStarted);
}

// =========================
// 공통 게임 상태
// =========================

void ABridgeRunGameState::SetCurrentPhase(EGamePhase NewPhase)
{
    if (HasAuthority())
    {
        CurrentPhase = NewPhase;
    }
}

void ABridgeRunGameState::SetPhaseTimeRemaining(float NewTime)
{
    if (HasAuthority())
    {
        PhaseTimeRemaining = FMath::Max(0.0f, NewTime);
    }
}

void ABridgeRunGameState::SetCurrentRoundNumber(int32 NewRound)
{
    if (HasAuthority())
    {
        CurrentRoundNumber = FMath::Max(1, NewRound);
    }
}

// =========================
// 로비 시스템
// =========================

void ABridgeRunGameState::SetRoomName(const FString& NewRoomName)
{
    if (HasAuthority())
    {
        RoomName = NewRoomName;
        ForceNetUpdate();
    }
}

void ABridgeRunGameState::SetMaxPlayersCount(int32 NewMaxPlayers)
{
    if (HasAuthority())
    {
        MaxPlayersCount = FMath::Clamp(NewMaxPlayers, 2, 12);
        ForceNetUpdate();
    }
}

void ABridgeRunGameState::UpdateCurrentPlayersCount()
{
    if (HasAuthority())
    {
        CurrentPlayersCount = PlayerArray.Num();
        ForceNetUpdate();
    }
}

bool ABridgeRunGameState::AreAllNonHostPlayersReady() const
{
    int32 NonHostPlayers = 0;
    int32 ReadyNonHostPlayers = 0;

    for (APlayerState* PS : PlayerArray)
    {
        ABridgeRunPlayerState* BRPS = Cast<ABridgeRunPlayerState>(PS);
        if (BRPS && !BRPS->GetHostStatus()) // 방장이 아닌 플레이어
        {
            NonHostPlayers++;
            if (BRPS->GetReadyStatus())
            {
                ReadyNonHostPlayers++;
            }
        }
    }

    return (NonHostPlayers > 0) && (ReadyNonHostPlayers == NonHostPlayers);
}

int32 ABridgeRunGameState::GetReadyPlayersCount() const
{
    int32 ReadyCount = 0;

    for (APlayerState* PS : PlayerArray)
    {
        ABridgeRunPlayerState* BRPS = Cast<ABridgeRunPlayerState>(PS);
        if (BRPS && !BRPS->GetHostStatus() && BRPS->GetReadyStatus())
        {
            ReadyCount++;
        }
    }

    return ReadyCount;
}

int32 ABridgeRunGameState::GetTotalNonHostPlayersCount() const
{
    int32 NonHostCount = 0;

    for (APlayerState* PS : PlayerArray)
    {
        ABridgeRunPlayerState* BRPS = Cast<ABridgeRunPlayerState>(PS);
        if (BRPS && !BRPS->GetHostStatus())
        {
            NonHostCount++;
        }
    }

    return NonHostCount;
}

bool ABridgeRunGameState::CanStartGame() const
{
    return IsLobbyPhase() &&
        GetTotalNonHostPlayersCount() >= 1 &&
        AreAllNonHostPlayersReady();
}

void ABridgeRunGameState::MulticastUpdateLobbyUI_Implementation()
{
    // 모든 클라이언트에서 로비 UI 업데이트 (블루프린트에서 구현)
}

// =========================
// 게임플레이 시스템
// =========================

void ABridgeRunGameState::OnRep_GameStarted()
{
    if (bGameStarted)
    {
        BP_CreateTeamScoreWidgets();
    }
}

void ABridgeRunGameState::StartGameWithTeams(const TArray<int32>& ActiveTeamIDs)
{
    if (!HasAuthority()) return;

    InitializeTeams(ActiveTeamIDs);
    bGameStarted = true;
    OnRep_GameStarted();
    SetCurrentPhase(EGamePhase::StrategyTime);
}

void ABridgeRunGameState::InitializeTeams(const TArray<int32>& ActiveTeamIDs)
{
    if (!HasAuthority()) return;

    TeamVictoryPoints.Empty();
    TeamVictoryPoints.Reserve(ActiveTeamIDs.Num());

    for (int32 TeamID : ActiveTeamIDs)
    {
        FTeamVictoryData NewTeam;
        NewTeam.TeamID = TeamID;
        NewTeam.CurrentRoundScore = 0;
        NewTeam.TotalVictoryPoints = 0;
        TeamVictoryPoints.Add(NewTeam);
    }

    ForceNetUpdate();
}

FString ABridgeRunGameState::GetTeamName(int32 TeamID) const
{
    switch (TeamID)
    {
    case 0: return TEXT("Red Team");
    case 1: return TEXT("Blue Team");
    case 2: return TEXT("Yellow Team");
    case 3: return TEXT("Green Team");
    default: return FString::Printf(TEXT("Team %d"), TeamID);
    }
}

FSlateColor ABridgeRunGameState::GetTeamColor(int32 TeamID) const
{
    switch (TeamID)
    {
    case 0: return FSlateColor(FLinearColor::Red);
    case 1: return FSlateColor(FLinearColor::Blue);
    case 2: return FSlateColor(FLinearColor::Yellow);
    case 3: return FSlateColor(FLinearColor::Green);
    default: return FSlateColor(FLinearColor::White);
    }
}

bool ABridgeRunGameState::IsTeamActive(int32 TeamID) const
{
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            return true;
        }
    }
    return false;
}

int32 ABridgeRunGameState::GetActiveTeamCount() const
{
    return TeamVictoryPoints.Num();
}

TArray<int32> ABridgeRunGameState::GetActiveTeamIDs() const
{
    TArray<int32> ActiveIDs;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        ActiveIDs.Add(Team.TeamID);
    }
    return ActiveIDs;
}

void ABridgeRunGameState::UpdateTeamScore(int32 TeamID, int32 NewScore)
{
    if (!HasAuthority()) return;

    for (FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            Team.CurrentRoundScore = FMath::Max(0, NewScore);
            return;
        }
    }
}

int32 ABridgeRunGameState::GetTeamCurrentScore(int32 TeamID) const
{
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            return Team.CurrentRoundScore;
        }
    }
    return 0;
}

int32 ABridgeRunGameState::GetTeamVictoryPoints(int32 TeamID) const
{
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            return Team.TotalVictoryPoints;
        }
    }
    return 0;
}

TArray<int32> ABridgeRunGameState::GetTeamRankings() const
{
    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;

    SortedTeams.Sort([](const FTeamVictoryData& A, const FTeamVictoryData& B) {
        return A.TotalVictoryPoints > B.TotalVictoryPoints;
        });

    TArray<int32> Rankings;
    for (const FTeamVictoryData& Team : SortedTeams)
    {
        Rankings.Add(Team.TeamID);
    }

    return Rankings;
}

int32 ABridgeRunGameState::GetTeamRank(int32 TeamID) const
{
    TArray<int32> Rankings = GetTeamRankings();
    int32 RankIndex = Rankings.Find(TeamID);
    return (RankIndex != INDEX_NONE) ? RankIndex + 1 : 0;
}

bool ABridgeRunGameState::IsGameTied() const
{
    if (TeamVictoryPoints.Num() < 2) return false;

    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    int32 TiedCount = 0;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints == HighestScore)
        {
            TiedCount++;
        }
    }

    return TiedCount > 1;
}

TArray<int32> ABridgeRunGameState::GetWinningTeams() const
{
    TArray<int32> WinningTeams;
    if (TeamVictoryPoints.Num() == 0) return WinningTeams;

    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints == HighestScore)
        {
            WinningTeams.Add(Team.TeamID);
        }
    }

    return WinningTeams;
}

FString ABridgeRunGameState::GetRankDisplayText(int32 TeamID) const
{
    int32 MyRank = GetTeamRank(TeamID);
    if (MyRank == 0) return TEXT("Rank Unknown");

    int32 MyPoints = GetTeamVictoryPoints(TeamID);
    int32 SameRankCount = 0;

    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints == MyPoints)
        {
            SameRankCount++;
        }
    }

    if (SameRankCount > 1)
    {
        return FString::Printf(TEXT("%d Place (Tied)"), MyRank);
    }
    else
    {
        return FString::Printf(TEXT("%d Place"), MyRank);
    }
}

void ABridgeRunGameState::CalculateRoundVictoryPoints()
{
    if (!HasAuthority()) return;

    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;
    SortedTeams.Sort([](const FTeamVictoryData& A, const FTeamVictoryData& B) {
        return A.CurrentRoundScore > B.CurrentRoundScore;
        });

    TArray<int32> VictoryPointsTable = { 5, 2, -1, -2 };

    for (int32 i = 0; i < SortedTeams.Num(); i++)
    {
        int32 TeamID = SortedTeams[i].TeamID;
        int32 PointsToAdd = (i < VictoryPointsTable.Num()) ? VictoryPointsTable[i] : -2;

        for (FTeamVictoryData& Team : TeamVictoryPoints)
        {
            if (Team.TeamID == TeamID)
            {
                Team.TotalVictoryPoints += PointsToAdd;
                Team.RoundResults.Add(i + 1);
                break;
            }
        }
    }

    ForceNetUpdate();
}

bool ABridgeRunGameState::ShouldEndGame() const
{
    return CurrentRoundNumber >= 3 && CurrentPhase == EGamePhase::RoundEnd;
}

void ABridgeRunGameState::MulticastGameOverUI_Implementation()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (PC->IsLocalController())
        {
            ShowGameOverUIEvent();
        }
    }
}

// =========================
// UI 표시용 함수들
// =========================

FString ABridgeRunGameState::GetFormattedTime() const
{
    if (PhaseTimeRemaining < 0.0f) return TEXT("00:00");

    int32 Minutes = (int32)PhaseTimeRemaining / 60;
    int32 Seconds = (int32)PhaseTimeRemaining % 60;
    return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

FString ABridgeRunGameState::GetRoundText() const
{
    return FString::Printf(TEXT("Round %d"), CurrentRoundNumber);
}

FString ABridgeRunGameState::GetPhaseText() const
{
    switch (CurrentPhase)
    {
    case EGamePhase::Lobby:
        return TEXT("Lobby");
    case EGamePhase::StrategyTime:
        return TEXT("Strategy Time");
    case EGamePhase::RoundPlaying:
        return TEXT("Round Playing");
    case EGamePhase::RoundEnd:
        return TEXT("Round End");
    case EGamePhase::GameEnd:
        return TEXT("Game End");
    default:
        return TEXT("Unknown");
    }
}