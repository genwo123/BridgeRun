// Copyright BridgeRun Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "DeathVolume.generated.h"

UCLASS()
class BRIDGERUN_API ADeathVolume : public AActor
{
    GENERATED_BODY()

public:
    ADeathVolume();

protected:
    virtual void BeginPlay() override;

    // 충돌 박스
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* DeathBox;

    // 오버랩 이벤트 처리
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    // 리스폰 관련 설정
    UPROPERTY(EditAnywhere, Category = "Settings")
    float PlayerRespawnDelay = 5.0f;  // 플레이어 리스폰 대기 시간

    UPROPERTY(EditAnywhere, Category = "Settings")
    float TrophyRespawnHeight = 500.0f;  // 트로피 리스폰 높이

    UPROPERTY(EditAnywhere, Category = "Settings")
    float SafeRespawnRadius = 300.0f;   // 트로피 리스폰 시 안전 거리

private:
    // 트로피 리스폰 위치 계산
    FVector GetTrophyRespawnLocation(const FVector& DeathLocation) const;

    FVector GetRespawnLocation() const;

    FVector FindSafeLocationNearby(const FVector& SearchOrigin) const;
};