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

    // 스폰 관련 속성
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemsToSpawn;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* SpawnVolume;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxItemCount;

    // 함수들
    UFUNCTION()
    void SpawnItem();

    UFUNCTION()
    void StartSpawnTimer();  // 새로 추가

    UFUNCTION()
    FVector GetRandomPointInVolume();

protected:
    virtual void BeginPlay() override;

    // 타이머와 카운트 관련
    FTimerHandle SpawnTimer;

    UPROPERTY()
    int32 CurrentItemCount;

    // Overlap 이벤트 함수들
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