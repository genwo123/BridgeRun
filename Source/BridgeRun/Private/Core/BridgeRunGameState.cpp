// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // �⺻�� ����
    CurrentPhase = EGamePhase::Lobby;
    PhaseTimeRemaining = 0.0f;
    CurrentRoundNumber = 1;

    // �� �����ʹ� �ʱ�ȭ�� �ϰ�, ���� ���� ���߿� �������� �߰�
    TeamVictoryPoints.Empty();
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ��Ʈ��ũ ������ ������ ���
    DOREPLIFETIME(ABridgeRunGameState, CurrentPhase);
    DOREPLIFETIME(ABridgeRunGameState, PhaseTimeRemaining);
    DOREPLIFETIME(ABridgeRunGameState, CurrentRoundNumber);
    DOREPLIFETIME(ABridgeRunGameState, TeamVictoryPoints);
}

// === Setter �Լ��� ===

void ABridgeRunGameState::SetCurrentPhase(EGamePhase NewPhase)
{
    if (HasAuthority()) // ���������� ����
    {
        CurrentPhase = NewPhase;
        UE_LOG(LogTemp, Log, TEXT("Game Phase changed to: %d"), (int32)NewPhase);
    }
}

void ABridgeRunGameState::SetPhaseTimeRemaining(float NewTime)
{
    if (HasAuthority())
    {
        PhaseTimeRemaining = FMath::Max(0.0f, NewTime); // ���� ����
    }
}

void ABridgeRunGameState::SetCurrentRoundNumber(int32 NewRound)
{
    if (HasAuthority())
    {
        CurrentRoundNumber = FMath::Max(1, NewRound); // �ּ� 1����
        UE_LOG(LogTemp, Log, TEXT("Round changed to: %d"), CurrentRoundNumber);
    }
}

// === UI ǥ�ÿ� �Լ��� ===

FString ABridgeRunGameState::GetFormattedTime() const
{
    if (PhaseTimeRemaining < 0.0f) return TEXT("00:00"); // ������ġ

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

// === �� ���� ���� �Լ��� ===

void ABridgeRunGameState::UpdateTeamScore(int32 TeamID, int32 NewScore)
{
    if (!HasAuthority()) return;

    // �ش� TeamID�� ���� �� ã��
    for (FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TeamID == TeamID)
        {
            Team.CurrentRoundScore = FMath::Max(0, NewScore); // ���� ����
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

// === �� ���� �Լ��� ===

void ABridgeRunGameState::InitializeTeams(const TArray<int32>& ActiveTeamIDs)
{
    if (!HasAuthority()) return;

    // ���� �� ������ �ʱ�ȭ
    TeamVictoryPoints.Empty();
    TeamVictoryPoints.Reserve(ActiveTeamIDs.Num());

    // Ȱ��ȭ�� ���� �߰�
    for (int32 TeamID : ActiveTeamIDs)
    {
        FTeamVictoryData NewTeam;
        NewTeam.TeamID = TeamID;
        NewTeam.CurrentRoundScore = 0;
        NewTeam.TotalVictoryPoints = 0;
        TeamVictoryPoints.Add(NewTeam);

        UE_LOG(LogTemp, Log, TEXT("Team %d initialized"), TeamID);
    }

    // ��Ʈ��ũ ������Ʈ ����
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
    case 0: return FSlateColor(FLinearColor::Red);        // ����
    case 1: return FSlateColor(FLinearColor::Blue);       // �Ķ�
    case 2: return FSlateColor(FLinearColor::Yellow);     // ���
    case 3: return FSlateColor(FLinearColor::Green);      // �ʷ�
    default: return FSlateColor(FLinearColor::White);
    }
}

bool ABridgeRunGameState::IsTeamActive(int32 TeamID) const
{
    // TeamVictoryPoints �迭�� �ش� TeamID�� �ִ��� Ȯ��
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

// === ���� ���� �Լ��� ===

TArray<int32> ABridgeRunGameState::GetTeamRankings() const
{
    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;

    // ���� ������ ���� (��������)
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
    return (RankIndex != INDEX_NONE) ? RankIndex + 1 : 0; // 0-based�� 1-based�� ��ȯ
}

bool ABridgeRunGameState::IsGameTied() const
{
    if (TeamVictoryPoints.Num() < 2)
        return false;

    // �ְ� ���� ã��
    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    // �ְ� ������ ���� ���� �� ������ Ȯ��
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

    // �ְ� ���� ã��
    int32 HighestScore = INT32_MIN;
    for (const FTeamVictoryData& Team : TeamVictoryPoints)
    {
        if (Team.TotalVictoryPoints > HighestScore)
        {
            HighestScore = Team.TotalVictoryPoints;
        }
    }

    // �ְ� ������ ���� ��� �� ã��
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

    // ���� Ȯ��
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

    // ���� ���� ������ �������� �� ����
    TArray<FTeamVictoryData> SortedTeams = TeamVictoryPoints;
    SortedTeams.Sort([](const FTeamVictoryData& A, const FTeamVictoryData& B) {
        return A.CurrentRoundScore > B.CurrentRoundScore;
        });

    // �긴���� ���� �ý���: 1��(+5), 2��(+2), 3��(-1), 4��(-2)
    TArray<int32> VictoryPointsTable = { 5, 2, -1, -2 };

    for (int32 i = 0; i < SortedTeams.Num(); i++)
    {
        int32 TeamID = SortedTeams[i].TeamID;
        int32 PointsToAdd = (i < VictoryPointsTable.Num()) ? VictoryPointsTable[i] : -2;

        // �ش� ���� ���� ������Ʈ
        for (FTeamVictoryData& Team : TeamVictoryPoints)
        {
            if (Team.TeamID == TeamID)
            {
                Team.TotalVictoryPoints += PointsToAdd;
                Team.RoundResults.Add(i + 1); // �̹� ���� ���� ����

                UE_LOG(LogTemp, Log, TEXT("Team %d: Round rank %d, gained %d victory points, total: %d"),
                    TeamID, i + 1, PointsToAdd, Team.TotalVictoryPoints);
                break;
            }
        }
    }

    // ��Ʈ��ũ ������Ʈ ����
    ForceNetUpdate();

    UE_LOG(LogTemp, Log, TEXT("Round victory points calculated for %d teams"), SortedTeams.Num());
}

bool ABridgeRunGameState::ShouldEndGame() const
{
    return CurrentRoundNumber >= 3 && CurrentPhase == EGamePhase::RoundEnd;
}