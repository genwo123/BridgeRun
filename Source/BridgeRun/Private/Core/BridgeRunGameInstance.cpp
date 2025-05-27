// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"


TWeakObjectPtr<UBridgeRunGameInstance> UBridgeRunGameInstance::Instance;

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // 빈 생성자
}

void UBridgeRunGameInstance::Init()
{
    // 부모 클래스의 Init 호출
    UGameInstance::Init();

    Instance = this;

    // 기본 값 초기화 추가
    CurrentPlayerName = TEXT("");
    PlayerNameText = FText::FromString(TEXT(""));
    SkinIndex = 0;
    bSkipLoginScreen = false;
    bHasPlayerName = false;
    bReturningFromGame = false;


    // 기본 팀 점수 초기화 (기존 코드 그대로)
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    // 플레이어 팀 정보 배열 초기화 (기존 코드 그대로)
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

    // 팀별 플레이어 수 출력 (수정된 부분)
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    for (int32 i = 0; i < TeamCounts.Num(); i++)
    {
        if (TeamCounts[i] > 0)
        {
            // GetPlayerNamesByTeam 대신 직접 구현
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
    // 저장된 값이 있으면 그걸 반환
    if (StoredTeamCounts.Num() > 0)
    {
        return StoredTeamCounts;
    }

    // 없으면 기본값 [0,0,0,0] 반환
    TArray<int32> DefaultCounts;
    DefaultCounts.SetNumZeroed(NumberOfTeams);
    return DefaultCounts;
}

void UBridgeRunGameInstance::SetTeamPlayerCounts(const TArray<int32>& NewTeamCounts)
{
 
    StoredTeamCounts = NewTeamCounts;

    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("SetTeamPlayerCounts: [%d,%d,%d,%d]"),
        NewTeamCounts.Num() > 0 ? NewTeamCounts[0] : 0,
        NewTeamCounts.Num() > 1 ? NewTeamCounts[1] : 0,
        NewTeamCounts.Num() > 2 ? NewTeamCounts[2] : 0,
        NewTeamCounts.Num() > 3 ? NewTeamCounts[3] : 0);
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
    // 자동으로 고유 ID 생성하여 플레이어 추가
    FPlayerTeamInfo NewPlayer(PlayerName, TeamID);
    PlayersTeamInfo.Add(NewPlayer);

    UE_LOG(LogTemp, Log, TEXT("Added player: %s (ID: %s) to Team %d"),
        *PlayerName, *NewPlayer.PlayerID, TeamID);

    return NewPlayer.PlayerID; // 생성된 고유 ID 반환
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



// BridgeRunGameInstance.cpp에 추가할 함수들

// =========================
// 기존 SavePlayerTeamInfo 함수 수정 (이름 파라미터 추가)
// =========================
void UBridgeRunGameInstance::SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID, const FString& InPlayerName)
{
    // 이미 존재하는 플레이어 정보인지 확인
    for (int32 i = 0; i < PlayersTeamInfo.Num(); i++)
    {
        if (PlayersTeamInfo[i].PlayerID == InPlayerID)
        {
            // 기존 정보 업데이트
            PlayersTeamInfo[i].TeamID = InTeamID;
            if (!InPlayerName.IsEmpty()) // 이름이 제공된 경우에만 업데이트
            {
                PlayersTeamInfo[i].PlayerName = InPlayerName;
            }
            UE_LOG(LogTemp, Log, TEXT("Updated player info: %s -> Team %d, Name: %s"),
                *InPlayerID, InTeamID, *PlayersTeamInfo[i].PlayerName);
            return;
        }
    }

    // 새 플레이어 정보 추가 - 새로운 생성자 사용
    PlayersTeamInfo.Add(FPlayerTeamInfo(InPlayerID, InPlayerName, InTeamID));
    UE_LOG(LogTemp, Log, TEXT("Added new player info: %s -> Team %d, Name: %s"),
        *InPlayerID, InTeamID, *InPlayerName);
}

// =========================
// 새로 추가할 함수들
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
