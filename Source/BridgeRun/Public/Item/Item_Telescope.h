// Public/Item/Item_Telescope.h
#pragma once
#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Telescope.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Telescope : public AItem
{
    GENERATED_BODY()

public:
    AItem_Telescope();

    // ���� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_ZoomState)
    bool bIsZoomed = false;

    // �⺻ �Լ� ������
    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    // �� ���
    UFUNCTION(Server, Reliable)
    void ToggleZoom();

    // ������ ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Telescope")
    float ZoomedFOV = 45.0f;

protected:
    // ī�޶� ���� �����
    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    // ī�޶� ���� �Լ�
    void ResetCameraSettings(class ACitizen* Player);
    void ApplyZoomSettings(class ACitizen* Player, bool bZoom);
    void UpdateCameraTransform(class ACitizen* Player);

    // ��Ʈ��ũ ���� �� �̺�Ʈ
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_ZoomState();
};