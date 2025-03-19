// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/TeamManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Characters/Citizen.h"
#include "BridgeRunPlayerState.h"

UTeamManagerComponent::UTeamManagerComponent()
{
    SetIsReplicatedByDefault(true);

    // �⺻ ����
    ActiveTeamCount = 2;
    MaxTeams = 4;
    MaxPlayersPerTeam = 8;

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

    // �ӽ�: �÷��̾� ���� ���� ��ȯ ������� �� �Ҵ�
    int32 OptimalTeamID = GetOptimalTeamForTeam();

    // Ȱ��ȭ�� ������ Ȯ��
    if (OptimalTeamID >= ActiveTeamCount || !TeamActive[OptimalTeamID])
        return;

    // �� �ο� ����
    TeamInfo[OptimalTeamID].PlayerCount++;

    // �÷��̾�-�� ���� ����
    PlayerTeamMap.Add(PlayerController, OptimalTeamID);

    // PlayerState�� �� ID ����
    if (APlayerController* PC = Cast<APlayerController>(PlayerController))
    {
        if (ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState))
        {
            BridgeRunPS->SetTeamID(OptimalTeamID);
        }
    }

    // �÷��̾� ĳ���Ϳ� �� ���� ����
    if (APawn* PlayerPawn = PlayerController->GetPawn())
    {
        if (ACitizen* Character = Cast<ACitizen>(PlayerPawn))
        {
            Character->TeamID = OptimalTeamID;
            Character->MulticastSetTeamMaterial(OptimalTeamID);
        }
    }
}

int32 UTeamManagerComponent::GetOptimalTeamForTeam() const
{
    // �ӽ�: �÷��̾� ���� ���� ��ȯ ������� �� �Ҵ�
    return PlayerTeamMap.Num() % ActiveTeamCount;

    // ���� ����: �ο� ������ ���ߴ� ����
    /*
    TArray<int32> TeamCounts;
    TeamCounts.SetNum(ActiveTeamCount);
    TeamCounts.Init(0, ActiveTeamCount);

    // �� ���� ���� �÷��̾� �� ���
    for (const auto& PlayerTeamPair : PlayerTeamMap)
    {
        int32 TeamID = PlayerTeamPair.Value;
        if (TeamID >= 0 && TeamID < ActiveTeamCount)
        {
            TeamCounts[TeamID]++;
        }
    }

    // ���� �ο��� ���� �� ã��
    int32 OptimalTeam = 0;
    int32 MinPlayers = TeamCounts[0];

    for (int32 i = 1; i < ActiveTeamCount; i++)
    {
        if (TeamCounts[i] < MinPlayers)
        {
            MinPlayers = TeamCounts[i];
            OptimalTeam = i;
        }
    }

    return OptimalTeam;
    */
}

void UTeamManagerComponent::RespawnPlayerInTeam(AController* PlayerController, int32 TeamID)
{
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (!PC)
        return;

    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (!GameMode)
        return;

    // �� ID�� �ش��ϴ� �±׸� ���� PlayerStart ã��
    FString TeamName = TeamInfo[TeamID].TeamName;
    FString TeamTag = FString::Printf(TEXT("Team_%s"), *TeamName);

    AActor* StartSpot = FindPlayerStartForTeam(PC, TeamTag);
    if (!StartSpot)
    {
        StartSpot = GameMode->FindPlayerStart(PC, FString::FromInt(TeamID));
    }

    if (StartSpot)
    {
        if (PC->GetPawn())
        {
            PC->GetPawn()->Destroy();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        UClass* PawnClass = GameMode->GetDefaultPawnClassForController(PC);
        if (!PawnClass)
            return;

        if (APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
            PawnClass,
            StartSpot->GetActorLocation(),
            StartSpot->GetActorRotation(),
            SpawnParams))
        {
            PC->Possess(NewPawn);

            // �� ���� ����
            if (ACitizen* Character = Cast<ACitizen>(NewPawn))
            {
                Character->TeamID = TeamID;
                Character->MulticastSetTeamMaterial(TeamID);
            }
        }
    }
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
    // ���� �κ� �ý��ۿ��� �߿��� ���
    if (!PlayerController || RequestedTeamID < 0 || RequestedTeamID >= TeamInfo.Num())
        return false;

    if (RequestedTeamID >= ActiveTeamCount || !TeamActive[RequestedTeamID])
        return false;

    int32 CurrentTeamID = GetPlayerTeamID(PlayerController);
    if (CurrentTeamID == RequestedTeamID)
        return true;

    if (TeamInfo[RequestedTeamID].PlayerCount >= MaxPlayersPerTeam)
        return false;

    // ���� ������ ����
    if (CurrentTeamID >= 0 && CurrentTeamID < TeamInfo.Num())
    {
        TeamInfo[CurrentTeamID].PlayerCount--;
    }

    // �� ���� �߰�
    TeamInfo[RequestedTeamID].PlayerCount++;
    PlayerTeamMap.Add(PlayerController, RequestedTeamID);

    // PlayerState ������Ʈ
    if (APlayerController* PC = Cast<APlayerController>(PlayerController))
    {
        if (ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState))
        {
            BridgeRunPS->SetTeamID(RequestedTeamID);
        }
    }

    // �÷��̾� ������
    RespawnPlayerInTeam(PlayerController, RequestedTeamID);
    return true;
}