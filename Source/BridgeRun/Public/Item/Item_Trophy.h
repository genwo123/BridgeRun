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


    UFUNCTION(NetMulticast, Reliable)
    void MulticastHandleRespawn(const FVector& NewLocation);

    UFUNCTION(Server, Reliable)
    void ServerTryRespawn(const FVector& RespawnLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trophy")
    int32 TrophyValue = 5;  // 기본값 5점


protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FTransform GetPickupTransform_Implementation(ACharacter* Player) const override;

    // 트로피 상태 관련
    UPROPERTY(ReplicatedUsing = OnRep_TrophyState)
    bool bIsTrophyActive;

    UFUNCTION()
    void OnRep_TrophyState();


    // 트로피 변환 관련
    UPROPERTY(EditDefaultsOnly, Category = "Trophy")
    FVector PickupOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Trophy")
    FRotator PickupRotation;

private:
    // 헬퍼 함수
    void UpdateTrophyState();
    void UpdateTrophyVisibility(bool bIsVisible);
    void SetTrophyPhysics(bool bEnablePhysics);
    void UpdateTrophyCollision();
    void AttachTrophyToPlayer(class ACharacter* Player);
    void DetachTrophyFromPlayer();
};