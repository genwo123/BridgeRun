#include "Zones/ItemSpawnZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Item/Item.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

AItemSpawnZone::AItemSpawnZone()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 🆕 단순한 컴포넌트 설정
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    // 🆕 작은 스폰 영역 (2m x 2m x 0.2m)
    SpawnVolume->SetBoxExtent(FVector(100.0f, 100.0f, 10.0f));
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 🆕 바닥 메시 (시각적 표시용)
    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    FloorMesh->SetupAttachment(SpawnVolume);
    FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FloorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // 🆕 기본 플레인 메시 사용
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMeshAsset.Succeeded())
    {
        FloorMesh->SetStaticMesh(PlaneMeshAsset.Object);
        FloorMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 1.0f));  // 작게 설정
    }

    // 🆕 기본값 설정
    SpawnInterval = 20.0f;
    MaxItems = 15;
    SpawnHeight = 100.0f;
    SpawnRadius = 80.0f;  // 작은 반경
    CurrentItemCount = 0;
}

void AItemSpawnZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemSpawnZone, CurrentItemCount);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        StartSpawnTimer();
    }
}

void AItemSpawnZone::StartSpawnTimer()
{
    if (!HasAuthority()) return;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SpawnTimer,
            this,
            &AItemSpawnZone::ServerSpawnItem,
            SpawnInterval,
            true
        );
    }
}

void AItemSpawnZone::ServerSpawnItem_Implementation()
{
    if (!HasAuthority() || !CanSpawnMoreItems()) return;

    // 🆕 파괴된 아이템들 정리
    CleanupDestroyedItems();

    if (!CanSpawnMoreItems()) return;

    // 🆕 랜덤하게 아이템 선택
    if (ItemsToSpawn.Num() == 0) return;

    int32 RandomIndex = FMath::RandRange(0, ItemsToSpawn.Num() - 1);
    TSubclassOf<AItem> ItemClass = ItemsToSpawn[RandomIndex];

    // 🆕 랜덤 위치에서 스폰
    FVector SpawnLocation = GetRandomSpawnLocation();
    FRotator SpawnRotation = FRotator(0, FMath::RandRange(0, 360), 0);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = this;

    if (AItem* SpawnedItem = GetWorld()->SpawnActor<AItem>(ItemClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // 🆕 기본 아이템 설정 (물리 활성화로 자연스럽게 떨어지게)
        SpawnedItem->SetReplicates(true);
        SpawnedItem->SetReplicateMovement(true);

        if (SpawnedItem->MeshComponent)
        {
            // 🆕 물리 활성화 - 자연스럽게 떨어져서 쌓이게
            SpawnedItem->MeshComponent->SetSimulatePhysics(true);
            SpawnedItem->MeshComponent->SetEnableGravity(true);
            SpawnedItem->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SpawnedItem->MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

            // 🆕 네트워크 물리 동기화
            SpawnedItem->MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
        }

        CurrentItemCount++;
        UE_LOG(LogTemp, Log, TEXT("Spawned item at %s, total items: %d"),
            *SpawnLocation.ToString(), CurrentItemCount);
    }
}

FVector AItemSpawnZone::GetRandomSpawnLocation() const
{
    // 🆕 작은 원형 영역에서 랜덤 위치 생성
    FVector BaseLocation = GetActorLocation();

    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomRadius = FMath::RandRange(0.0f, SpawnRadius);

    float X = RandomRadius * FMath::Cos(FMath::DegreesToRadians(RandomAngle));
    float Y = RandomRadius * FMath::Sin(FMath::DegreesToRadians(RandomAngle));

    return FVector(
        BaseLocation.X + X,
        BaseLocation.Y + Y,
        BaseLocation.Z + SpawnHeight
    );
}

void AItemSpawnZone::CleanupDestroyedItems()
{
    if (!HasAuthority()) return;

    // 🆕 더 안전한 방식으로 변경
    int32 ValidItemCount = 0;
    FVector CenterLocation = GetActorLocation();
    float CheckRadius = SpawnRadius + 50.0f;

    // 🆕 null 체크를 강화한 안전한 방식
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AItem> ActorItr(World); ActorItr; ++ActorItr)
    {
        AItem* Item = *ActorItr;

        // 🆕 더 엄격한 유효성 검사
        if (!Item || !IsValid(Item) || Item->IsPendingKill())
        {
            continue;
        }

        // 🆕 안전한 거리 계산
        FVector ItemLocation = Item->GetActorLocation();
        float Distance = FVector::Dist(ItemLocation, CenterLocation);

        if (Distance <= CheckRadius)
        {
            ValidItemCount++;
        }
    }

    CurrentItemCount = FMath::Max(0, ValidItemCount);
    UE_LOG(LogTemp, Log, TEXT("Cleanup: Found %d valid items in spawn zone"), ValidItemCount);
}

bool AItemSpawnZone::CanSpawnMoreItems() const
{
    return CurrentItemCount < MaxItems;
}