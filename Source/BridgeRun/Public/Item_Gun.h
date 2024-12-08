#pragma once
#include "CoreMinimal.h"
#include "Item.h"
#include "Item_Gun.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Gun : public AItem
{
    GENERATED_BODY()

protected:
    // 탄약 관련
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Ammo")
    int32 MaxAmmo = 3;

    UPROPERTY(SaveGame)
    int32 CurrentAmmo;

    // 아이템 상태
    UPROPERTY()
    bool bIsHeld;

    // 총 고유 태그
    UPROPERTY(SaveGame)
    FString GunTag;

    // 조준 관련
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

    // 태그 관련 함수
    void SetGunTag(const FString& NewTag);
    FString GetGunTag() const;

    // 탄약 관련 함수
    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void InitializeAmmo() { CurrentAmmo = MaxAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void SetAmmo(int32 NewAmmo) { CurrentAmmo = NewAmmo; }

    // 조준 상태 확인
    UFUNCTION(BlueprintPure, Category = "Gun|Aiming")
    bool IsAiming() const { return bIsAiming; }

    // 무기 기능
    UFUNCTION(BlueprintCallable, Category = "Gun|Combat")
    void Fire();

    // 아이템 픽업/드롭
    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void PickUp(class ACitizen* Player);

    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void Drop();

    UFUNCTION(BlueprintCallable, Category = "Gun|Interaction")
    void ThrowForward();

    // 조준 관련
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun|Aiming")
    float AimFOV = 75.0f;

    UFUNCTION(BlueprintCallable, Category = "Gun|Aiming")
    void ToggleAim();
};