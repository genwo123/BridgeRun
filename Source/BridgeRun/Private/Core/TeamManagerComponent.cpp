// Copyright BridgeRun Game, Inc. All Rights Reserved.
// �ھ� ���
#include "Core/TeamManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

// ���� �����ӿ�ũ ���
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"

// �긴���� ���� ���
#include "Characters/Citizen.h"
#include "Core/BridgeRunPlayerState.h"
#include "Core/BridgeRunGameInstance.h"

UTeamManagerComponent::UTeamManagerComponent()
{
    SetIsReplicatedByDefault(true);

    // �⺻ ����
    ActiveTeamCount = 4;
    MaxTeams = 4;
    MaxPlayersPerTeam = 3; 

    // �� ���� �ʱ�ȭ
    TeamInfo.Reserve(MaxTeams);
    for (int32 i = 0; i < MaxTeams; i++)
    {
        FTeamInfo NewTeam;
        NewTeam.TeamID = i;
        TeamInfo.Add(NewTeam);
    }

    // �� Ȱ��ȭ ���� �ʱ�ȭ
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
        // �⺻ �� ���� �� �̸� ����
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

    // �̹� ���� �Ҵ�Ǿ� �ִ��� Ȯ��
    if (PlayerTeamMap.Contains(PlayerController))
        return;

    int32 FinalTeamID = -1;

    // PlayerState���� �� ID ��������
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (PC && PC->PlayerState)
    {
        ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState);
        if (BridgeRunPS)
        {
            int32 StateTeamID = BridgeRunPS->GetTeamID();

            UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Player %s has TeamID %d"),
                *PlayerController->GetName(), StateTeamID);

            // PlayerState TeamID�� ��ȿ�ϸ� �״�� ���
            if (StateTeamID >= 0 && StateTeamID < MaxTeams)
            {
                FinalTeamID = StateTeamID;
                UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Using existing TeamID %d"), FinalTeamID);
            }
        }
    }

    // PlayerState�� �� ������ ���� ���� ���� �Ҵ�
    if (FinalTeamID < 0)
    {
        FinalTeamID = GetOptimalTeamForTeam();
        UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam: Assigned new TeamID %d"), FinalTeamID);

        // ���� �Ҵ��� ��쿡�� PlayerState ������Ʈ
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

    // ���� ��ȿ�� �˻�
    if (FinalTeamID < 0 || FinalTeamID >= MaxTeams)
    {
        UE_LOG(LogTemp, Error, TEXT("AssignPlayerToTeam: Invalid TeamID %d"), FinalTeamID);
        return;
    }

    // �� �Ҵ�
    TeamInfo[FinalTeamID].PlayerCount++;
    PlayerTeamMap.Add(PlayerController, FinalTeamID);

    // ĳ���Ϳ� ���� ����
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

    // Ȱ��ȭ�� ������ ���� �ο��� ���
    for (int32 i = 0; i < MaxTeams; i++)
    {
        if (TeamActive[i]) // Ȱ��ȭ�� ����
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
        return 0; // �⺻��
    }

    // ���� �ο��� ���� Ȱ��ȭ�� �� ã��
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

    // ����� �α�
    FString TeamCountsStr = TEXT("");
    for (int32 i = 0; i < ActiveTeamIDs.Num(); i++)
    {
        TeamCountsStr += FString::Printf(TEXT("%d:%d "), ActiveTeamIDs[i], ActiveTeamCounts[i]);
    }

    UE_LOG(LogTemp, Warning, TEXT("GetOptimalActiveTeam: Active teams [%s] -> Selected Team %d"),
        *TeamCountsStr, OptimalTeamID);

    // ��� Ȱ��ȭ�� ���� ���� á���� Ȯ��
    if (MinPlayers >= MaxPlayersPerTeam)
    {
        UE_LOG(LogTemp, Warning, TEXT("All active teams are full"));
        return -1;
    }

    return OptimalTeamID;
}

