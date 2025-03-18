// Public/Modes/Components/CombatComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/Item_Telescope.h"
#include "Item/Item_Gun.h"
#include "Modes/PlayerModeComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 기본 함수
    UCombatComponent();

    // 모드 전환 함수
    void OnCombatModeEntered();
    void OnCombatModeExited();

    // 무기 액션 함수
    UFUNCTION(Server, Reliable)
    void HandleShoot();
    UFUNCTION(Server, Reliable)
    void HandleAim();
    UFUNCTION(Server, Reliable)
    void DropCurrentWeapon();

    // 망원경 관련 함수
    UFUNCTION(Server, Reliable)
    void OnTelescopeEquipped(AItem_Telescope* Telescope);
    UFUNCTION(Server, Reliable)
    void OnTelescopeUnequipped();

    // 총기 관련 함수
    UFUNCTION(Server, Reliable)
    void OnGunEquipped(AItem_Gun* Gun);
    UFUNCTION(Server, Reliable)
    void OnGunUnequipped();

    // 접근자 메서드
    UFUNCTION(BlueprintCallable, Category = "Combat")
    AItem_Telescope* GetEquippedTelescope() const { return EquippedTelescope; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TSubclassOf<AItem_Telescope> GetTelescopeClass() const { return TelescopeClass; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AItem_Gun* GetEquippedGun() const { return EquippedGun; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TSubclassOf<AItem_Gun> GetGunClass() const { return GunClass; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasEquippedTelescope() const { return EquippedTelescope != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasEquippedGun() const { return EquippedGun != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool GetHasGun() const { return bHasGun; }

protected:
    // 기본 오버라이드 함수
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 소유자 참조
    UPROPERTY()
    class ACitizen* OwnerCitizen;

    // 무기 상태
    UPROPERTY(Replicated)
    bool bHasGun = false;

    // 망원경 관련
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Telescope> TelescopeClass;
    UPROPERTY(Replicated)
    class AItem_Telescope* EquippedTelescope;

    // 총기 관련
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Gun> GunClass;
    UPROPERTY(Replicated)
    class AItem_Gun* EquippedGun;

private:
    // 헬퍼 함수
    void ResetCameraSettings();
    void HideCurrentGun();
    bool IsValidCombatState() const;
    void UpdateCharacterRotation(bool bAllowYawRotation);
};