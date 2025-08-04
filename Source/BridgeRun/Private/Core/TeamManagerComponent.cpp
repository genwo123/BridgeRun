// Copyright BridgeRun Game, Inc. All Rights Reserved.
// 코어 헤더
#include "Core/TeamManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

// 게임 프레임워크 헤더
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"

// 브릿지런 게임 헤더
#include "Characters/Citizen.h"
#include "Core/BridgeRunPlayerState.h"
#include "Core/BridgeRunGameInstance.h"

UTeamManagerComponent::UTeamManagerComponent()
{
    SetIsReplicatedByDefault(true);

    // 기본 설정
    ActiveTeamCount = 4;
    MaxTeams = 4;
    MaxPlayersPerTeam = 3; 

    // 팀 정보 초기화
    TeamInfo.Reserve(MaxTeams);
    for (int32 i = 0; i < MaxTeams; i++)
    {
        FTeamInfo NewTeam;
        NewTeam.TeamID = i;
        TeamInfo.Add(NewTeam);
    }

    // 팀 활성화 상태 초기화
    TeamActive.SetNum(MaxTeams);
    for (int32 i = 0; i < MaxTeams; i++)
    {
        TeamActive[i] = (i < ActiveTeamCount);
    }
}

void UTeamManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeTeamColors();
    UpdateTeamActiveStatus();
}

void UTeamManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UTeamManagerComponent, TeamInfo);
    DOREPLIFETIME(UTeamManagerComponent, ActiveTeamCount);
    DOREPLIFETIME(UTeamManagerComponent, TeamActive);
}

void UTeamManagerComponent::InitializeTeamColors()
{
    if (TeamInfo.Num() >= MaxTeams)
    {
        // 기본 팀 색상 및 이름 설정
        TeamInfo[0].TeamName = TEXT("Red");
        TeamInfo[0].TeamColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

        TeamInfo[1].TeamName = TEXT("Blue");
        TeamInfo[1].TeamColor = FLinearColor(0.0f, 0.0f, 1.0f, 1.0f);

        TeamInfo[2].TeamName = TEXT("Yellow");
        TeamInfo[2].TeamColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

        TeamInfo[3].TeamName = TEXT("Green");
        TeamInfo[3].TeamColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
}


void UTeamManagerComponent::AssignPlayerToTeam(AController* PlayerController)
{
    if (!PlayerController)
        return;

    // 이미 팀에 할당되어 있는지 확인
    if (PlayerTeamMap.Contains(PlayerController))
        return;

    int32 FinalTeamID = -1;

    // PlayerState에서 팀 ID 가져오기
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (PC && PC->PlayerState)
    {
        ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState);
        if (BridgeRunPS)
        {
            int32 StateTeamID = BridgeRunPS->GetTeamID();

            UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Player %s has TeamID %d"),
                *PlayerController->GetName(), StateTeamID);

            // PlayerState TeamID가 유효하면 그대로 사용
            if (StateTeamID >= 0 && StateTeamID < MaxTeams)
            {
                FinalTeamID = StateTeamID;
                UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Using existing TeamID %d"), FinalTeamID);
            }
        }
    }

    // PlayerState에 팀 정보가 없을 때만 새로 할당
    if (FinalTeamID < 0)
    {
        FinalTeamID = GetOptimalTeamForTeam();
        UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Assigned new TeamID %d"), FinalTeamID);

        // 새로 할당한 경우에만 PlayerState 업데이트
        if (PC && PC->PlayerState)
        {
            ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState);
            if (BridgeRunPS)
            {
                BridgeRunPS->SetTeamID(FinalTeamID);
                UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Updated PlayerState TeamID %d"), FinalTeamID);
            }
        }
    }

    // 최종 유효성 검사
    if (FinalTeamID < 0 || FinalTeamID >= MaxTeams)
    {
        UE_LOG(LogTemp, Error, TEXT("AssignPlayerToTeam: Invalid TeamID %d"), FinalTeamID);
        return;
    }

    // 팀 할당
    TeamInfo[FinalTeamID].PlayerCount++;
    PlayerTeamMap.Add(PlayerController, FinalTeamID);

    // 캐릭터에 색상 적용
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (PlayerPawn)
    {
        ACitizen* Character = Cast<ACitizen>(PlayerPawn);
        if (Character)
        {
            Character->TeamID = FinalTeamID;
            Character->MulticastSetTeamMaterial(FinalTeamID);
            UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Applied TeamID %d color"), FinalTeamID);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: COMPLETE - Player %s TeamID %d"),
        *PlayerController->GetName(), FinalTeamID);
}


