// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Zones/ItemSpawnZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Item/Item.h"             
#include "Item/Item_Gun.h"         
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AItemSpawnZone::AItemSpawnZone()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpawnVolume->SetGenerateOverlapEvents(true);
    SpawnVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SpawnVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    SpawnVolume->SetIsReplicated(true);

    SpawnVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapBegin);
    SpawnVolume->OnComponentEndOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapEnd);

    SpawnInterval = 5.0f;
    MaxItemCount = 10;
    CurrentItemCount = 0;
}

void AItemSpawnZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItemSpawnZone, CurrentItemCount);
    DOREPLIFETIME(AItemSpawnZone, SpawnedItems);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        StartSpawnTimer();
    }
}

void AItemSpawnZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (HasAuthority())
    {
        CleanupSpawnedItems();
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimer);
    }
}

void AItemSpawnZone::StartSpawnTimer()
{
    if (!HasAuthority()) return;

    if (UWorld* World = GetWorld())
    {
        if (!World->GetTimerManager().IsTimerActive(SpawnTimer))
        {
            World->GetTimerManager().SetTimer(SpawnTimer, this, &AItemSpawnZone::ServerSpawnItem, SpawnInterval, true);
        }
    }
}

void AItemSpawnZone::ServerSpawnItem_Implementation()
{
    if (!HasAuthority() || !ValidateSpawnConditions())
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    int32 RandomIndex = FMath::RandRange(0, ItemsToSpawn.Num() - 1);
    TSubclassOf<AItem> ItemToSpawn = ItemsToSpawn[RandomIndex];
    if (!ItemToSpawn) return;

    FVector SpawnLocation = GetRandomPointInVolume();
    FRotator SpawnRotation = FRotator::ZeroRotator;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = this;

    if (AItem* SpawnedItem = World->SpawnActor<AItem>(ItemToSpawn, SpawnLocation, SpawnRotation, SpawnParams))
    {
        SpawnedItem->SetReplicates(true);
        SpawnedItem->SetReplicateMovement(true);

        if (AItem_Gun* Gun = Cast<AItem_Gun>(SpawnedItem))
        {
            Gun->InitializeAmmo();
            if (Gun->CollisionComponent)
            {
                Gun->CollisionComponent->SetGenerateOverlapEvents(false);
            }
        }

        SpawnedItems.Add(SpawnedItem);
        CurrentItemCount++;

        MulticastOnItemSpawned(SpawnedItem);
        UpdateSpawnState();
    }
}

void AItemSpawnZone::MulticastOnItemSpawned_Implementation(AItem* SpawnedItem)
{
    if (!SpawnedItem) return;

    if (SpawnedItem->CollisionComponent)
    {
        SpawnedItem->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AItemSpawnZone::MulticastOnItemRemoved_Implementation(AItem* RemovedItem)
{
    if (!RemovedItem) return;

    SpawnedItems.Remove(RemovedItem);
    if (CurrentItemCount > 0)
    {
        CurrentItemCount--;
    }
}

void AItemSpawnZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    if (AItem* Item = Cast<AItem>(OtherActor))
    {
        if (!SpawnedItems.Contains(Item))
        {
            SpawnedItems.Add(Item);
            CurrentItemCount++;
            UpdateSpawnState();
        }
    }
}

void AItemSpawnZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!HasAuthority()) return;

    if (AItem* Item = Cast<AItem>(OtherActor))
    {
        if (SpawnedItems.Contains(Item))
        {
            MulticastOnItemRemoved(Item);
            UpdateSpawnState();
        }
    }
}

void AItemSpawnZone::OnRep_CurrentItemCount()
{
    UpdateSpawnState();
}

FVector AItemSpawnZone::GetRandomPointInVolume()
{
    FVector ExtentMax = SpawnVolume->Bounds.BoxExtent;
    return UKismetMathLibrary::RandomPointInBoundingBox(
        SpawnVolume->GetComponentLocation(),
        ExtentMax);
}

bool AItemSpawnZone::ValidateSpawnConditions() const
{
    return CurrentItemCount < MaxItemCount && ItemsToSpawn.Num() > 0;
}

void AItemSpawnZone::CleanupSpawnedItems()
{
    for (AItem* Item : SpawnedItems)
    {
        if (Item)
        {
            Item->Destroy();
        }
    }
    SpawnedItems.Empty();
    CurrentItemCount = 0;
}

void AItemSpawnZone::UpdateSpawnState()
{
    if (!HasAuthority()) return;

    // 타이머 상태 업데이트
    if (CurrentItemCount < MaxItemCount)
    {
        StartSpawnTimer();
    }
    else
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(SpawnTimer);
        }
    }

    // 네트워크 상태 강제 업데이트
    ForceNetUpdate();
}