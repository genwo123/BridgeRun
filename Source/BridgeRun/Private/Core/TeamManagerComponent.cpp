// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/TeamManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Characters/Citizen.h"
#include <BridgeRunPlayerState.h>

UTeamManagerComponent::UTeamManagerComponent()
{
    // 네트워크 복제 활성화
    SetIsReplicatedByDefault(true);

    // 기본 설정
    ActiveTeamCount = 2;
    MaxTeams = 4;
    MaxPlayersPerTeam = 8;

    // 팀 정보 초기화
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

    // 팀 색상 초기화
    InitializeTeamColors();

    // 팀 활성화 상태 업데이트
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
    // 팀 정보 초기화
    if (TeamInfo.Num() >= MaxTeams)
    {
        // 팀 0: Red
        TeamInfo[0].TeamName = TEXT("Red");
        TeamInfo[0].TeamColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

        // 팀 1: Blue
        TeamInfo[1].TeamName = TEXT("Blue");
        TeamInfo[1].TeamColor = FLinearColor(0.0f, 0.0f, 1.0f, 1.0f);

        // 팀 2: Yellow
        TeamInfo[2].TeamName = TEXT("Yellow");
        TeamInfo[2].TeamColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

        // 팀 3: Green
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

    // 최적의 팀 ID 계산 (현재는 간단하게 플레이어 번호에 따라 할당)
    int32 OptimalTeamID = GetOptimalTeamForTeam();

    // 활성화된 팀인지 확인
    if (OptimalTeamID >= ActiveTeamCount || !TeamActive[OptimalTeamID])
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot assign player to team: Team %d is not active"), OptimalTeamID);
        return;
    }

    // 팀 인원 증가
    TeamInfo[OptimalTeamID].PlayerCount++;

    // 플레이어-팀 맵핑 저장
    PlayerTeamMap.Add(PlayerController, OptimalTeamID);

    // PlayerState에 팀 ID 설정
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (PC && PC->PlayerState)
    {
        ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState);
        if (BridgeRunPS)
        {
            BridgeRunPS->SetTeamID(OptimalTeamID);

            // 디버그 로그
            UE_LOG(LogTemp, Log, TEXT("Player assigned to team %d (%s)"),
                OptimalTeamID, *TeamInfo[OptimalTeamID].TeamName);
        }
    }

    // 플레이어 캐릭터에 직접 팀 색상 적용
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (PlayerPawn)
    {
        ACitizen* Character = Cast<ACitizen>(PlayerPawn);
        if (Character)
        {
            Character->TeamID = OptimalTeamID;
            Character->MulticastSetTeamMaterial(OptimalTeamID);
            UE_LOG(LogTemp, Warning, TEXT("Applied team color %d to character"), OptimalTeamID);
        }
    }
}

int32 UTeamManagerComponent::GetOptimalTeamForTeam() const
{
    // 임시 방편: 플레이어 수에 따라 순환 방식으로 팀 할당
    return PlayerTeamMap.Num() % ActiveTeamCount;
}

//int32 UTeamManagerComponent::GetOptimalTeamForTeam() const
//{
//    TArray<int32> TeamCounts;
//    TeamCounts.SetNum(ActiveTeamCount);
//    TeamCounts.Init(0, ActiveTeamCount);
//
//    // 각 팀의 현재 플레이어 수 계산
//    for (const auto& PlayerTeamPair : PlayerTeamMap)
//    {
//        int32 TeamID = PlayerTeamPair.Value;
//        if (TeamID >= 0 && TeamID < ActiveTeamCount)
//        {
//            TeamCounts[TeamID]++;
//        }
//    }
//
//    // 가장 인원이 적은 팀 찾기
//    int32 OptimalTeam = 0;
//    int32 MinPlayers = TeamCounts[0];
//
//    for (int32 i = 1; i < ActiveTeamCount; i++)
//    {
//        if (TeamCounts[i] < MinPlayers)
//        {
//            MinPlayers = TeamCounts[i];
//            OptimalTeam = i;
//        }
//    }
//
//    // 로그로 각 팀의 현재 인원 출력
//    FString TeamCountsStr;
//    for (int32 i = 0; i < ActiveTeamCount; i++)
//    {
//        TeamCountsStr += FString::Printf(TEXT("Team%d: %d players, "), i, TeamCounts[i]);
//    }
//
//    UE_LOG(LogTemp, Warning, TEXT("Team counts: %s"), *TeamCountsStr);
//    UE_LOG(LogTemp, Warning, TEXT("Selecting optimal team: %d with %d players"), OptimalTeam, MinPlayers);
//
//    return OptimalTeam;
//}

void UTeamManagerComponent::RespawnPlayerInTeam(AController* PlayerController, int32 TeamID)
{
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (!PC)
        return;

    // 게임모드 가져오기
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (!GameMode)
        return;

    // 팀 ID에 해당하는 태그를 가진 PlayerStart 찾기
    FString TeamName = TeamInfo[TeamID].TeamName;
    FString TeamTag = FString::Printf(TEXT("Team_%s"), *TeamName); // Team_Red, Team_Blue 등의 형식

    // PlayerStart를 찾기 위한 함수 사용
    AActor* StartSpot = FindPlayerStartForTeam(PC, TeamTag);

    // 적절한 PlayerStart를 찾지 못한 경우 FindPlayerStart 사용
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

        // 폰 클래스 가져오기
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

            // 팀 색상 적용 (필요한 경우)
            if (ACitizen* Character = Cast<ACitizen>(NewPawn))
            {
                // Character->SetTeamColor(TeamInfo[TeamID].TeamColor);
            }
        }
    }
}