int32 UTeamManagerComponent::GetOptimalActiveTeam() const
{
    TArray<int32> ActiveTeamCounts;
    TArray<int32> ActiveTeamIDs;

    // 활성화된 팀들의 현재 인원수 계산
    for (int32 i = 0; i < MaxTeams; i++)
    {
        if (TeamActive[i]) // 활성화된 팀만
        {
            int32 TeamCount = 0;
            for (const auto& PlayerTeamPair : PlayerTeamMap)
            {
                if (PlayerTeamPair.Value == i)
                {
                    TeamCount++;
                }
            }
            ActiveTeamCounts.Add(TeamCount);
            ActiveTeamIDs.Add(i);
        }
    }

    if (ActiveTeamIDs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GetOptimalActiveTeam: No active teams found!"));
        return 0; // 기본값
    }

    // 가장 인원이 적은 활성화된 팀 찾기
    int32 OptimalIndex = 0;
    int32 MinPlayers = ActiveTeamCounts[0];

    for (int32 i = 1; i < ActiveTeamCounts.Num(); i++)
    {
        if (ActiveTeamCounts[i] < MinPlayers)
        {
            MinPlayers = ActiveTeamCounts[i];
            OptimalIndex = i;
        }
    }

    int32 OptimalTeamID = ActiveTeamIDs[OptimalIndex];

    // 디버그 로그
    FString TeamCountsStr = TEXT("");
    for (int32 i = 0; i < ActiveTeamIDs.Num(); i++)
    {
        TeamCountsStr += FString::Printf(TEXT("%d:%d "), ActiveTeamIDs[i], ActiveTeamCounts[i]);
    }

    UE_LOG(LogTemp, Warning, TEXT("GetOptimalActiveTeam: Active teams [%s] -> Selected Team %d"),
        *TeamCountsStr, OptimalTeamID);

    // 모든 활성화된 팀이 가득 찼는지 확인
    if (MinPlayers >= MaxPlayersPerTeam)
    {
        UE_LOG(LogTemp, Warning, TEXT("All active teams are full"));
        return -1;
    }

    return OptimalTeamID;
}

int32 UTeamManagerComponent::GetOptimalTeamForTeam() const
{
    // ★ 4팀 모두 사용하는 균형 배정 ★
    TArray<int32> TeamCounts;
    TeamCounts.SetNum(MaxTeams); // 4팀 모두
    TeamCounts.Init(0, MaxTeams);

    // 각 팀의 현재 플레이어 수 계산
    for (const auto& PlayerTeamPair : PlayerTeamMap)
    {
        int32 TeamID = PlayerTeamPair.Value;
        if (TeamID >= 0 && TeamID < MaxTeams) // MaxTeams(4) 사용
        {
            TeamCounts[TeamID]++;
        }
    }

    // ★ 가장 인원이 적은 팀 찾기 (4팀 모두 고려) ★
    int32 OptimalTeam = 0;
    int32 MinPlayers = TeamCounts[0];

    for (int32 i = 1; i < MaxTeams; i++) // MaxTeams(4)까지 순회
    {
        if (TeamCounts[i] < MinPlayers)
        {
            MinPlayers = TeamCounts[i];
            OptimalTeam = i;
        }
    }

    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("GetOptimalTeam: Team counts [0:%d, 1:%d, 2:%d, 3:%d] -> Selected Team %d"),
        TeamCounts[0], TeamCounts[1], TeamCounts[2], TeamCounts[3], OptimalTeam);

    // 모든 팀이 MaxPlayersPerTeam에 도달했는지 확인
    if (MinPlayers >= MaxPlayersPerTeam)
    {
        UE_LOG(LogTemp, Warning, TEXT("All teams are full"));
        return -1;
    }

    return OptimalTeam;
}

AActor* UTeamManagerComponent::FindPlayerStartForTeam(AController* Controller, const FString& TeamTag)
{
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    TArray<AActor*> TeamPlayerStarts;
    for (AActor* Start : PlayerStarts)
    {
        if (Start->ActorHasTag(FName(*TeamTag)))
        {
            TeamPlayerStarts.Add(Start);
        }
    }

    if (TeamPlayerStarts.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, TeamPlayerStarts.Num() - 1);
        return TeamPlayerStarts[Index];
    }

    return nullptr;
}

