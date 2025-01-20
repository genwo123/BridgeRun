// Copyright BridgeRun Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Item/Item.h"
#include "Item/Item_Gun.h"
#include "ItemSpawnZone.generated.h"

UCLASS()
class BRIDGERUN_API AItemSpawnZone : public AActor
{
    GENERATED_BODY()

public:
    AItemSpawnZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* SpawnVolume;

    // Spawn Settings
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItemCount;

    // Replicated Properties
    UPROPERTY(ReplicatedUsing = OnRep_CurrentItemCount)
    int32 CurrentItemCount;

    UPROPERTY(Replicated)
    TArray<AItem*> SpawnedItems;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Spawn Functions
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

    UFUNCTION()
    FVector GetRandomPointInVolume();

    // Network Functions
    UFUNCTION()
    void OnRep_CurrentItemCount();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemSpawned(AItem* SpawnedItem);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemRemoved(AItem* RemovedItem);

    // Overlap Events
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
    FTimerHandle SpawnTimer;

    // Helper Functions
    bool ValidateSpawnConditions() const;
    void CleanupSpawnedItems();
    void UpdateSpawnState();
};