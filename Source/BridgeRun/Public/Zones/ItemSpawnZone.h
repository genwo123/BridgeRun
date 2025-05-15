// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item.h"
#include "ItemSpawnZone.generated.h"

// ���� ����Ʈ ���� ������ �����ϴ� ����ü
USTRUCT(BlueprintType)
struct FSpawnPointInfo
{
    GENERATED_BODY()

    // ���� ����Ʈ�� ��� ������ ����
    UPROPERTY(BlueprintReadWrite)
    bool bIsOccupied = false;

    // ������ ������ ���� (������)
    UPROPERTY()
    AItem* SpawnedItem = nullptr;

    // ������ Ÿ�� (�ʿ��� ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AItem> PreferredItemClass = nullptr;
};

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

    // �ٴ� �޽� ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* FloorMesh;

    // ���� ����Ʈ ������Ʈ �迭 (�����Ϳ��� ���� ��ġ)
    UPROPERTY(VisibleAnywhere, Category = "Spawn Points")
    TArray<class USceneComponent*> SpawnPoints;

    // ���� ����Ʈ ���� (�����Ϳ��� ���� ����)
    UPROPERTY(EditAnywhere, Category = "Spawn Points")
    TArray<FSpawnPointInfo> SpawnPointInfos;

    // ������ ������ ���� ����
    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxPlankCount = 15;

    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxTentCount = 10;

    UPROPERTY(EditAnywhere, Category = "Spawn Limits")
    int32 MaxGunCount = 3;

    // ���� ����
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval = 5.0f;

    // ���� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_SpawnedItems)
    TArray<AItem*> SpawnedItems;

    // ������ ������ ī��Ʈ (����)
    UPROPERTY(Replicated)
    int32 CurrentPlankCount;

    UPROPERTY(Replicated)
    int32 CurrentTentCount;

    UPROPERTY(Replicated)
    int32 CurrentGunCount;

protected:
    // ����������Ŭ �Լ�
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ���� �Լ�
    UFUNCTION(Server, Reliable)
    void ServerSpawnItem();

    UFUNCTION()
    void StartSpawnTimer();

    // ��Ʈ��ũ �Լ�
    UFUNCTION()
    void OnRep_SpawnedItems();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemSpawned(AItem* SpawnedItem, int32 SpawnPointIndex);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnItemRemoved(AItem* RemovedItem, int32 SpawnPointIndex);

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
    bool ValidateSpawnConditions(TSubclassOf<AItem>& OutItemToSpawn) const;
    void CleanupSpawnedItems();
    void UpdateSpawnState();
    void InitializeSpawnVolume();
    void ConfigureSpawnedItem(AItem* SpawnedItem, int32 SpawnPointIndex);

    // ���� ����Ʈ ���� �Լ�
    void InitializeSpawnPoints();
    void ArrangeSpawnPointsInGrid(int32 Rows, int32 Columns, float SpacingX, float SpacingY);
    int32 FindFreeSpawnPointForItem(TSubclassOf<AItem> ItemClass) const;
    void MarkSpawnPointOccupied(int32 Index, AItem* Item, bool bOccupied);
    bool IsSpawnPointOccupied(int32 Index) const;
    int32 GetSpawnPointIndexForPosition(const FVector& Position) const;

    // ������ ī��Ʈ ���� �Լ�
    void IncrementItemCount(TSubclassOf<AItem> ItemClass);
    void DecrementItemCount(TSubclassOf<AItem> ItemClass);
    int32 GetCurrentCountForItemClass(TSubclassOf<AItem> ItemClass) const;
    int32 GetMaxCountForItemClass(TSubclassOf<AItem> ItemClass) const;
};