// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

TWeakObjectPtr<UBridgeRunGameInstance> UBridgeRunGameInstance::Instance;

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    PhotonClient = nullptr;
}

void UBridgeRunGameInstance::Init()
{
    UGameInstance::Init();
    Instance = this;

    // 기본 플레이어 정보 초기화
    CurrentPlayerName = TEXT("");
    PlayerNameText = FText::FromString(TEXT(""));
    SkinIndex = 0;
    bSkipLoginScreen = false;
    bHasPlayerName = false;
    bReturningFromGame = false;

    // 팀 점수 초기화
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    // 플레이어 팀 정보 배열 초기화
    PlayersTeamInfo.Empty();

    // Photon 클라이언트 초기화
    InitializePhotonClient();

    UE_LOG(LogTemp, Warning, TEXT("BridgeRun GameInstance Initialized with Photon Support"));

    // 에디터가 아닌 배포 버전에서만 자동 연결
#if !WITH_EDITOR
    if (bAutoConnectOnStartup)
    {
        GetWorld()->GetTimerManager().SetTimer(
            PhotonUpdateTimerHandle,
            this,
            &UBridgeRunGameInstance::ConnectToPhoton,
            1.0f,
            false
        );
    }
#endif
}

void UBridgeRunGameInstance::Shutdown()
{
    UE_LOG(LogTemp, Warning, TEXT("BridgeRun GameInstance Shutting Down"));

    CleanupPhotonClient();

    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(PhotonUpdateTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(PhotonUpdateTimerHandle);
    }

    Super::Shutdown();
}

// =========================
// Photon 네트워크 관리
// =========================

void UBridgeRunGameInstance::InitializePhotonClient()
{
    try
    {
        PhotonClient = new ExitGames::LoadBalancing::Client(*this, PhotonAppID.GetCharArray().GetData(), "1.0");

        if (PhotonClient)
        {
            PhotonStatusMessage = "Initialized";
            UE_LOG(LogTemp, Warning, TEXT("Photon Client Initialized Successfully"));

            GetWorld()->GetTimerManager().SetTimer(
                PhotonUpdateTimerHandle,
                this,
                &UBridgeRunGameInstance::UpdatePhotonClient,
                0.05f,
                true
            );
        }
        else
        {
            PhotonStatusMessage = "Failed to Initialize";
            UE_LOG(LogTemp, Error, TEXT("Failed to create Photon Client"));
        }
    }
    catch (const std::exception& e)
    {
        PhotonStatusMessage = "Initialization Error";
        UE_LOG(LogTemp, Error, TEXT("Photon Client Initialization Exception: %s"), ANSI_TO_TCHAR(e.what()));
    }
}

void UBridgeRunGameInstance::CleanupPhotonClient()
{
    if (PhotonClient)
    {
        try
        {
            if (bIsInRoom)
            {
                PhotonClient->opLeaveRoom();
            }

            if (bIsPhotonConnected)
            {
                PhotonClient->disconnect();
            }

            delete PhotonClient;
            PhotonClient = nullptr;

            bIsPhotonConnected = false;
            bIsInRoom = false;
            PhotonStatusMessage = "Disconnected";

            UE_LOG(LogTemp, Warning, TEXT("Photon Client Cleaned Up"));
        }
        catch (const std::exception& e)
        {
            UE_LOG(LogTemp, Error, TEXT("Photon Cleanup Exception: %s"), ANSI_TO_TCHAR(e.what()));
        }
    }
}

void UBridgeRunGameInstance::UpdatePhotonClient()
{
    if (PhotonClient)
    {
        try
        {
            PhotonClient->service();
        }
        catch (const std::exception& e)
        {
            UE_LOG(LogTemp, Error, TEXT("Photon Service Exception: %s"), ANSI_TO_TCHAR(e.what()));
        }
    }
}

void UBridgeRunGameInstance::ConnectToPhoton()
{
    if (!PhotonClient)
    {
        InitializePhotonClient();
        if (!PhotonClient)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot connect: Photon Client not initialized"));
            return;
        }
    }

    if (bIsPhotonConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already connected to Photon"));
        return;
    }

    try
    {
        UE_LOG(LogTemp, Warning, TEXT("Connecting to Photon Server..."));
        PhotonStatusMessage = "Connecting...";

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                TEXT("Connecting to Photon Server..."));
        }

        bool ConnectResult = PhotonClient->connect();

        if (ConnectResult)
        {
            UE_LOG(LogTemp, Warning, TEXT("Photon connection request sent"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to send Photon connection request"));
            PhotonStatusMessage = "Connection Failed";
        }
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Photon Connect Exception: %s"), ANSI_TO_TCHAR(e.what()));
        PhotonStatusMessage = "Connection Error";
    }
}

