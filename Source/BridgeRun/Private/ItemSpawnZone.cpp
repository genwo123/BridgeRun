// ItemSpawnZone.cpp
#include "ItemSpawnZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

AItemSpawnZone::AItemSpawnZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // ���� ���� ����
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;

    // �ݸ��� ���� ����
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpawnVolume->SetGenerateOverlapEvents(true);
    SpawnVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SpawnVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    // �⺻�� ����
    SpawnInterval = 5.0f;
    MaxItemCount = 10;
    CurrentItemCount = 0;

    // Overlap �̺�Ʈ ���ε�
    SpawnVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapBegin);
    SpawnVolume->OnComponentEndOverlap.AddDynamic(this, &AItemSpawnZone::OnOverlapEnd);
}

void AItemSpawnZone::BeginPlay()
{
    Super::BeginPlay();

    // �ʱ� ���� ����
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

        // ����� �α�
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            TEXT("Spawn Timer Started"));
    }
}

void AItemSpawnZone::SpawnItem()
{
    // ����� �α�
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("Attempting Spawn: Current Count: %d/%d"),
            CurrentItemCount, MaxItemCount));

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

        // CurrentItemCount++ ���� - OnOverlapBegin������ ī��Ʈ
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

        // ����� �α׷� Ȯ��
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
            FString::Printf(TEXT("OnOverlapBegin - Count increased to: %d"), CurrentItemCount));
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

            // ����� �α�
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange,
                FString::Printf(TEXT("Item left zone: Count %d/%d"),
                    CurrentItemCount, MaxItemCount));
        }

        // �������� �����ϸ� ���� ����
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
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("SpawnZone: %s (Count: %d/%d)"),
            *Message,
            CurrentItemCount,
            MaxItemCount));
}