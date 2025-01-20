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

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	class ACitizen* OwnerCitizen;

public:
	UCombatComponent();

	UPROPERTY(Replicated)
	bool bHasGun = false;

	UFUNCTION(Server, Reliable)
	void DropCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void HandleShoot();

	UFUNCTION(Server, Reliable)
	void HandleAim();

	void OnCombatModeEntered();
	void OnCombatModeExited();

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
	TSubclassOf<class AItem_Telescope> TelescopeClass;

	UPROPERTY(Replicated)
	class AItem_Telescope* EquippedTelescope;

	UFUNCTION(Server, Reliable)
	void OnTelescopeEquipped(AItem_Telescope* Telescope);

	UFUNCTION(Server, Reliable)
	void OnTelescopeUnequipped();

	UPROPERTY(Replicated)
	class AItem_Gun* EquippedGun;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
	TSubclassOf<class AItem_Gun> GunClass;

	UFUNCTION(Server, Reliable)
	void OnGunEquipped(AItem_Gun* Gun);

	UFUNCTION(Server, Reliable)
	void OnGunUnequipped();

private:
	void ResetCameraSettings();
};