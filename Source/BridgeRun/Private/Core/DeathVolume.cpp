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

    // Box Component ����
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

    // ���������� ������ �̺�Ʈ ó��
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

    // �ù� ĳ���� ó��
    if (ACitizen* Citizen = Cast<ACitizen>(OtherActor))
    {
        // Ʈ���Ǹ� ��� �ִ� ��� ó��
        HandleCitizenWithTrophy(Citizen);

        // ĳ���� ��� ó��
        Citizen->MulticastHandleDeath();

        // ������ Ÿ�̸� ����
        ScheduleRespawn(Citizen);
    }
    // Ʈ���� ��ü�� ������ ���
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

    // ������ ��ġ���� ���� ����� ���� ��ġ ã��
    FVector SafeLocation = FindSafeLocationNearby(Citizen->GetActorLocation());

    // Ʈ���� ���
    Trophy->Drop();
    Citizen->HeldTrophy = nullptr;

    // Ʈ���� ������
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

    // ������ ��ġ���� ���� ����� NavMesh ���� ã��
    if (NavSystem->ProjectPointToNavigation(
        DeathLocation,
        ResultLocation,
        FVector(5000.0f, 5000.0f, 5000.0f)))  // �˻� ������ ����� ũ��
    {
        return ResultLocation.Location;
    }

    // �����ϸ� �⺻ ���� ���
    return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
}

FVector ADeathVolume::GetRespawnLocation() const
{
    TArray<AActor*> FoundTargetPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundTargetPoints);

    if (FoundTargetPoints.Num() > 0)
    {
        // �����ϰ� Ÿ�� ����Ʈ ����
        int32 RandomIndex = FMath::RandRange(0, FoundTargetPoints.Num() - 1);
        return FoundTargetPoints[RandomIndex]->GetActorLocation();
    }

    // Ÿ�� ����Ʈ�� ������ ������ �⺻ ��ġ ���
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

    // NavMesh ���� ������ ��ġ ã��
    if (NavSystem->GetRandomReachablePointInRadius(OriginLocation, SafeRespawnRadius, ResultLocation))
    {
        return ResultLocation.Location;
    }

    return FVector(DeathLocation.X, DeathLocation.Y, TrophyRespawnHeight);
}