void UBridgeRunGameInstance::DisconnectFromPhoton()
{
    if (!PhotonClient || !bIsPhotonConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not connected to Photon"));
        return;
    }

    try
    {
        UE_LOG(LogTemp, Warning, TEXT("Disconnecting from Photon..."));
        PhotonClient->disconnect();
        PhotonStatusMessage = "Disconnecting...";
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Photon Disconnect Exception: %s"), ANSI_TO_TCHAR(e.what()));
    }
}

void UBridgeRunGameInstance::CreateOrJoinRoom(const FString& RoomName)
{
    if (!bIsPhotonConnected)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create/join room: Not connected to Photon"));
        return;
    }

    if (bIsInRoom)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already in a room"));
        return;
    }

    try
    {
        FString SafeRoomName = RoomName.IsEmpty() ? "BridgeRun_Main" : RoomName;

        UE_LOG(LogTemp, Warning, TEXT("Creating or joining room: %s"), *SafeRoomName);

        ExitGames::LoadBalancing::RoomOptions RoomOptions;
        RoomOptions.setMaxPlayers(12);
        RoomOptions.setIsVisible(true);
        RoomOptions.setIsOpen(true);

        bool Result = PhotonClient->opJoinOrCreateRoom(
            SafeRoomName.GetCharArray().GetData(),
            RoomOptions
        );

        if (Result)
        {
            CurrentRoomName = SafeRoomName;
            PhotonStatusMessage = "Joining Room...";
            UE_LOG(LogTemp, Warning, TEXT("Room join/create request sent"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to send room join/create request"));
        }
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Room Join/Create Exception: %s"), ANSI_TO_TCHAR(e.what()));
    }
}

void UBridgeRunGameInstance::LeaveRoom()
{
    if (!bIsInRoom)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not in any room"));
        return;
    }

    try
    {
        UE_LOG(LogTemp, Warning, TEXT("Leaving room..."));
        PhotonClient->opLeaveRoom();
        PhotonStatusMessage = "Leaving Room...";
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Room Leave Exception: %s"), ANSI_TO_TCHAR(e.what()));
    }
}

bool UBridgeRunGameInstance::IsConnectedToPhoton() const
{
    return bIsPhotonConnected;
}

bool UBridgeRunGameInstance::IsInRoom() const
{
    return bIsInRoom;
}

FString UBridgeRunGameInstance::GetCurrentRoomName() const
{
    return CurrentRoomName;
}

int32 UBridgeRunGameInstance::GetRoomPlayerCount() const
{
    if (PhotonClient && bIsInRoom)
    {
        try
        {
            return PhotonClient->getCurrentlyJoinedRoom().getPlayerCount();
        }
        catch (const std::exception& e)
        {
            UE_LOG(LogTemp, Error, TEXT("Get Player Count Exception: %s"), ANSI_TO_TCHAR(e.what()));
        }
    }
    return 0;
}

void UBridgeRunGameInstance::SetPhotonAppID(const FString& NewAppID)
{
    PhotonAppID = NewAppID;
    UE_LOG(LogTemp, Warning, TEXT("Photon App ID set to: %s"), *PhotonAppID);
}

FString UBridgeRunGameInstance::GetPhotonStatusMessage() const
{
    return PhotonStatusMessage;
}

// =========================
// Photon 콜백 함수들
// =========================

void UBridgeRunGameInstance::OnPhotonConnected()
{
    bIsPhotonConnected = true;
    PhotonStatusMessage = "Connected";

    UE_LOG(LogTemp, Warning, TEXT("Connected to Photon Server Successfully!"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            TEXT("Connected to Photon Server!"));
    }
}

void UBridgeRunGameInstance::OnPhotonDisconnected()
{
    bIsPhotonConnected = false;
    bIsInRoom = false;
    PhotonStatusMessage = "Disconnected";
    CurrentRoomName = "";

    UE_LOG(LogTemp, Warning, TEXT("Disconnected from Photon Server"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
            TEXT("Disconnected from Photon"));
    }
}

void UBridgeRunGameInstance::OnPhotonJoinedRoom()
{
    bIsInRoom = true;
    PhotonStatusMessage = "In Room";

    UE_LOG(LogTemp, Warning, TEXT("Joined room: %s"), *CurrentRoomName);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("Joined room: %s"), *CurrentRoomName));
    }
}

void UBridgeRunGameInstance::OnPhotonLeftRoom()
{
    bIsInRoom = false;
    PhotonStatusMessage = "Connected";
    CurrentRoomName = "";

    UE_LOG(LogTemp, Warning, TEXT("Left room"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
            TEXT("Left room"));
    }
}

void UBridgeRunGameInstance::OnPhotonError(const FString& ErrorMessage)
{
    PhotonStatusMessage = FString::Printf(TEXT("Error: %s"), *ErrorMessage);

    UE_LOG(LogTemp, Error, TEXT("Photon Error: %s"), *ErrorMessage);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
            FString::Printf(TEXT("Photon Error: %s"), *ErrorMessage));
    }
}

