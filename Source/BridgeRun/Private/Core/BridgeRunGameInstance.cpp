// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // �� ������
}

void UBridgeRunGameInstance::Init()
{
    // �θ� Ŭ������ Init ȣ��
    UGameInstance::Init();

    // �⺻ �� ���� �ʱ�ȭ
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    // �÷��̾� �� ���� �迭 �ʱ�ȭ
    PlayersTeamInfo.Empty();
}

void UBridgeRunGameInstance::UpdateTeamScore(int32 TeamID, int32 NewScore)
{
    if (IsValidTeamID(TeamID))
    {
        TeamScores[TeamID] = NewScore;
    }
}

void UBridgeRunGameInstance::AddTeamScore(int32 TeamID, int32 ScoreToAdd)
{
    if (IsValidTeamID(TeamID))
    {
        TeamScores[TeamID] += ScoreToAdd;
    }
}

int32 UBridgeRunGameInstance::GetTeamScore(int32 TeamID) const
{
    if (IsValidTeamID(TeamID))
    {
        return TeamScores[TeamID];
    }
    return 0;
}

int32 UBridgeRunGameInstance::GetWinningTeam() const
{
    int32 WinningTeam = -1;
    int32 HighestScore = -1;
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        if (TeamScores[i] > HighestScore)
        {
            HighestScore = TeamScores[i];
            WinningTeam = i;
        }
    }
    return WinningTeam;
}

bool UBridgeRunGameInstance::IsValidTeamID(int32 TeamID) const
{
    return (TeamID >= 0 && TeamID < NumberOfTeams);
}

void UBridgeRunGameInstance::SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID)
{
    // �̹� �����ϴ� �÷��̾� �������� Ȯ��
    for (int32 i = 0; i < PlayersTeamInfo.Num(); i++)
    {
        if (PlayersTeamInfo[i].PlayerID == InPlayerID)
        {
            // ���� ���� ������Ʈ
            PlayersTeamInfo[i].TeamID = InTeamID;
            UE_LOG(LogTemp, Log, TEXT("Updated team info for player %s: TeamID=%d"), *InPlayerID, InTeamID);
            return;
        }
    }

    // �� �÷��̾� ���� �߰� - ������ ���
    PlayersTeamInfo.Add(FPlayerTeamInfo(InPlayerID, InTeamID));
    UE_LOG(LogTemp, Log, TEXT("Added new team info for player %s: TeamID=%d"), *InPlayerID, InTeamID);
}

int32 UBridgeRunGameInstance::GetPlayerTeamID(const FString& InPlayerID) const
{
    // �÷��̾� ID�� �� ���� �˻�
    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (Info.PlayerID == InPlayerID)
        {
            return Info.TeamID;
        }
    }

    // ã�� ���� ��� -1 ��ȯ
    UE_LOG(LogTemp, Warning, TEXT("Player team info not found for player %s"), *InPlayerID);
    return -1;
}

void UBridgeRunGameInstance::ClearPlayersTeamInfo()
{
    PlayersTeamInfo.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all player team info"));
}

void UBridgeRunGameInstance::PrintPlayersTeamInfo() const
{
    UE_LOG(LogTemp, Log, TEXT("Players Team Info (%d entries):"), PlayersTeamInfo.Num());
    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        UE_LOG(LogTemp, Log, TEXT("Player %s: TeamID=%d"), *Info.PlayerID, Info.TeamID);
    }

    // ���� �÷��̾� �� ���
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    UE_LOG(LogTemp, Log, TEXT("Team Player Counts:"));
    for (int32 i = 0; i < TeamCounts.Num(); i++)
    {
        UE_LOG(LogTemp, Log, TEXT("Team %d: %d players"), i, TeamCounts[i]);
    }
}

TArray<int32> UBridgeRunGameInstance::GetTeamPlayerCounts() const
{
    // ���� �÷��̾� �� ���
    TArray<int32> TeamCounts;
    TeamCounts.SetNumZeroed(NumberOfTeams);

    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (IsValidTeamID(Info.TeamID))
        {
            TeamCounts[Info.TeamID]++;
        }
    }

    return TeamCounts;
}

int32 UBridgeRunGameInstance::GetActiveTeamsCount() const
{
    // Ȱ�� �� �� ��� (1�� �̻��� �÷��̾ �ִ� ��)
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    int32 ActiveTeams = 0;

    for (int32 Count : TeamCounts)
    {
        if (Count > 0)
        {
            ActiveTeams++;
        }
    }

    return ActiveTeams;
}

bool UBridgeRunGameInstance::HasMinimumTeams(int32 MinTeamCount) const
{
    // Ȱ�� �� ���� �ּ� �䱸ġ �̻����� Ȯ��
    int32 ActiveTeams = GetActiveTeamsCount();
    UE_LOG(LogTemp, Log, TEXT("Active Teams: %d, Minimum Required: %d"), ActiveTeams, MinTeamCount);
    return ActiveTeams >= MinTeamCount;
}

bool UBridgeRunGameInstance::AreTeamsBalanced() const
{
    // ���� �÷��̾� �� Ȯ��
    TArray<int32> TeamCounts = GetTeamPlayerCounts();

    // Ȱ�� �� ���� �ִ�/�ּ� �÷��̾� �� Ȯ��
    int32 ActiveTeams = 0;
    int32 MinPlayers = INT_MAX;
    int32 MaxPlayers = 0;

    for (int32 Count : TeamCounts)
    {
        if (Count > 0)
        {
            ActiveTeams++;
            MinPlayers = FMath::Min(MinPlayers, Count);
            MaxPlayers = FMath::Max(MaxPlayers, Count);
        }
    }

    // Ȱ�� ���� �ּ� �䱸ġ �̻��̰�, �� �� �÷��̾� �� ���̰� ��� ���� ������ Ȯ��
    bool IsBalanced = (ActiveTeams >= MinimumTeamsRequired) &&
        (MinPlayers != INT_MAX) &&  // �ּ� 1�� �̻��� �÷��̾ �ִ��� Ȯ��
        (MaxPlayers - MinPlayers <= MaxPlayerDifferenceAllowed);

    UE_LOG(LogTemp, Log, TEXT("Team Balance Check: ActiveTeams=%d, MinPlayers=%d, MaxPlayers=%d, IsBalanced=%d"),
        ActiveTeams, MinPlayers == INT_MAX ? 0 : MinPlayers, MaxPlayers, IsBalanced);

    return IsBalanced;
}