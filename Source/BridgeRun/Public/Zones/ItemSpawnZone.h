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
    // 생성자 및 기본 함수
    AItemSpawnZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* SpawnVolume;

    // 스폰 설정
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItemCount;

    // 복제 속성
    UPROPERTY(ReplicatedUsing = OnRep_CurrentItemCount)
    int32 CurrentItemCount;

    UPROPERTY(Replicated)
    TArray<AItem*> SpawnedItems;

protected:
    // 라이프사이클 함수
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 스폰 함수
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

    UFUNCTION()
    FVector GetRandomPointInVolume();

    // 네트워크 함수
    UFUNCTION()
    void OnRep_CurrentItemCount();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemSpawned(AItem* SpawnedItem);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemRemoved(AItem* RemovedItem);

    

    // 오버랩 이벤트
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
    // 타이머 핸들
    FTimerHandle SpawnTimer;

    // 헬퍼 함수
    bool ValidateSpawnConditions() const;
    void CleanupSpawnedItems();
    void UpdateSpawnState();
    void InitializeSpawnVolume();
    void ConfigureSpawnedItem(AItem* SpawnedItem);
};