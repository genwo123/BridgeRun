// CombatComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item_Telescope.h"
#include "Item_Gun.h"
#include "PlayerModeComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;
    UPROPERTY()
    class ACitizen* OwnerCitizen;

public:
    UCombatComponent();

    // 총 소유 상태
    UPROPERTY()
    bool bHasGun = false;

    // 무기 관련 액션
    void DropCurrentWeapon();
    void HandleShoot();
    void HandleAim();

    // 전투 모드 상태 변경
    void OnCombatModeEntered();
    void OnCombatModeExited();

    // 망원경 관련
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Telescope> TelescopeClass;

    UPROPERTY()
    class AItem_Telescope* EquippedTelescope;

    void OnTelescopeEquipped(AItem_Telescope* Telescope);
    void OnTelescopeUnequipped();

    // 총 관련
    UPROPERTY()
    class AItem_Gun* EquippedGun;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Gun> GunClass;

    void OnGunEquipped(AItem_Gun* Gun);
    void OnGunUnequipped();

private:
    void ResetCameraSettings();
};