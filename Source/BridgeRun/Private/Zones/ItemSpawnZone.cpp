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

    // 컴포넌트 생성
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    // 고정 크기 - 에디터와 플레이 모드에서 일치하게 설정
    const FVector FixedSize(200.0f, 200.0f, 20.0f);
    SpawnVolume->SetBoxExtent(FixedSize);

    // 바닥 메시 컴포넌트
    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    FloorMesh->SetupAttachment(SpawnVolume);
    FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -5.0f));
    FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FloorMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // 스태틱 메시 에셋 로드 및 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMeshAsset.Succeeded())
    {
        FloorMesh->SetStaticMesh(PlaneMeshAsset.Object);
        // Plane 메시는 100x100 유닛이므로 볼륨 크기에 맞게 조정
        FloorMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 1.0f));
    }

    // 스폰 포인트 생성 (4x4 = 16개)
    const int32 TotalPoints = 16;
    for (int i = 0; i < TotalPoints; i++)
    {
        FString ComponentName = FString::Printf(TEXT("SpawnPoint%d"), i);
        USceneComponent* SpawnPoint = CreateDefaultSubobject<USceneComponent>(*ComponentName);
        SpawnPoint->SetupAttachment(RootComponent);
        SpawnPoints.Add(SpawnPoint);

        FSpawnPointInfo NewPointInfo;
        SpawnPointInfos.Add(NewPointInfo);
    }

    // 스폰 포인트를 4x4 그리드로 배치
    ArrangeSpawnPointsInGrid(4, 4, 80.0f, 80.0f);

    InitializeSpawnVolume();

    // 기본값 설정
    SpawnInterval = 5.0f;
    CurrentPlankCount = 0;
    CurrentTentCount = 0;
    CurrentGunCount = 0;
}

void AItemSpawnZone::ArrangeSpawnPointsInGrid(int32 Rows, int32 Columns, float SpacingX, float SpacingY)
{
    if (SpawnPoints.Num() < Rows * Columns) return;

    // 중앙 정렬을 위한 시작 위치 계산
    float StartX = -((Columns - 1) * SpacingX) / 2.0f;
    float StartY = -((Rows - 1) * SpacingY) / 2.0f;

    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Column = 0; Column < Columns; Column++)
        {
            int32 Index = Row * Columns + Column;
            if (Index < SpawnPoints.Num())
            {
                float X = StartX + Column * SpacingX;
                float Y = StartY + Row * SpacingY;
                SpawnPoints[Index]->SetRelativeLocation(FVector(X, Y, 10.0f));
            }
        }
    }
}


void AItemSpawnZone::InitializeSpawnVolume()
{
    if (!SpawnVolume) return;

    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpawnVolume->SetGenerateOverlapEvents(true);
    SpawnVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SpawnVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    SpawnVolume->SetIsReplicated(true);

    SpawnVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapBegin);
    SpawnVolume->OnComponentEndOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapEnd);
}

void AItemSpawnZone::InitializeSpawnPoints()
{
    if (SpawnPointInfos.Num() < SpawnPoints.Num())
    {
        int32 NumToAdd = SpawnPoints.Num() - SpawnPointInfos.Num();
        for (int32 i = 0; i < NumToAdd; i++)
        {
            FSpawnPointInfo NewInfo;
            SpawnPointInfos.Add(NewInfo);
        }
    }

    for (int32 i = 0; i < SpawnPointInfos.Num(); i++)
    {
        SpawnPointInfos[i].bIsOccupied = false;
        SpawnPointInfos[i].SpawnedItem = nullptr;
    }
}

void AItemSpawnZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItemSpawnZone, SpawnedItems);
    DOREPLIFETIME(AItemSpawnZone, CurrentPlankCount);
    DOREPLIFETIME(AItemSpawnZone, CurrentTentCount);
    DOREPLIFETIME(AItemSpawnZone, CurrentGunCount);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    InitializeSpawnPoints();

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
    if (!HasAuthority()) return;

    TSubclassOf<AItem> ItemToSpawn;
    if (!ValidateSpawnConditions(ItemToSpawn)) return;

    UWorld* World = GetWorld();
    if (!World || !ItemToSpawn) return;

    int32 SpawnPointIndex = FindFreeSpawnPointForItem(ItemToSpawn);
    if (SpawnPointIndex < 0) return;

    FVector SpawnLocation = SpawnPoints[SpawnPointIndex]->GetComponentLocation();
    FRotator SpawnRotation = SpawnPoints[SpawnPointIndex]->GetComponentRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = this;

    if (AItem* SpawnedItem = World->SpawnActor<AItem>(ItemToSpawn, SpawnLocation, SpawnRotation, SpawnParams))
    {
        ConfigureSpawnedItem(SpawnedItem, SpawnPointIndex);

        IncrementItemCount(ItemToSpawn);

        SpawnedItems.Add(SpawnedItem);
        MarkSpawnPointOccupied(SpawnPointIndex, SpawnedItem, true);

        MulticastOnItemSpawned(SpawnedItem, SpawnPointIndex);

        UpdateSpawnState();
    }
}

void AItemSpawnZone::ConfigureSpawnedItem(AItem* SpawnedItem, int32 SpawnPointIndex)
{
    if (!SpawnedItem) return;

    SpawnedItem->SetReplicates(true);
    SpawnedItem->SetReplicateMovement(true);

    if (SpawnedItem->MeshComponent)
    {
        SpawnedItem->MeshComponent->SetSimulatePhysics(false);
        SpawnedItem->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    if (AItem_Gun* Gun = Cast<AItem_Gun>(SpawnedItem))
    {
        Gun->InitializeAmmo();
        if (Gun->CollisionComponent)
        {
            Gun->CollisionComponent->SetGenerateOverlapEvents(true);
        }
    }
}

int32 AItemSpawnZone::FindFreeSpawnPointForItem(TSubclassOf<AItem> ItemClass) const
{
    TArray<int32> CompatiblePoints;

    for (int32 i = 0; i < SpawnPointInfos.Num(); i++)
    {
        if (SpawnPointInfos[i].bIsOccupied) continue;

        bool bIsPreferred = !SpawnPointInfos[i].PreferredItemClass ||
            SpawnPointInfos[i].PreferredItemClass == ItemClass;

        if (bIsPreferred)
        {
            return i;
        }

        CompatiblePoints.Add(i);
    }

    if (CompatiblePoints.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CompatiblePoints.Num() - 1);
        return CompatiblePoints[RandomIndex];
    }

    return -1;
}

bool AItemSpawnZone::ValidateSpawnConditions(TSubclassOf<AItem>& OutItemToSpawn) const
{
    if (ItemsToSpawn.Num() == 0) return false;

    TArray<TSubclassOf<AItem>> SpawnableItems;

    for (auto ItemClass : ItemsToSpawn)
    {
        int32 CurrentCount = GetCurrentCountForItemClass(ItemClass);
        int32 MaxCount = GetMaxCountForItemClass(ItemClass);

        if (CurrentCount < MaxCount)
        {
            SpawnableItems.Add(ItemClass);
        }
    }

    if (SpawnableItems.Num() == 0) return false;

    int32 RandomIndex = FMath::RandRange(0, SpawnableItems.Num() - 1);
    OutItemToSpawn = SpawnableItems[RandomIndex];

    return true;
}

int32 AItemSpawnZone::GetCurrentCountForItemClass(TSubclassOf<AItem> ItemClass) const
{
    if (ItemClass->GetName().Contains("Plank"))
    {
        return CurrentPlankCount;
    }
    else if (ItemClass->GetName().Contains("Tent"))
    {
        return CurrentTentCount;
    }
    else if (ItemClass->GetName().Contains("Gun"))
    {
        return CurrentGunCount;
    }

    return 0;
}

int32 AItemSpawnZone::GetMaxCountForItemClass(TSubclassOf<AItem> ItemClass) const
{
    if (ItemClass->GetName().Contains("Plank"))
    {
        return MaxPlankCount;
    }
    else if (ItemClass->GetName().Contains("Tent"))
    {
        return MaxTentCount;
    }
    else if (ItemClass->GetName().Contains("Gun"))
    {
        return MaxGunCount;
    }

    return 0;
}

