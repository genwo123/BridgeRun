// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Modes/InvenComponent.h"
#include "Item.generated.h"

UCLASS()
class BRIDGERUN_API AItem : public AActor
{
    GENERATED_BODY()
public:
    AItem();
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 기본 속성
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Item")
    EInventorySlot ItemType;

    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Item")
    int32 Amount;

    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* CollisionComponent;

    // 복제되는 상태
    UPROPERTY(Replicated)
    bool bIsBuiltItem;

    UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
    bool bIsPickedUp;

    UPROPERTY(ReplicatedUsing = OnRep_OwningPlayer)
    class ACharacter* OwningPlayer;

    // 네트워크 RPC
    UFUNCTION(Server, Reliable)
    void PickUp(ACharacter* Character);

    UFUNCTION(Server, Reliable)
    void Drop();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnPickedUp(ACharacter* NewOwner);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnDropped();

    // 상태 접근자
    UFUNCTION(BlueprintPure, Category = "Item")
    bool IsPickedUp() const { return bIsPickedUp; }

    UFUNCTION(BlueprintPure, Category = "Item")
    ACharacter* GetOwningPlayer() const { return OwningPlayer; }

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetMobility(EComponentMobility::Type NewMobility);

protected:
    virtual void BeginPlay() override;

    // 복제 이벤트
    UFUNCTION()
    void OnRep_IsPickedUp();

    UFUNCTION()
    void OnRep_OwningPlayer();

    // 유틸리티 함수
    void UpdateItemState();
    void UpdatePhysicsState(bool bEnablePhysics);
    void UpdateCollisionState(bool bEnableCollision);
    void SetItemPhysicsState(bool bEnablePhysics);

    // 아이템별 커스텀 위치/회전 설정을 위한 가상 함수
    UFUNCTION(BlueprintNativeEvent, Category = "Item")
    FTransform GetPickupTransform(ACharacter* Player) const;
    virtual FTransform GetPickupTransform_Implementation(ACharacter* Player) const;

    // 아이템별 기본 설정값
    UPROPERTY(EditDefaultsOnly, Category = "Item Settings")
    FVector DefaultPickupOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Item Settings")
    FRotator DefaultPickupRotation;

private:
    // 내부 헬퍼 함수
    void AttachToPlayer(ACharacter* Player);
    void DetachFromPlayer();
    void SetupInitialState();
    void InitializeComponents();
};