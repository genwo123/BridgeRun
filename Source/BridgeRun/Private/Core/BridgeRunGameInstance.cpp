// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // 빈 생성자
}

void UBridgeRunGameInstance::Init()
{
    // 부모 클래스의 Init 호출
    UGameInstance::Init();

    // 기본 팀 점수 초기화
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    // 플레이어 팀 정보 배열 초기화
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
    // 이미 존재하는 플레이어 정보인지 확인
    for (int32 i = 0; i < PlayersTeamInfo.Num(); i++)
    {
        if (PlayersTeamInfo[i].PlayerID == InPlayerID)
        {
            // 기존 정보 업데이트
            PlayersTeamInfo[i].TeamID = InTeamID;
            UE_LOG(LogTemp, Log, TEXT("Updated team info for player %s: TeamID=%d"), *InPlayerID, InTeamID);
            return;
        }
    }

    // 새 플레이어 정보 추가 - 생성자 사용
    PlayersTeamInfo.Add(FPlayerTeamInfo(InPlayerID, InTeamID));
    UE_LOG(LogTemp, Log, TEXT("Added new team info for player %s: TeamID=%d"), *InPlayerID, InTeamID);
}

int32 UBridgeRunGameInstance::GetPlayerTeamID(const FString& InPlayerID) const
{
    // 플레이어 ID로 팀 정보 검색
    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (Info.PlayerID == InPlayerID)
        {
            return Info.TeamID;
        }
    }

    // 찾지 못한 경우 -1 반환
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

    // 팀별 플레이어 수 출력
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    UE_LOG(LogTemp, Log, TEXT("Team Player Counts:"));
    for (int32 i = 0; i < TeamCounts.Num(); i++)
    {
        UE_LOG(LogTemp, Log, TEXT("Team %d: %d players"), i, TeamCounts[i]);
    }
}

TArray<int32> UBridgeRunGameInstance::GetTeamPlayerCounts() const
{
    // 팀별 플레이어 수 계산
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
    // 활성 팀 수 계산 (1명 이상의 플레이어가 있는 팀)
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
    // 활성 팀 수가 최소 요구치 이상인지 확인
    int32 ActiveTeams = GetActiveTeamsCount();
    UE_LOG(LogTemp, Log, TEXT("Active Teams: %d, Minimum Required: %d"), ActiveTeams, MinTeamCount);
    return ActiveTeams >= MinTeamCount;
}

bool UBridgeRunGameInstance::AreTeamsBalanced() const
{
    // 팀별 플레이어 수 확인
    TArray<int32> TeamCounts = GetTeamPlayerCounts();

    // 활성 팀 수와 최대/최소 플레이어 수 확인
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

    // 활성 팀이 최소 요구치 이상이고, 팀 간 플레이어 수 차이가 허용 범위 내인지 확인
    bool IsBalanced = (ActiveTeams >= MinimumTeamsRequired) &&
        (MinPlayers != INT_MAX) &&  // 최소 1명 이상의 플레이어가 있는지 확인
        (MaxPlayers - MinPlayers <= MaxPlayerDifferenceAllowed);

    UE_LOG(LogTemp, Log, TEXT("Team Balance Check: ActiveTeams=%d, MinPlayers=%d, MaxPlayers=%d, IsBalanced=%d"),
        ActiveTeams, MinPlayers == INT_MAX ? 0 : MinPlayers, MaxPlayers, IsBalanced);

    return IsBalanced;
}