void AItemSpawnZone::IncrementItemCount(TSubclassOf<AItem> ItemClass)
{
    if (ItemClass->GetName().Contains("Plank"))
    {
        CurrentPlankCount++;
    }
    else if (ItemClass->GetName().Contains("Tent"))
    {
        CurrentTentCount++;
    }
    else if (ItemClass->GetName().Contains("Gun"))
    {
        CurrentGunCount++;
    }
}

void AItemSpawnZone::DecrementItemCount(TSubclassOf<AItem> ItemClass)
{
    if (ItemClass->GetName().Contains("Plank"))
    {
        if (CurrentPlankCount > 0) CurrentPlankCount--;
    }
    else if (ItemClass->GetName().Contains("Tent"))
    {
        if (CurrentTentCount > 0) CurrentTentCount--;
    }
    else if (ItemClass->GetName().Contains("Gun"))
    {
        if (CurrentGunCount > 0) CurrentGunCount--;
    }
}

void AItemSpawnZone::MarkSpawnPointOccupied(int32 Index, AItem* Item, bool bOccupied)
{
    if (Index >= 0 && Index < SpawnPointInfos.Num())
    {
        SpawnPointInfos[Index].bIsOccupied = bOccupied;
        SpawnPointInfos[Index].SpawnedItem = bOccupied ? Item : nullptr;
    }
}

bool AItemSpawnZone::IsSpawnPointOccupied(int32 Index) const
{
    if (Index >= 0 && Index < SpawnPointInfos.Num())
    {
        return SpawnPointInfos[Index].bIsOccupied;
    }
    return true;
}

int32 AItemSpawnZone::GetSpawnPointIndexForPosition(const FVector& Position) const
{
    float ClosestDistSq = FLT_MAX;
    int32 ClosestIndex = -1;

    for (int32 i = 0; i < SpawnPoints.Num(); i++)
    {
        float DistSq = FVector::DistSquared(Position, SpawnPoints[i]->GetComponentLocation());
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

void AItemSpawnZone::MulticastOnItemSpawned_Implementation(AItem* SpawnedItem, int32 SpawnPointIndex)
{
    if (!SpawnedItem) return;

    if (SpawnedItem->CollisionComponent)
    {
        SpawnedItem->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AItemSpawnZone::MulticastOnItemRemoved_Implementation(AItem* RemovedItem, int32 SpawnPointIndex)
{
    if (!RemovedItem) return;

    SpawnedItems.Remove(RemovedItem);

    if (SpawnPointIndex >= 0 && SpawnPointIndex < SpawnPointInfos.Num())
    {
        SpawnPointInfos[SpawnPointIndex].bIsOccupied = false;
        SpawnPointInfos[SpawnPointIndex].SpawnedItem = nullptr;
    }
}

void AItemSpawnZone::OnRep_SpawnedItems()
{
    UpdateSpawnState();
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

            TSubclassOf<AItem> ItemClass = Item->GetClass();
            IncrementItemCount(ItemClass);

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
            int32 SpawnPointIndex = GetSpawnPointIndexForPosition(Item->GetActorLocation());

            if (SpawnPointIndex >= 0)
            {
                MarkSpawnPointOccupied(SpawnPointIndex, nullptr, false);
            }

            TSubclassOf<AItem> ItemClass = Item->GetClass();
            DecrementItemCount(ItemClass);

            MulticastOnItemRemoved(Item, SpawnPointIndex);
            UpdateSpawnState();
        }
    }
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
    CurrentPlankCount = 0;
    CurrentTentCount = 0;
    CurrentGunCount = 0;

    for (int32 i = 0; i < SpawnPointInfos.Num(); i++)
    {
        SpawnPointInfos[i].bIsOccupied = false;
        SpawnPointInfos[i].SpawnedItem = nullptr;
    }
}

void AItemSpawnZone::UpdateSpawnState()
{
    if (!HasAuthority()) return;

    bool bShouldSpawn = CurrentPlankCount < MaxPlankCount ||
        CurrentTentCount < MaxTentCount ||
        CurrentGunCount < MaxGunCount;

    if (bShouldSpawn)
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

    ForceNetUpdate();
}