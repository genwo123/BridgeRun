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

    // 컴포넌트 생성 및 설정
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    InitializeSpawnVolume();

    // 기본값 설정
    SpawnInterval = 5.0f;
    MaxItemCount = 10;
    CurrentItemCount = 0;
}

void AItemSpawnZone::InitializeSpawnVolume()
{
    if (!SpawnVolume) return;

    // 충돌 설정
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpawnVolume->SetGenerateOverlapEvents(true);
    SpawnVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SpawnVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    SpawnVolume->SetIsReplicated(true);

    // 오버랩 이벤트 바인딩
    SpawnVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapBegin);
    SpawnVolume->OnComponentEndOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapEnd);
}

void AItemSpawnZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 복제 속성 등록
    DOREPLIFETIME(AItemSpawnZone, CurrentItemCount);
    DOREPLIFETIME(AItemSpawnZone, SpawnedItems);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 스폰 타이머 시작
    if (HasAuthority())
    {
        StartSpawnTimer();
    }
}

void AItemSpawnZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 정리 작업
    if (HasAuthority())
    {
        CleanupSpawnedItems();
    }

    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimer);
    }
}

void AItemSpawnZone::StartSpawnTimer()
{
    if (!HasAuthority()) return;

    // 이미 활성화된 타이머가 없을 경우만 타이머 설정
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
    // 전제 조건 검사
    if (!HasAuthority() || !ValidateSpawnConditions())
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    // 스폰할 아이템 클래스 선택
    int32 RandomIndex = FMath::RandRange(0, ItemsToSpawn.Num() - 1);
    TSubclassOf<AItem> ItemToSpawn = ItemsToSpawn[RandomIndex];
    if (!ItemToSpawn) return;

    // 스폰 매개변수 설정
    FVector SpawnLocation = GetRandomPointInVolume();
    FRotator SpawnRotation = FRotator::ZeroRotator;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = this;

    // 아이템 스폰
    if (AItem* SpawnedItem = World->SpawnActor<AItem>(ItemToSpawn, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // 아이템 설정
        ConfigureSpawnedItem(SpawnedItem);

        // 상태 업데이트
        SpawnedItems.Add(SpawnedItem);
        CurrentItemCount++;

        // 네트워크 통지 및 상태 업데이트
        MulticastOnItemSpawned(SpawnedItem);
        UpdateSpawnState();
    }
}

void AItemSpawnZone::ConfigureSpawnedItem(AItem* SpawnedItem)
{
    if (!SpawnedItem) return;

    // 기본 네트워크 설정
    SpawnedItem->SetReplicates(true);
    SpawnedItem->SetReplicateMovement(true);

    // 총기 특수 처리
    if (AItem_Gun* Gun = Cast<AItem_Gun>(SpawnedItem))
    {
        Gun->InitializeAmmo();
        if (Gun->CollisionComponent)
        {
            Gun->CollisionComponent->SetGenerateOverlapEvents(false);
        }
    }
}

void AItemSpawnZone::MulticastOnItemSpawned_Implementation(AItem* SpawnedItem)
{
    if (!SpawnedItem) return;

    // 충돌 활성화
    if (SpawnedItem->CollisionComponent)
    {
        SpawnedItem->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AItemSpawnZone::MulticastOnItemRemoved_Implementation(AItem* RemovedItem)
{
    if (!RemovedItem) return;

    // 아이템 목록에서 제거
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

    // 영역에 들어온 아이템 추적
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

    // 영역을 떠난 아이템 제거
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
    // 복제 이벤트 처리
    UpdateSpawnState();
}

FVector AItemSpawnZone::GetRandomPointInVolume()
{
    // 볼륨 내 무작위 위치 생성
    FVector ExtentMax = SpawnVolume->Bounds.BoxExtent;
    return UKismetMathLibrary::RandomPointInBoundingBox(
        SpawnVolume->GetComponentLocation(),
        ExtentMax);
}

bool AItemSpawnZone::ValidateSpawnConditions() const
{
    // 스폰 조건 검증
    return CurrentItemCount < MaxItemCount && ItemsToSpawn.Num() > 0;
}

void AItemSpawnZone::CleanupSpawnedItems()
{
    // 모든 스폰된 아이템 정리
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