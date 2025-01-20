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

    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_ZoomState)
    bool bIsZoomed = false;

    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    UFUNCTION(Server, Reliable)
    void ToggleZoom();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Telescope")
    float ZoomedFOV = 45.0f;

protected:
    float DefaultFOV;
    float DefaultArmLength;

    void ResetCameraSettings(class ACitizen* Player);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_ZoomState();

};