// Public/Item/Item_Plank.h
#pragma once
#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Plank.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API AItem_Plank : public AItem
{
    GENERATED_BODY()

public:
    AItem_Plank();

    // ���� ���� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_IsBuilt, EditAnywhere, Category = "Plank")
    bool bIsBuiltPlank;

    UPROPERTY(Replicated, EditAnywhere, Category = "Plank")
    float MaxPlankLength;

    // ������ ��Ƽ����
    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    // �׼� �Լ�
    UFUNCTION(Server, Reliable)
    void OnPlaced();

    // ��Ʈ��ũ ����ȭ �Լ�
    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetPlankPhysicsState(EComponentMobility::Type NewMobility);

    // ������ �Լ�
    UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
    UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ���� �̺�Ʈ
    UFUNCTION()
    void OnRep_IsBuilt();

    // ���� �Լ�
    void SetupCollisionSettings();
    void ApplyBuiltPlankState();
};