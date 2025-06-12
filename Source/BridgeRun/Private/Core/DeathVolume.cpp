// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Core/DeathVolume.h"
#include "Components/BoxComponent.h"
#include "Characters/Citizen.h"
#include "Item/Item_Trophy.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/PlayerStart.h"

ADeathVolume::ADeathVolume()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Box Component 설정
    DeathBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DeathBox"));
    RootComponent = DeathBox;

    DeathBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DeathBox->SetCollisionObjectType(ECC_WorldStatic);
    DeathBox->SetCollisionResponseToAllChannels(ECR_Overlap);
    DeathBox->SetBoxExtent(FVector(5000.0f, 5000.0f, 50.0f));
}

void ADeathVolume::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 오버랩 이벤트 처리
    if (HasAuthority())
    {
        DeathBox->OnComponentBeginOverlap.AddDynamic(this, &ADeathVolume::HandleOverlap);
    }
}

void ADeathVolume::HandleOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    // 시민 캐릭터 처리
    if (ACitizen* Citizen = Cast<ACitizen>(OtherActor))
    {
        // 트로피를 들고 있는 경우 처리
        HandleCitizenWithTrophy(Citizen);

        // 캐릭터 사망 처리
        Citizen->MulticastHandleDeath();

        // 리스폰 타이머 설정
        ScheduleRespawn(Citizen);
    }
    // 트로피 자체가 낙사한 경우
    else if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor))
    {
        FVector SafeLocation = FindSafeLocationNearby(Trophy->GetActorLocation());
        Trophy->ServerTryRespawn(SafeLocation);
    }
}

void ADeathVolume::HandleCitizenWithTrophy(ACitizen* Citizen)
{
    if (!Citizen || !Citizen->HeldTrophy)
        return;

    AItem_Trophy* Trophy = Citizen->HeldTrophy;

    // 떨어진 위치에서 가장 가까운 안전 위치 찾기
    FVector SafeLocation = FindSafeLocationNearby(Citizen->GetActorLocation());

    // 트로피 드롭
    Trophy->Drop();
    Citizen->HeldTrophy = nullptr;

    // 트로피 리스폰
    Trophy->ServerTryRespawn(SafeLocation);
}

void ADeathVolume::ScheduleRespawn(ACitizen* Citizen)
{
    if (!IsValid(Citizen))
        return;

    FTimerHandle RespawnTimer;
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        [this, Citizen]()
        {
            if (IsValid(Citizen))
            {
                FVector SpawnLocation = GetRespawnLocation();
                Citizen->ServerRespawn(SpawnLocation);
            }
        },
        PlayerRespawnDelay,
        false
    );
}

FVector ADeathVolume::FindSafeLocationNearby(const FVector& DeathLocation) const
{
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
    }

    FNavLocation ResultLocation;

    // 떨어진 위치에서 가장 가까운 NavMesh 지점 찾기
    if (NavSystem->ProjectPointToNavigation(
        DeathLocation,
        ResultLocation,
        FVector(5000.0f, 5000.0f, 5000.0f)))  // 검색 범위를 충분히 크게
    {
        return ResultLocation.Location;
    }

    // 실패하면 기본 높이 사용
    return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
}

FVector ADeathVolume::GetRespawnLocation() const
{
    TArray<AActor*> FoundTargetPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundTargetPoints);

    if (FoundTargetPoints.Num() > 0)
    {
        // 랜덤하게 타겟 포인트 선택
        int32 RandomIndex = FMath::RandRange(0, FoundTargetPoints.Num() - 1);
        return FoundTargetPoints[RandomIndex]->GetActorLocation();
    }

    // 타겟 포인트가 없으면 설정된 기본 위치 사용
    return DefaultRespawnLocation;
}

FVector ADeathVolume::GetTrophyRespawnLocation(const FVector& DeathLocation) const
{
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
    }

    FNavLocation ResultLocation;
    FVector OriginLocation(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);

    // NavMesh 상의 안전한 위치 찾기
    if (NavSystem->GetRandomReachablePointInRadius(OriginLocation, SafeRespawnRadius, ResultLocation))
    {
        return ResultLocation.Location;
    }

    return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
}