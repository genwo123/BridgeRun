// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item.h"
#include "ItemSpawnZone.generated.h"

// 스폰 포인트 상태 정보를 저장하는 구조체
USTRUCT(BlueprintType)
struct FSpawnPointInfo
{
    GENERATED_BODY()

    // 스폰 포인트가 사용 중인지 여부
    UPROPERTY(BlueprintReadWrite)
    bool bIsOccupied = false;

    // 스폰된 아이템 참조 (추적용)
    UPROPERTY()
    AItem* SpawnedItem = nullptr;

    // 아이템 타입 (필요한 경우)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AItem> PreferredItemClass = nullptr;
};

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

    // 바닥 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* FloorMesh;

    // 스폰 포인트 컴포넌트 배열 (에디터에서 직접 배치)
    UPROPERTY(VisibleAnywhere, Category = "Spawn Points")
    TArray<class USceneComponent*> SpawnPoints;

    // 스폰 포인트 정보 (에디터에서 설정 가능)
    UPROPERTY(EditAnywhere, Category = "Spawn Points")
    TArray<FSpawnPointInfo> SpawnPointInfos;

    // 아이템 유형별 스폰 제한
    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxPlankCount = 15;

    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxTentCount = 10;

    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxGunCount = 3;

    // 스폰 설정
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval = 5.0f;

    // 복제 속성
    UPROPERTY(ReplicatedUsing = OnRep_SpawnedItems)
    TArray<AItem*> SpawnedItems;

    // 아이템 종류별 카운트 (복제)
    UPROPERTY(Replicated)
    int32 CurrentPlankCount;

    UPROPERTY(Replicated)
    int32 CurrentTentCount;

    UPROPERTY(Replicated)
    int32 CurrentGunCount;

protected:
    // 라이프사이클 함수
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 스폰 함수
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

    // 네트워크 함수
    UFUNCTION()
    void OnRep_SpawnedItems();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemSpawned(AItem* SpawnedItem, int32 SpawnPointIndex);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemRemoved(AItem* RemovedItem, int32 SpawnPointIndex);

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
    bool ValidateSpawnConditions(TSubclassOf<AItem>& OutItemToSpawn) const;
    void CleanupSpawnedItems();
    void UpdateSpawnState();
    void InitializeSpawnVolume();
    void ConfigureSpawnedItem(AItem* SpawnedItem, int32 SpawnPointIndex);

    // 스폰 포인트 관리 함수
    void InitializeSpawnPoints();
    void ArrangeSpawnPointsInGrid(int32 Rows, int32 Columns, float SpacingX, float SpacingY);
    int32 FindFreeSpawnPointForItem(TSubclassOf<AItem> ItemClass) const;
    void MarkSpawnPointOccupied(int32 Index, AItem* Item, bool bOccupied);
    bool IsSpawnPointOccupied(int32 Index) const;
    int32 GetSpawnPointIndexForPosition(const FVector& Position) const;

    // 아이템 카운트 관리 함수
    void IncrementItemCount(TSubclassOf<AItem> ItemClass);
    void DecrementItemCount(TSubclassOf<AItem> ItemClass);
    int32 GetCurrentCountForItemClass(TSubclassOf<AItem> ItemClass) const;
    int32 GetMaxCountForItemClass(TSubclassOf<AItem> ItemClass) const;
};