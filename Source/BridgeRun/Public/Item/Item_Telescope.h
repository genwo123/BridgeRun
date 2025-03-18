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

    // 상태 속성
    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_ZoomState)
    bool bIsZoomed = false;

    // 기본 함수 재정의
    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    // 줌 기능
    UFUNCTION(Server, Reliable)
    void ToggleZoom();

    // 망원경 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Telescope")
    float ZoomedFOV = 45.0f;

protected:
    // 카메라 설정 저장용
    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    // 카메라 관련 함수
    void ResetCameraSettings(class ACitizen* Player);
    void ApplyZoomSettings(class ACitizen* Player, bool bZoom);
    void UpdateCameraTransform(class ACitizen* Player);

    // 네트워크 복제 및 이벤트
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_ZoomState();
};