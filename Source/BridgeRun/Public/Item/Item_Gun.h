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
    // ź�� ���� �Ӽ�
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Ammo")
    int32 MaxAmmo = 3;

    UPROPERTY(Replicated, SaveGame)
    int32 CurrentAmmo;

    // ���� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_AimState)
    bool bIsAiming = false;

    // ī�޶� ���� �����
    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    // ����� ����
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    bool bShowDebugLine = true;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    float DebugLineDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    FColor DebugLineColor = FColor::Red;

    // ���� �� �ʱ�ȭ
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    // ���� ���� �̺�Ʈ
    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_AimState();

    // �߻� ���� ���� �Լ�
    void ShowFireDebugEffects(const FVector& Start, const FVector& End, const FHitResult& HitResult);
    void ProcessFireHit(const FHitResult& HitResult);
    void UpdateAmmoCount(bool bLogChange = true);

public:
    AItem_Gun();

    // ��� �Լ� ������
    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    // �׼� �Լ�
    UFUNCTION(Server, Reliable)
    void Fire();

    UFUNCTION(Server, Reliable)
    void ThrowForward();

    UFUNCTION(Server, Reliable)
    void ToggleAim();

    // ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void InitializeAmmo() { CurrentAmmo = MaxAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void SetAmmo(int32 NewAmmo) { CurrentAmmo = NewAmmo; }

    UFUNCTION(BlueprintPure, Category = "Gun|Aiming")
    bool IsAiming() const { return bIsAiming; }

    // ���� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun|Aiming")
    float AimFOV = 75.0f;

    // ���� �Ӽ�
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float ThrowForce = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float LinearDamping = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float AngularDamping = 0.5f;
};