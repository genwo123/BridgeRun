// ItemSpawnZone.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.h"
#include "ItemSpawnZone.generated.h"

UCLASS()
class BRIDGERUN_API AItemSpawnZone : public AActor
{
    GENERATED_BODY()
public:
    AItemSpawnZone();
    virtual void Tick(float DeltaTime) override;

    // ���� ���� �Ӽ�
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* SpawnVolume;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItemCount;

    // �Լ���
    UFUNCTION()
    void SpawnItem();

    UFUNCTION()
    void StartSpawnTimer();  // ���� �߰�

    UFUNCTION()
    FVector GetRandomPointInVolume();

protected:
    virtual void BeginPlay() override;

    // Ÿ�̸ӿ� ī��Ʈ ����
    FTimerHandle SpawnTimer;

    UPROPERTY()
    int32 CurrentItemCount;

    // Overlap �̺�Ʈ �Լ���
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

};