// =========================
// 플레이어 관리 함수
// =========================

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

UBridgeRunGameInstance* UBridgeRunGameInstance::GetInstance()
{
    return Instance.Get();
}

// =========================
// 팀 점수 관리 함수
// =========================

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

// =========================
// 플레이어 정보 관리 함수
// =========================

FString UBridgeRunGameInstance::AddPlayerInfo(const FString& PlayerName, int32 TeamID)
{
    FPlayerTeamInfo NewPlayer(PlayerName, TeamID);
    PlayersTeamInfo.Add(NewPlayer);

    UE_LOG(LogTemp, Log, TEXT("Added player: %s (ID: %s) to Team %d"),
        *PlayerName, *NewPlayer.PlayerID, TeamID);

    return NewPlayer.PlayerID;
}

void UBridgeRunGameInstance::SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID, const FString& InPlayerName)
{
    for (int32 i = 0; i < PlayersTeamInfo.Num(); i++)
    {
        if (PlayersTeamInfo[i].PlayerID == InPlayerID)
        {
            PlayersTeamInfo[i].TeamID = InTeamID;
            if (!InPlayerName.IsEmpty())
            {
                PlayersTeamInfo[i].PlayerName = InPlayerName;
            }
            UE_LOG(LogTemp, Log, TEXT("Updated player info: %s -> Team %d, Name: %s"),
                *InPlayerID, InTeamID, *PlayersTeamInfo[i].PlayerName);
            return;
        }
    }

    PlayersTeamInfo.Add(FPlayerTeamInfo(InPlayerID, InPlayerName, InTeamID));
    UE_LOG(LogTemp, Log, TEXT("Added new player info: %s -> Team %d, Name: %s"),
        *InPlayerID, InTeamID, *InPlayerName);
}

int32 UBridgeRunGameInstance::GetPlayerTeamID(const FString& InPlayerID) const
{
    for (const FPlayerTeamInfo& Info : PlayersTeamInfo)
    {
        if (Info.PlayerID == InPlayerID)
        {
            return Info.TeamID;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Player team info not found for player %s"), *InPlayerID);
    return -1;
}

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

void UBridgeRunGameInstance::ClearPlayersTeamInfo()
{
    PlayersTeamInfo.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all player team info"));
}

void UBridgeRunGameInstance::PrintPlayersTeamInfo() const
{
    TArray<int32> TeamCounts = GetTeamPlayerCounts();
    for (int32 i = 0; i < TeamCounts.Num(); i++)
    {
        if (TeamCounts[i] > 0)
        {
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

// =========================
// 팀 균형 관리 함수
// =========================

TArray<int32> UBridgeRunGameInstance::GetTeamPlayerCounts() const
{
    if (StoredTeamCounts.Num() > 0)
    {
        return StoredTeamCounts;
    }

    TArray<int32> DefaultCounts;
    DefaultCounts.SetNumZeroed(NumberOfTeams);
    return DefaultCounts;
}

void UBridgeRunGameInstance::SetTeamPlayerCounts(const TArray<int32>& NewTeamCounts)
{
    StoredTeamCounts = NewTeamCounts;

    UE_LOG(LogTemp, Warning, TEXT("SetTeamPlayerCounts: [%d,%d,%d,%d]"),
        NewTeamCounts.Num() > 0 ? NewTeamCounts[0] : 0,
        NewTeamCounts.Num() > 1 ? NewTeamCounts[1] : 0,
        NewTeamCounts.Num() > 2 ? NewTeamCounts[2] : 0,
        NewTeamCounts.Num() > 3 ? NewTeamCounts[3] : 0);
}

int32 UBridgeRunGameInstance::GetActiveTeamsCount() const
{
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
    int32 ActiveTeams = GetActiveTeamsCount();
    UE_LOG(LogTemp, Log, TEXT("Active Teams: %d, Minimum Required: %d"), ActiveTeams, MinTeamCount);
    return ActiveTeams >= MinTeamCount;
}

bool UBridgeRunGameInstance::AreTeamsBalanced() const
{
    TArray<int32> TeamCounts = GetTeamPlayerCounts();

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

    bool IsBalanced = (ActiveTeams >= MinimumTeamsRequired) &&
        (MinPlayers != INT_MAX) &&
        (MaxPlayers - MinPlayers <= MaxPlayerDifferenceAllowed);

    UE_LOG(LogTemp, Log, TEXT("Team Balance Check: ActiveTeams=%d, MinPlayers=%d, MaxPlayers=%d, IsBalanced=%d"),
        ActiveTeams, MinPlayers == INT_MAX ? 0 : MinPlayers, MaxPlayers, IsBalanced);

    return IsBalanced;
}