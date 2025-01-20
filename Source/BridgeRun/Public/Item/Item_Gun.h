// Public/Item/Item_Gun.h
#pragma once
#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Gun.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Gun : public AItem
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Ammo")
    int32 MaxAmmo = 3;

    UPROPERTY(Replicated, SaveGame)
    int32 CurrentAmmo;

    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_AimState)
    bool bIsAiming = false;

    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_AimState();

public:
    AItem_Gun();

    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    UFUNCTION(Server, Reliable)
    void Fire();

    UFUNCTION(Server, Reliable)
    void ThrowForward();

    UFUNCTION(Server, Reliable)
    void ToggleAim();

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void InitializeAmmo() { CurrentAmmo = MaxAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void SetAmmo(int32 NewAmmo) { CurrentAmmo = NewAmmo; }

    UFUNCTION(BlueprintPure, Category = "Gun|Aiming")
    bool IsAiming() const { return bIsAiming; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun|Aiming")
    float AimFOV = 75.0f;
};