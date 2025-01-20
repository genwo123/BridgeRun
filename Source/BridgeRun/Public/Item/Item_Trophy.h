// Copyright BridgeRun Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Trophy.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Trophy : public AItem
{
    GENERATED_BODY()

public:
    AItem_Trophy();

    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;


    virtual void MulticastOnPickedUp_Implementation(class ACharacter* Player) override;



protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FTransform GetPickupTransform_Implementation(ACharacter* Player) const override;

    // Ʈ���� ���� ����
    UPROPERTY(ReplicatedUsing = OnRep_TrophyState)
    bool bIsTrophyActive;

    UFUNCTION()
    void OnRep_TrophyState();


    // Ʈ���� ��ȯ ����
    UPROPERTY(EditDefaultsOnly, Category = "Trophy")
    FVector PickupOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Trophy")
    FRotator PickupRotation;

private:
    // ���� �Լ�
    void UpdateTrophyState();
    void UpdateTrophyVisibility(bool bIsVisible);
    void SetTrophyPhysics(bool bEnablePhysics);
    void UpdateTrophyCollision();
    void AttachTrophyToPlayer(class ACharacter* Player);
    void DetachTrophyFromPlayer();
};