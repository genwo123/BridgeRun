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

    // �浹 �ڽ�
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* DeathBox;

    // ������ �̺�Ʈ ó��
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    // ������ ���� ����
    UPROPERTY(EditAnywhere, Category = "Settings")
    float PlayerRespawnDelay = 5.0f;  // �÷��̾� ������ ��� �ð�

    UPROPERTY(EditAnywhere, Category = "Settings")
    float TrophyRespawnHeight = 500.0f;  // Ʈ���� ������ ����

    UPROPERTY(EditAnywhere, Category = "Settings")
    float SafeRespawnRadius = 300.0f;   // Ʈ���� ������ �� ���� �Ÿ�

private:
    // Ʈ���� ������ ��ġ ���
    FVector GetTrophyRespawnLocation(const FVector& DeathLocation) const;

    FVector GetRespawnLocation() const;

    FVector FindSafeLocationNearby(const FVector& SearchOrigin) const;
};