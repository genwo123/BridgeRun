// ItemSpawnZone.cpp
#include "ItemSpawnZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

AItemSpawnZone::AItemSpawnZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // 스폰 볼륨 설정
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    // 콜리전 설정 수정
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpawnVolume->SetGenerateOverlapEvents(true);
    SpawnVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SpawnVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    // 기본값 설정
    SpawnInterval = 5.0f;
    MaxItemCount = 10;
    CurrentItemCount = 0;

    // Overlap 이벤트 바인딩
    SpawnVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapBegin);
    SpawnVolume->OnComponentEndOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapEnd);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    // 초기 스폰 시작
    StartSpawnTimer();
}

void AItemSpawnZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItemSpawnZone::StartSpawnTimer()
{
    if (!GetWorld()->GetTimerManager().IsTimerActive(SpawnTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnTimer,
            this,
            &AItemSpawnZone::SpawnItem,
            SpawnInterval,
            true);

     }
}

void AItemSpawnZone::SpawnItem()
{

    if (CurrentItemCount >= MaxItemCount || ItemsToSpawn.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, ItemsToSpawn.Num() - 1);
    TSubclassOf<AItem> ItemToSpawn = ItemsToSpawn[RandomIndex];

    if (ItemToSpawn)
    {
        FVector SpawnLocation = GetRandomPointInVolume();
        FRotator SpawnRotation = FRotator(0.f);
        GetWorld()->SpawnActor<AItem>(ItemToSpawn,
            SpawnLocation,
            SpawnRotation);

        // CurrentItemCount++ 제거 - OnOverlapBegin에서만 카운트
    }
}

void AItemSpawnZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (AItem* Item = Cast<AItem>(OtherActor))
    {
        CurrentItemCount++;

     }
}

void AItemSpawnZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (AItem* Item = Cast<AItem>(OtherActor))
    {
        if (CurrentItemCount > 0)
        {
            CurrentItemCount--;
        }

        // 아이템이 부족하면 스폰 시작
        if (CurrentItemCount < MaxItemCount)
        {
            StartSpawnTimer();
        }
    }
}

FVector AItemSpawnZone::GetRandomPointInVolume()
{
    FVector ExtentMax = SpawnVolume->Bounds.BoxExtent;
    FVector ExtentMin = -ExtentMax;
    return UKismetMathLibrary::RandomPointInBoundingBox(
        SpawnVolume->GetComponentLocation(),
        ExtentMax);
}

void AItemSpawnZone::LogSpawnStatus(const FString& Message)
{
            
}