void UTeamManagerComponent::SetActiveTeamCount(int32 Count)
{
    if (Count < 2 || Count > 4)
        return;

    if (ActiveTeamCount == Count)
        return;

    ActiveTeamCount = Count;
    UpdateTeamActiveStatus();

    // 향후 로비 시스템에서는 게임 시작 전에만 처리
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (GameMode)
    {
        ReallocatePlayersToTeams();
    }
}

void UTeamManagerComponent::UpdateTeamActiveStatus()
{
    if (TeamActive.Num() < MaxTeams)
    {
        TeamActive.SetNum(MaxTeams);
    }

    for (int32 i = 0; i < MaxTeams; i++)
    {
        TeamActive[i] = (i < ActiveTeamCount);
    }
}

TArray<FTeamInfo> UTeamManagerComponent::GetActiveTeams() const
{
    TArray<FTeamInfo> ActiveTeams;
    for (int32 i = 0; i < FMath::Min(ActiveTeamCount, TeamInfo.Num()); i++)
    {
        if (TeamActive[i])
        {
            ActiveTeams.Add(TeamInfo[i]);
        }
    }
    return ActiveTeams;
}

FLinearColor UTeamManagerComponent::GetTeamColor(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < TeamInfo.Num())
    {
        return TeamInfo[TeamID].TeamColor;
    }
    return FLinearColor::White;
}

FString UTeamManagerComponent::GetTeamName(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < TeamInfo.Num())
    {
        return TeamInfo[TeamID].TeamName;
    }
    return TEXT("Unknown");
}

int32 UTeamManagerComponent::GetPlayerTeamID(AController* PlayerController) const
{
    if (!PlayerController)
        return -1;

    if (const int32* FoundTeamID = PlayerTeamMap.Find(PlayerController))
    {
        return *FoundTeamID;
    }
    return -1;
}

void UTeamManagerComponent::ReallocatePlayersToTeams()
{
    // 현재 접속한 모든 플레이어 컨트롤러 수집
    TArray<AController*> AllControllers;
    for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
    {
        if (APlayerController* Controller = *It)
        {
            AllControllers.Add(Controller);
        }
    }

    // 기존 팀 정보 초기화
    for (FTeamInfo& Team : TeamInfo)
    {
        Team.PlayerCount = 0;
    }

    // 플레이어-팀 맵핑 초기화
    PlayerTeamMap.Empty();

    // 모든 플레이어를 다시 팀에 배정
    for (AController* Controller : AllControllers)
    {
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            AssignPlayerToTeam(PC);
        }
    }
}

bool UTeamManagerComponent::RequestTeamChange(AController* PlayerController, int32 RequestedTeamID)
{
    // ★ 강화된 로그 및 유효성 검사 ★
    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Player %s wants team %d"),
        *PlayerController->GetName(), RequestedTeamID);

    // 기본 유효성 검사
    if (!PlayerController || RequestedTeamID < 0 || RequestedTeamID >= TeamInfo.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("RequestTeamChange: Invalid parameters - Controller: %s, TeamID: %d"),
            PlayerController ? *PlayerController->GetName() : TEXT("NULL"), RequestedTeamID);
        return false;
    }

    if (RequestedTeamID >= ActiveTeamCount || !TeamActive[RequestedTeamID])
    {
        UE_LOG(LogTemp, Error, TEXT("RequestTeamChange: Team %d is not active (ActiveCount: %d)"),
            RequestedTeamID, ActiveTeamCount);
        return false;
    }

    int32 CurrentTeamID = GetPlayerTeamID(PlayerController);
    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Current team: %d, Requested: %d"),
        CurrentTeamID, RequestedTeamID);

    if (CurrentTeamID == RequestedTeamID)
    {
        UE_LOG(LogTemp, Log, TEXT("RequestTeamChange: Already in requested team %d"), RequestedTeamID);
        return true;
    }

    if (TeamInfo[RequestedTeamID].PlayerCount >= MaxPlayersPerTeam)
    {
        UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Team %d is full (%d/%d)"),
            RequestedTeamID, TeamInfo[RequestedTeamID].PlayerCount, MaxPlayersPerTeam);
        return false;
    }

    // 기존 팀에서 제거
    if (CurrentTeamID >= 0 && CurrentTeamID < TeamInfo.Num())
    {
        TeamInfo[CurrentTeamID].PlayerCount--;
        UE_LOG(LogTemp, Log, TEXT("RequestTeamChange: Removed from team %d (new count: %d)"),
            CurrentTeamID, TeamInfo[CurrentTeamID].PlayerCount);
    }

    // 새 팀에 추가
    TeamInfo[RequestedTeamID].PlayerCount++;
    PlayerTeamMap.Add(PlayerController, RequestedTeamID);

    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Added to team %d (new count: %d)"),
        RequestedTeamID, TeamInfo[RequestedTeamID].PlayerCount);

    // PlayerState 업데이트
    if (APlayerController* PC = Cast<APlayerController>(PlayerController))
    {
        if (ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState))
        {
            BridgeRunPS->SetTeamID(RequestedTeamID);
            UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Updated PlayerState TeamID to %d"), RequestedTeamID);
        }
    }

    // 게임 인스턴스에 팀 정보 저장
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetWorld()->GetGameInstance()))
    {
        FString PlayerID = PlayerController->GetName();
        FPlayerTeamInfo TempInfo(PlayerID, TEXT(""), RequestedTeamID);
        TArray<FPlayerTeamInfo> TempArray = GameInst->GetTeamInfoFromTransition();
        TempArray.Add(TempInfo);
        GameInst->SaveTeamInfoForTransition(TempArray);
    }

    // 플레이어 리스폰
    RespawnPlayerInTeam(PlayerController, RequestedTeamID);

    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: SUCCESS - Player %s moved to team %d"),
        *PlayerController->GetName(), RequestedTeamID);

    return true;
}

