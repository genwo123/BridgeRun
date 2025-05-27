// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item.h"
#include "EngineUtils.h"
#include "ItemSpawnZone.generated.h"

UCLASS()
class BRIDGERUN_API AItemSpawnZone : public AActor
{
    GENERATED_BODY()
public:
    AItemSpawnZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 🆕 간단한 컴포넌트 구조
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* SpawnVolume;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* FloorMesh;

    // 🆕 단순한 스폰 설정
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval = 20.0f;  // 20초마다 스폰

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItems = 15;  // 최대 15개까지만

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnHeight = 100.0f;  // 스폰 높이

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnRadius = 100.0f;  // 스폰 반경 (좁게)

protected:
    virtual void BeginPlay() override;

    // 🆕 단순한 스폰 함수
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

private:
    FTimerHandle SpawnTimer;

    // 🆕 현재 스폰된 아이템 개수 추적
    UPROPERTY(Replicated)
    int32 CurrentItemCount = 0;

    // 🆕 단순한 헬퍼 함수들
    FVector GetRandomSpawnLocation() const;
    void CleanupDestroyedItems();
    bool CanSpawnMoreItems() const;
};