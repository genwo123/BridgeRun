// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"


TWeakObjectPtr<UBridgeRunGameInstance> UBridgeRunGameInstance::Instance;

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // �� ������
}

void UBridgeRunGameInstance::Init()
{
    // �θ� Ŭ������ Init ȣ��
    UGameInstance::Init();

    Instance = this;

    // �⺻ �� �ʱ�ȭ �߰�
    CurrentPlayerName = TEXT("");
    PlayerNameText = FText::FromString(TEXT(""));
    SkinIndex = 0;
    bSkipLoginScreen = false;
    bHasPlayerName = false;
    bReturningFromGame = false;


    // �⺻ �� ���� �ʱ�ȭ (���� �ڵ� �״��)
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    // �÷��̾� �� ���� �迭 �ʱ�ȭ (���� �ڵ� �״��)
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

    // ���� �÷��̾� �� ��� (������ �κ�)
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    for (int32 i = 0; i < TeamCounts.Num(); i++)
    {
        if (TeamCounts[i] > 0)
        {
            // GetPlayerNamesByTeam ��� ���� ����
            TArray<FString> TeamNames;
            for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
            {
                if (Info.TeamID == i && !Info.PlayerName.IsEmpty())
                {
                    TeamNames.Add(Info.PlayerName);
                }
            }

            FString NamesString = FString::Join(TeamNames, TEXT(", "));
        }
    }
}

TArray<int32> UBridgeRunGameInstance::GetTeamPlayerCounts() const
{
    // ����� ���� ������ �װ� ��ȯ
    if (StoredTeamCounts.Num() > 0)
    {
        return StoredTeamCounts;
    }

    // ������ �⺻�� [0,0,0,0] ��ȯ
    TArray<int32> DefaultCounts;
    DefaultCounts.SetNumZeroed(NumberOfTeams);
    return DefaultCounts;
}

void UBridgeRunGameInstance::SetTeamPlayerCounts(const TArray<int32>& NewTeamCounts)
{
 
    StoredTeamCounts = NewTeamCounts;

    // ����� �α�
    UE_LOG(LogTemp, Warning, TEXT("SetTeamPlayerCounts: [%d,%d,%d,%d]"),
        NewTeamCounts.Num() > 0 ? NewTeamCounts[0] : 0,
        NewTeamCounts.Num() > 1 ? NewTeamCounts[1] : 0,
        NewTeamCounts.Num() > 2 ? NewTeamCounts[2] : 0,
        NewTeamCounts.Num() > 3 ? NewTeamCounts[3] : 0);
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

void UBridgeRunGameInstance::SetCurrentPlayerName(const FString& NewPlayerName)
{
    CurrentPlayerName = NewPlayerName;
    PlayerNameText = FText::FromString(NewPlayerName);
    bHasPlayerName = true;
    UE_LOG(LogTemp, Log, TEXT("Current player name set to: %s"), *CurrentPlayerName);
}

FString UBridgeRunGameInstance::GetCurrentPlayerName() const
{
    return CurrentPlayerName;
}

FText UBridgeRunGameInstance::GetCurrentPlayerNameAsText() const
{
    return PlayerNameText;
}

FString UBridgeRunGameInstance::AddPlayerInfo(const FString& PlayerName, int32 TeamID)
{
    // �ڵ����� ���� ID �����Ͽ� �÷��̾� �߰�
    FPlayerTeamInfo NewPlayer(PlayerName, TeamID);
    PlayersTeamInfo.Add(NewPlayer);

    UE_LOG(LogTemp, Log, TEXT("Added player: %s (ID: %s) to Team %d"),
        *PlayerName, *NewPlayer.PlayerID, TeamID);

    return NewPlayer.PlayerID; // ������ ���� ID ��ȯ
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



// BridgeRunGameInstance.cpp�� �߰��� �Լ���

// =========================
// ���� SavePlayerTeamInfo �Լ� ���� (�̸� �Ķ���� �߰�)
// =========================
void UBridgeRunGameInstance::SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID, const FString& InPlayerName)
{
    // �̹� �����ϴ� �÷��̾� �������� Ȯ��
    for (int32 i = 0; i < PlayersTeamInfo.Num(); i++)
    {
        if (PlayersTeamInfo[i].PlayerID == InPlayerID)
        {
            // ���� ���� ������Ʈ
            PlayersTeamInfo[i].TeamID = InTeamID;
            if (!InPlayerName.IsEmpty()) // �̸��� ������ ��쿡�� ������Ʈ
            {
                PlayersTeamInfo[i].PlayerName = InPlayerName;
            }
            UE_LOG(LogTemp, Log, TEXT("Updated player info: %s -> Team %d, Name: %s"),
                *InPlayerID, InTeamID, *PlayersTeamInfo[i].PlayerName);
            return;
        }
    }

    // �� �÷��̾� ���� �߰� - ���ο� ������ ���
    PlayersTeamInfo.Add(FPlayerTeamInfo(InPlayerID, InPlayerName, InTeamID));
    UE_LOG(LogTemp, Log, TEXT("Added new player info: %s -> Team %d, Name: %s"),
        *InPlayerID, InTeamID, *InPlayerName);
}

// =========================
// ���� �߰��� �Լ���
// =========================

FString UBridgeRunGameInstance::GetPlayerNameByID(const FString& InPlayerID) const
{
    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (Info.PlayerID == InPlayerID)
        {
            return Info.PlayerName.IsEmpty() ? "Unknown Player" : Info.PlayerName;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Player name not found for ID: %s"), *InPlayerID);
    return "Unknown Player";
}



TArray<FPlayerTeamInfo> UBridgeRunGameInstance::GetAllPlayersInfo() const
{
    return PlayersTeamInfo;
}



TArray<FPlayerTeamInfo> UBridgeRunGameInstance::GetPlayersByTeam(int32 TeamID) const
{
    TArray<FPlayerTeamInfo> TeamPlayers;

    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (Info.TeamID == TeamID)
        {
            TeamPlayers.Add(Info);
        }
    }

    return TeamPlayers;
}



UBridgeRunGameInstance* UBridgeRunGameInstance::GetInstance()
{
    return Instance.Get();
}
