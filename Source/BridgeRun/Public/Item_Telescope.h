// Item_Telescope.h
#pragma once
#include "CoreMinimal.h"
#include "Item.h"
#include "Item_Telescope.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Telescope : public AItem
{
    GENERATED_BODY()
public:
    AItem_Telescope();

    UPROPERTY()
    bool bIsHeld;

    void PickUp(class ACitizen* Player);
    void Drop();

    UFUNCTION()
    void ToggleZoom();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Telescope")  // EditAnywhere를 EditDefaultsOnly로 변경하고 BlueprintReadWrite 추가
        float ZoomedFOV = 45.0f;

    bool bIsZoomed = false;

protected:
    float DefaultFOV;
    float DefaultArmLength;  // SpringArm 길이 저장용
    

    void ResetCameraSettings(ACitizen* Player);

    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult) override;
};