void UTeamManagerComponent::RespawnPlayerInTeam(AController* PlayerController, int32 TeamID)
{
    // 1. 기본 유효성 검사
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: PlayerController is null"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: World is null"));
        return;
    }

    // 2. 팀 ID 유효성 검사
    if (TeamID < 0 || TeamID >= TeamInfo.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Invalid TeamID: %d"), TeamID);
        return;
    }

    if (TeamInfo.Num() <= TeamID || !TeamActive.IsValidIndex(TeamID) || !TeamActive[TeamID])
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Team %d is not active"), TeamID);
        return;
    }

    // 3. PlayerState 업데이트 (안전하게)
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (PC)
    {
        APlayerState* PlayerState = PC->PlayerState;
        if (PlayerState)
        {
            ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PlayerState);
            if (BridgeRunPS)
            {
                BridgeRunPS->SetTeamID(TeamID);
                UE_LOG(LogTemp, Log, TEXT("RespawnPlayerInTeam: Set PlayerState TeamID to %d"), TeamID);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerInTeam: Failed to cast to BridgeRunPlayerState"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerInTeam: PlayerState is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Failed to cast to APlayerController"));
        return;
    }

    // 4. 스폰 포인트 찾기 (안전하게)
    FString TeamTag = FString::FromInt(TeamID);
    AActor* StartSpot = nullptr;

    // 우선 TeamManagerComponent의 메서드로 시도
    StartSpot = FindPlayerStartForTeam(PlayerController, TeamTag);

    // 없으면 GameMode에서 시도
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerInTeam: Cannot find team spawn point, trying default"));

        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (GameMode)
        {
            StartSpot = GameMode->FindPlayerStart(PlayerController, TeamTag);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: GameMode is null"));
            return;
        }
    }

    // 5. 스폰 포인트 유효성 검사
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Cannot find any valid spawn point"));
        return;
    }

    // 6. 기존 폰 정리 (안전하게)
    APawn* OldPawn = PlayerController->GetPawn();
    if (OldPawn)
    {
        OldPawn->Destroy();
    }

    // 7. 새 폰 스폰 (안전하게)
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: GameMode is null when spawning"));
        return;
    }

    TSubclassOf<APawn> PawnClass = GameMode->DefaultPawnClass;
    if (!PawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: DefaultPawnClass is null"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = PlayerController;

    APawn* NewPawn = World->SpawnActor<APawn>(
        PawnClass,
        StartSpot->GetActorLocation(),
        StartSpot->GetActorRotation(),
        SpawnParams
    );

    // 8. Possess 및 팀 색상 적용 (안전하게)
    if (NewPawn)
    {
        PlayerController->Possess(NewPawn);

        ACitizen* Character = Cast<ACitizen>(NewPawn);
        if (Character)
        {
            Character->TeamID = TeamID;

            // MulticastSetTeamMaterial 호출 전 확인
            if (Character->IsValidLowLevel() && !Character->IsPendingKill())
            {
                Character->MulticastSetTeamMaterial(TeamID);
                UE_LOG(LogTemp, Log, TEXT("RespawnPlayerInTeam: Applied team %d material to character"), TeamID);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Character is invalid after possession"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerInTeam: Failed to cast to Citizen"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Failed to spawn new pawn"));
    }
}