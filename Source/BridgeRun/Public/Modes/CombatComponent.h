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
    // �⺻ �Լ�
    UCombatComponent();

    // ��� ��ȯ �Լ�
    void OnCombatModeEntered();
    void OnCombatModeExited();

    // ���� �׼� �Լ�
    UFUNCTION(Server, Reliable)
    void HandleShoot();
    UFUNCTION(Server, Reliable)
    void HandleAim();
    UFUNCTION(Server, Reliable)
    void DropCurrentWeapon();

    // ������ ���� �Լ�
    UFUNCTION(Server, Reliable)
    void OnTelescopeEquipped(AItem_Telescope* Telescope);
    UFUNCTION(Server, Reliable)
    void OnTelescopeUnequipped();

    // �ѱ� ���� �Լ�
    UFUNCTION(Server, Reliable)
    void OnGunEquipped(AItem_Gun* Gun);
    UFUNCTION(Server, Reliable)
    void OnGunUnequipped();

    // ������ �޼���
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
    // �⺻ �������̵� �Լ�
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ������ ����
    UPROPERTY()
    class ACitizen* OwnerCitizen;

    // ���� ����
    UPROPERTY(Replicated)
    bool bHasGun = false;

    // ������ ����
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Telescope> TelescopeClass;
    UPROPERTY(Replicated)
    class AItem_Telescope* EquippedTelescope;

    // �ѱ� ����
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Items")
    TSubclassOf<class AItem_Gun> GunClass;
    UPROPERTY(Replicated)
    class AItem_Gun* EquippedGun;

private:
    // ���� �Լ�
    void ResetCameraSettings();
    void HideCurrentGun();
    bool IsValidCombatState() const;
    void UpdateCharacterRotation(bool bAllowYawRotation);
};