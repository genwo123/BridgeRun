// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // 기본값 설정
    CurrentPhase = EGamePhase::Lobby;
    PhaseTimeRemaining = 0.0f;
    CurrentRoundNumber = 1;

    // 팀 데이터는 초기화만 하고, 실제 팀은 나중에 동적으로 추가
    TeamVictoryPoints.Empty();
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 네트워크 복제할 변수들 등록
    DOREPLIFETIME(ABridgeRunGameState, CurrentPhase);
    DOREPLIFETIME(ABridgeRunGameState, PhaseTimeRemaining);
    DOREPLIFETIME(ABridgeRunGameState, CurrentRoundNumber);
    DOREPLIFETIME(ABridgeRunGameState, TeamVictoryPoints);
}

// === Setter 함수들 ===

void ABridgeRunGameState::SetCurrentPhase(EGamePhase NewPhase)
{
    if (HasAuthority()) // 서버에서만 실행
    {
        CurrentPhase = NewPhase;
        UE_LOG(LogTemp, Log, TEXT("Game Phase changed to: %d"), (int32)NewPhase);
    }
}

void ABridgeRunGameState::SetPhaseTimeRemaining(float NewTime)
{
    if (HasAuthority())
    {
        PhaseTimeRemaining = FMath::Max(0.0f, NewTime); // 음수 방지
    }
}

void ABridgeRunGameState::SetCurrentRoundNumber(int32 NewRound)
{
    if (HasAuthority())
    {
        CurrentRoundNumber = FMath::Max(1, NewRound); // 최소 1라운드
        UE_LOG(LogTemp, Log, TEXT("Round changed to: %d"), CurrentRoundNumber);
    }
}

// === UI 표시용 함수들 ===

FString ABridgeRunGameState::GetFormattedTime() const
{
    if (PhaseTimeRemaining < 0.0f) return TEXT("00:00"); // 안전장치

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
    case EGamePhase::StrategyTime:
        return TEXT("Strategy Time");
    case EGamePhase::RoundPlaying:
        return TEXT("Round Playing");
    case EGamePhase::RoundEnd:
        return TEXT("Round End");
    case EGamePhase::GameEnd:
        return TEXT("Game End");
    default:
        return TEXT("Lobby");
    }
}

// === 팀 점수 관련 함수들 ===

void ABridgeRunGameState::UpdateTeamScore(int32 TeamID, int32 NewScore)
{
    if (!HasAuthority()) return;

    // 해당 TeamID를 가진 팀 찾기
    for (FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            Team.CurrentRoundScore = FMath::Max(0, NewScore); // 음수 방지
            UE_LOG(LogTemp, Log, TEXT("Team %d score updated to: %d"), TeamID, NewScore);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Team %d not found for score update"), TeamID);
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

// === 팀 관리 함수들 ===

void ABridgeRunGameState::InitializeTeams(const TArray<int32>& ActiveTeamIDs)
{
    if (!HasAuthority()) return;

    // 기존 팀 데이터 초기화
    TeamVictoryPoints.Empty();
    TeamVictoryPoints.Reserve(ActiveTeamIDs.Num());

    // 활성화된 팀만 추가
    for (int32 TeamID : ActiveTeamIDs)
    {
        FTeamVictoryData NewTeam;
        NewTeam.TeamID = TeamID;
        NewTeam.CurrentRoundScore = 0;
        NewTeam.TotalVictoryPoints = 0;
        TeamVictoryPoints.Add(NewTeam);

        UE_LOG(LogTemp, Log, TEXT("Team %d initialized"), TeamID);
    }

    // 네트워크 업데이트 강제
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
    case 0: return FSlateColor(FLinearColor::Red);        // 빨강
    case 1: return FSlateColor(FLinearColor::Blue);       // 파랑
    case 2: return FSlateColor(FLinearColor::Yellow);     // 노랑
    case 3: return FSlateColor(FLinearColor::Green);      // 초록
    default: return FSlateColor(FLinearColor::White);
    }
}

bool ABridgeRunGameState::IsTeamActive(int32 TeamID) const
{
    // TeamVictoryPoints 배열에 해당 TeamID가 있는지 확인
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

// === 순위 관련 함수들 ===

TArray<int32> ABridgeRunGameState::GetTeamRankings() const
{
    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;

    // 승점 순으로 정렬 (내림차순)
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
    return (RankIndex != INDEX_NONE) ? RankIndex + 1 : 0; // 0-based를 1-based로 변환
}

bool ABridgeRunGameState::IsGameTied() const
{
    if (TeamVictoryPoints.Num() < 2)
        return false;

    // 최고 승점 찾기
    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    // 최고 승점을 가진 팀이 몇 개인지 확인
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
    if (TeamVictoryPoints.Num() == 0)
        return WinningTeams;

    // 최고 승점 찾기
    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    // 최고 승점을 가진 모든 팀 찾기
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
    if (MyRank == 0)
    {
        return TEXT("Rank Unknown");
    }

    // 동점 확인
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

void ABridgeRunGameState::CalculateRoundVictoryPoints()
{
    if (!HasAuthority())
        return;

    // 현재 라운드 점수를 기준으로 팀 정렬
    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;
    SortedTeams.Sort([](const FTeamVictoryData& A, const FTeamVictoryData& B) {
        return A.CurrentRoundScore > B.CurrentRoundScore;
        });

    // 브릿지런 승점 시스템: 1등(+5), 2등(+2), 3등(-1), 4등(-2)
    TArray<int32> VictoryPointsTable = { 5, 2, -1, -2 };

    for (int32 i = 0; i < SortedTeams.Num(); i++)
    {
        int32 TeamID = SortedTeams[i].TeamID;
        int32 PointsToAdd = (i < VictoryPointsTable.Num()) ? VictoryPointsTable[i] : -2;

        // 해당 팀의 승점 업데이트
        for (FTeamVictoryData& Team : TeamVictoryPoints)
        {
            if (Team.TeamID == TeamID)
            {
                Team.TotalVictoryPoints += PointsToAdd;
                Team.RoundResults.Add(i + 1); // 이번 라운드 순위 저장

                UE_LOG(LogTemp, Log, TEXT("Team %d: Round rank %d, gained %d victory points, total: %d"),
                    TeamID, i + 1, PointsToAdd, Team.TotalVictoryPoints);
                break;
            }
        }
    }

    // 네트워크 업데이트 강제
    ForceNetUpdate();

    UE_LOG(LogTemp, Log, TEXT("Round victory points calculated for %d teams"), SortedTeams.Num());
}

bool ABridgeRunGameState::ShouldEndGame() const
{
    return CurrentRoundNumber >= 3 && CurrentPhase == EGamePhase::RoundEnd;
}