int32 UTeamManagerComponent::GetOptimalTeamForTeam() const
{
    // �� 4�� ��� ����ϴ� ���� ���� ��
    TArray<int32> TeamCounts;
    TeamCounts.SetNum(MaxTeams); // 4�� ���
    TeamCounts.Init(0, MaxTeams);

    // �� ���� ���� �÷��̾� �� ���
    for (const auto& PlayerTeamPair : PlayerTeamMap)
    {
        int32 TeamID = PlayerTeamPair.Value;
        if (TeamID >= 0 && TeamID < MaxTeams) // MaxTeams(4) ���
        {
            TeamCounts[TeamID]++;
        }
    }

    // �� ���� �ο��� ���� �� ã�� (4�� ��� ���) ��
    int32 OptimalTeam = 0;
    int32 MinPlayers = TeamCounts[0];

    for (int32 i = 1; i < MaxTeams; i++) // MaxTeams(4)���� ��ȸ
    {
        if (TeamCounts[i] < MinPlayers)
        {
            MinPlayers = TeamCounts[i];
            OptimalTeam = i;
        }
    }

    // ����� �α�
    UE_LOG(LogTemp, Warning, TEXT("GetOptimalTeam: Team counts [0:%d, 1:%d, 2:%d, 3:%d] -> Selected Team %d"),
        TeamCounts[0], TeamCounts[1], TeamCounts[2], TeamCounts[3], OptimalTeam);

    // ��� ���� MaxPlayersPerTeam�� �����ߴ��� Ȯ��
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

    // ���� �κ� �ý��ۿ����� ���� ���� ������ ó��
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
    // ���� ������ ��� �÷��̾� ��Ʈ�ѷ� ����
    TArray<AController*> AllControllers;
    for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
    {
        if (APlayerController* Controller = *It)
        {
            AllControllers.Add(Controller);
        }
    }

    // ���� �� ���� �ʱ�ȭ
    for (FTeamInfo& Team : TeamInfo)
    {
        Team.PlayerCount = 0;
    }

    // �÷��̾�-�� ���� �ʱ�ȭ
    PlayerTeamMap.Empty();

    // ��� �÷��̾ �ٽ� ���� ����
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
    // �� ��ȭ�� �α� �� ��ȿ�� �˻� ��
    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Player %s wants team %d"),
        *PlayerController->GetName(), RequestedTeamID);

    // �⺻ ��ȿ�� �˻�
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

    // ���� ������ ����
    if (CurrentTeamID >= 0 && CurrentTeamID < TeamInfo.Num())
    {
        TeamInfo[CurrentTeamID].PlayerCount--;
        UE_LOG(LogTemp, Log, TEXT("RequestTeamChange: Removed from team %d (new count: %d)"),
            CurrentTeamID, TeamInfo[CurrentTeamID].PlayerCount);
    }

    // �� ���� �߰�
    TeamInfo[RequestedTeamID].PlayerCount++;
    PlayerTeamMap.Add(PlayerController, RequestedTeamID);

    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Added to team %d (new count: %d)"),
        RequestedTeamID, TeamInfo[RequestedTeamID].PlayerCount);

    // PlayerState ������Ʈ
    if (APlayerController* PC = Cast<APlayerController>(PlayerController))
    {
        if (ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState))
        {
            BridgeRunPS->SetTeamID(RequestedTeamID);
            UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: Updated PlayerState TeamID to %d"), RequestedTeamID);
        }
    }

    // ���� �ν��Ͻ��� �� ���� ����
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetWorld()->GetGameInstance()))
    {
        FString PlayerID = PlayerController->GetName();
        FPlayerTeamInfo TempInfo(PlayerID, TEXT(""), RequestedTeamID);
        TArray<FPlayerTeamInfo> TempArray = GameInst->GetTeamInfoFromTransition();
        TempArray.Add(TempInfo);
        GameInst->SaveTeamInfoForTransition(TempArray);
    }

    // �÷��̾� ������
    RespawnPlayerInTeam(PlayerController, RequestedTeamID);

    UE_LOG(LogTemp, Warning, TEXT("RequestTeamChange: SUCCESS - Player %s moved to team %d"),
        *PlayerController->GetName(), RequestedTeamID);

    return true;
}

void UTeamManagerComponent::RespawnPlayerInTeam(AController* PlayerController, int32 TeamID)
{
    // 1. �⺻ ��ȿ�� �˻�
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

    // 2. �� ID ��ȿ�� �˻�
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

    // 3. PlayerState ������Ʈ (�����ϰ�)
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

    // 4. ���� ����Ʈ ã�� (�����ϰ�)
    FString TeamTag = FString::FromInt(TeamID);
    AActor* StartSpot = nullptr;

    // �켱 TeamManagerComponent�� �޼���� �õ�
    StartSpot = FindPlayerStartForTeam(PlayerController, TeamTag);

    // ������ GameMode���� �õ�
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

    // 5. ���� ����Ʈ ��ȿ�� �˻�
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayerInTeam: Cannot find any valid spawn point"));
        return;
    }

    // 6. ���� �� ���� (�����ϰ�)
    APawn* OldPawn = PlayerController->GetPawn();
    if (OldPawn)
    {
        OldPawn->Destroy();
    }

    // 7. �� �� ���� (�����ϰ�)
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

    // 8. Possess �� �� ���� ���� (�����ϰ�)
    if (NewPawn)
    {
        PlayerController->Possess(NewPawn);

        ACitizen* Character = Cast<ACitizen>(NewPawn);
        if (Character)
        {
            Character->TeamID = TeamID;

            // MulticastSetTeamMaterial ȣ�� �� Ȯ��
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