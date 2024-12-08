#pragma once
#include "CoreMinimal.h"
#include "Item.h"
#include "Item_Gun.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Gun : public AItem
{
    GENERATED_BODY()

protected:
    // ź�� ����
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Ammo")
    int32 MaxAmmo = 3;

    UPROPERTY(SaveGame)
    int32 CurrentAmmo;

    // ������ ����
    UPROPERTY()
    bool bIsHeld;

    // �� ���� �±�
    UPROPERTY(SaveGame)
    FString GunTag;

    // ���� ����
    UPROPERTY()
    bool bIsAiming = false;

    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    virtual void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult) override;

public:
    AItem_Gun();

    // �±� ���� �Լ�
    void SetGunTag(const FString& NewTag);
    FString GetGunTag() const;

    // ź�� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void InitializeAmmo() { CurrentAmmo = MaxAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void SetAmmo(int32 NewAmmo) { CurrentAmmo = NewAmmo; }

    // ���� ���� Ȯ��
    UFUNCTION(BlueprintPure, Category = "Gun|Aiming")
    bool IsAiming() const { return bIsAiming; }

    // ���� ���
    UFUNCTION(BlueprintCallable, Category = "Gun|Combat")
    void Fire();

    // ������ �Ⱦ�/���
    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void PickUp(class ACitizen* Player);

    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void Drop();

    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void ThrowForward();

    // ���� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun|Aiming")
    float AimFOV = 75.0f;

    UFUNCTION(BlueprintCallable, Category = "Gun|Aiming")
    void ToggleAim();
};