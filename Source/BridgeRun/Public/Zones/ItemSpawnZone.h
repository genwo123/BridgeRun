// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item.h"
#include "ItemSpawnZone.generated.h"

UCLASS()
class BRIDGERUN_API AItemSpawnZone : public AActor
{
    GENERATED_BODY()

public:
    // ������ �� �⺻ �Լ�
    AItemSpawnZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* SpawnVolume;

    // ���� ����
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItemCount;

    // ���� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_CurrentItemCount)
    int32 CurrentItemCount;

    UPROPERTY(Replicated)
    TArray<AItem*> SpawnedItems;

protected:
    // ����������Ŭ �Լ�
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ���� �Լ�
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

    UFUNCTION()
    FVector GetRandomPointInVolume();

    // ��Ʈ��ũ �Լ�
    UFUNCTION()
    void OnRep_CurrentItemCount();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemSpawned(AItem* SpawnedItem);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemRemoved(AItem* RemovedItem);

    

    // ������ �̺�Ʈ
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

private:
    // Ÿ�̸� �ڵ�
    FTimerHandle SpawnTimer;

    // ���� �Լ�
    bool ValidateSpawnConditions() const;
    void CleanupSpawnedItems();
    void UpdateSpawnState();
    void InitializeSpawnVolume();
    void ConfigureSpawnedItem(AItem* SpawnedItem);
};