// TeamManagerComponent.cpp 파일에 다음 함수 추가
AActor* UTeamManagerComponent::FindPlayerStartForTeam(AController* Controller, const FString& TeamTag)
{
    // 모든 PlayerStart를 찾아서 해당 팀 태그를 가진 것 찾기
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    TArray<AActor*> TeamPlayerStarts;

    // 해당 팀 태그를 가진 PlayerStart만 필터링
    for (AActor* Start : PlayerStarts)
    {
        if (Start->ActorHasTag(FName(*TeamTag)))
        {
            TeamPlayerStarts.Add(Start);
        }
    }

    // 해당 팀의 PlayerStart가 있으면 랜덤하게 하나 선택
    if (TeamPlayerStarts.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, TeamPlayerStarts.Num() - 1);
        return TeamPlayerStarts[Index];
    }

    // 없으면 null 반환
    return nullptr;
}

void UTeamManagerComponent::SetActiveTeamCount(int32 Count)
{
    if (Count < 2 || Count > 4)
    {
        UE_LOG(LogTemp, Warning, TEXT("활성 팀 수는 2-4 사이여야 합니다. 요청된 값: %d"), Count);
        return;
    }

    // 이미 같은 값이면 변경하지 않음
    if (ActiveTeamCount == Count)
    {
        return;
    }

    // 활성 팀 수 변경
    ActiveTeamCount = Count;

    // 팀 활성화 상태 업데이트
    UpdateTeamActiveStatus();

    // 플레이어 재배정 필요
    // IsMatchInProgress 대신 항상 재배정 또는 게임 상태를 직접 확인
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (GameMode)
    {
        // 게임 상태 직접 확인 방식 (BridgeRunGameMode 사용 시)
        // ABridgeRunGameMode* BridgeRunGameMode = Cast<ABridgeRunGameMode>(GameMode);
        // if (BridgeRunGameMode && BridgeRunGameMode->CurrentGameState == EGameState::WaitingToStart)
        // {
        //     ReallocatePlayersToTeams();
        // }

        // 또는 항상 재배정 (간단한 방식)
        ReallocatePlayersToTeams();
    }
}

void UTeamManagerComponent::UpdateTeamActiveStatus()
{
    // 활성화 상태 배열 크기 확인
    if (TeamActive.Num() < MaxTeams)
    {
        TeamActive.SetNum(MaxTeams);
    }

    // 활성화 상태 업데이트
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

    return FLinearColor::White; // 기본 색상
}

FString UTeamManagerComponent::GetTeamName(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < TeamInfo.Num())
    {
        return TeamInfo[TeamID].TeamName;
    }

    return TEXT("Unknown"); // 기본 이름
}

int32 UTeamManagerComponent::GetPlayerTeamID(AController* PlayerController) const
{
    if (!PlayerController)
    {
        return -1;
    }

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
        APlayerController* Controller = *It;
        if (Controller)
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
        APlayerController* PC = Cast<APlayerController>(Controller);
        if (PC)
        {
            AssignPlayerToTeam(PC);
        }
    }
}

bool UTeamManagerComponent::RequestTeamChange(AController* PlayerController, int32 RequestedTeamID)
{
    if (!PlayerController || RequestedTeamID < 0 || RequestedTeamID >= TeamInfo.Num())
    {
        return false;
    }

    // 활성화된 팀인지 확인
    if (RequestedTeamID >= ActiveTeamCount || !TeamActive[RequestedTeamID])
    {
        return false;
    }

    // 현재 팀 확인
    int32 CurrentTeamID = GetPlayerTeamID(PlayerController);
    if (CurrentTeamID == RequestedTeamID)
    {
        return true; // 이미 해당 팀에 소속
    }

    // 팀 인원 제한 확인
    if (TeamInfo[RequestedTeamID].PlayerCount >= MaxPlayersPerTeam)
    {
        return false; // 팀 인원 초과
    }

    // 기존 팀에서 제거
    if (CurrentTeamID >= 0 && CurrentTeamID < TeamInfo.Num())
    {
        TeamInfo[CurrentTeamID].PlayerCount--;
    }

    // 새 팀에 추가
    TeamInfo[RequestedTeamID].PlayerCount++;

    // 플레이어-팀 맵핑 업데이트
    PlayerTeamMap.Add(PlayerController, RequestedTeamID);

    // 플레이어의 PlayerState에 팀 ID 설정
    APlayerController* PC = Cast<APlayerController>(PlayerController);
    if (PC && PC->PlayerState)
    {
        // UE 4.27에서는 SetTeamID 함수가 없으므로 태그를 사용
        PC->PlayerState->Tags.Empty();
        PC->PlayerState->Tags.Add(FName(*FString::Printf(TEXT("Team_%d"), RequestedTeamID)));
    }

    // 플레이어 리스폰 처리
    RespawnPlayerInTeam(PlayerController, RequestedTeamID);

    return true;
}