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

    // �⺻ �Ӽ�
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Item")
    EInventorySlot ItemType;

    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Item")
    int32 Amount;

    // ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* CollisionComponent;

    // �����Ǵ� ����
    UPROPERTY(Replicated)
    bool bIsBuiltItem;

    UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
    bool bIsPickedUp;

    UPROPERTY(ReplicatedUsing = OnRep_OwningPlayer)
    class ACharacter* OwningPlayer;

    // ��Ʈ��ũ RPC
    UFUNCTION(Server, Reliable)
    void PickUp(ACharacter* Character);

    UFUNCTION(Server, Reliable)
    void Drop();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnPickedUp(ACharacter* NewOwner);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnDropped();

    // ���� ������
    UFUNCTION(BlueprintPure, Category = "Item")
    bool IsPickedUp() const { return bIsPickedUp; }

    UFUNCTION(BlueprintPure, Category = "Item")
    ACharacter* GetOwningPlayer() const { return OwningPlayer; }

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetMobility(EComponentMobility::Type NewMobility);

protected:
    virtual void BeginPlay() override;

    // ���� �̺�Ʈ
    UFUNCTION()
    void OnRep_IsPickedUp();

    UFUNCTION()
    void OnRep_OwningPlayer();

    // ��ƿ��Ƽ �Լ�
    void UpdateItemState();
    void UpdatePhysicsState(bool bEnablePhysics);
    void UpdateCollisionState(bool bEnableCollision);
    void SetItemPhysicsState(bool bEnablePhysics);

    // �����ۺ� Ŀ���� ��ġ/ȸ�� ������ ���� ���� �Լ�
    UFUNCTION(BlueprintNativeEvent, Category = "Item")
    FTransform GetPickupTransform(ACharacter* Player) const;
    virtual FTransform GetPickupTransform_Implementation(ACharacter* Player) const;

    // �����ۺ� �⺻ ������
    UPROPERTY(EditDefaultsOnly, Category = "Item Settings")
    FVector DefaultPickupOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Item Settings")
    FRotator DefaultPickupRotation;

private:
    // ���� ���� �Լ�
    void AttachToPlayer(ACharacter* Player);
    void DetachFromPlayer();
    void SetupInitialState();
    void InitializeComponents();
};