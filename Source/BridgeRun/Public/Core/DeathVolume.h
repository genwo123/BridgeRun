// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathVolume.generated.h"

// ���� ����
class UBoxComponent;
class ACitizen;

/**
 * ���� ó���� ���� ���� Ŭ����
 * �� �Ʒ��� ��ġ�Ͽ� ĳ���ͳ� �������� �� ������ �������� �� ����
 */
UCLASS()
class BRIDGERUN_API ADeathVolume : public AActor
{
    GENERATED_BODY()

public:
    /** �⺻ ������ */
    ADeathVolume();

protected:
    /** ���� ���۽� �ʱ�ȭ */
    virtual void BeginPlay() override;

    /** �浹 ������ �ڽ� ������Ʈ */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* DeathBox;

    /** ������ �̺�Ʈ ó�� �Լ� */
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    /** �÷��̾� ������ ��� �ð� (��) */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float PlayerRespawnDelay = 5.0f;

    /** Ʈ���� ������ �� ����� �⺻ ���� */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float TrophyRespawnHeight = 500.0f;

    /** Ʈ���� ������ �� ������ ��ġ Ž�� �ݰ� */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float SafeRespawnRadius = 300.0f;

    /** Ÿ�� ����Ʈ�� ���� ��� ����� �⺻ ������ ��ġ */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    FVector DefaultRespawnLocation = FVector(-970.0f, -146.44342f, 222.000671f);

private:
    /** ������ ��ġ ��ó���� ������ ��ġ ã�� */
    FVector FindSafeLocationNearby(const FVector& SearchOrigin) const;

    /** �÷��̾� ������ ��ġ ã�� */
    FVector GetRespawnLocation() const;

    /** Ʈ���� ������ ��ġ ã�� */
    FVector GetTrophyRespawnLocation(const FVector& DeathLocation) const;

    /** Ʈ���Ǹ� �� �ù� ĳ���� ó�� */
    void HandleCitizenWithTrophy(ACitizen* Citizen);

    /** Ÿ�̸� ��� ������ ó�� */
    void ScheduleRespawn(ACitizen* Citizen);
};