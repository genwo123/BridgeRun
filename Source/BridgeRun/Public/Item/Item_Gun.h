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
    // 탄약 관련 속성
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Ammo")
    int32 MaxAmmo = 3;

    UPROPERTY(Replicated, SaveGame)
    int32 CurrentAmmo;

    // 상태 속성
    UPROPERTY(ReplicatedUsing = OnRep_HeldState)
    bool bIsHeld;

    UPROPERTY(ReplicatedUsing = OnRep_AimState)
    bool bIsAiming = false;

    // 카메라 설정 저장용
    UPROPERTY()
    float DefaultFOV;

    UPROPERTY()
    float DefaultArmLength;

    // 디버그 설정
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    bool bShowDebugLine = true;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    float DebugLineDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Debug")
    FColor DebugLineColor = FColor::Red;

    // 복제 및 초기화
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    // 상태 변경 이벤트
    UFUNCTION()
    void OnRep_HeldState();

    UFUNCTION()
    void OnRep_AimState();

    // 발사 관련 헬퍼 함수
    void ShowFireDebugEffects(const FVector& Start, const FVector& End, const FHitResult& HitResult);
    void ProcessFireHit(const FHitResult& HitResult);
    void UpdateAmmoCount(bool bLogChange = true);

public:
    AItem_Gun();

    // 상속 함수 재정의
    virtual void PickUp_Implementation(class ACharacter* Character) override;
    virtual void Drop_Implementation() override;

    // 액션 함수
    UFUNCTION(Server, Reliable)
    void Fire();

    UFUNCTION(Server, Reliable)
    void ThrowForward();

    UFUNCTION(Server, Reliable)
    void ToggleAim();

    // 편의 함수
    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void InitializeAmmo() { CurrentAmmo = MaxAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Gun|Ammo")
    void SetAmmo(int32 NewAmmo) { CurrentAmmo = NewAmmo; }

    UFUNCTION(BlueprintPure, Category = "Gun|Aiming")
    bool IsAiming() const { return bIsAiming; }

    // 에임 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun|Aiming")
    float AimFOV = 75.0f;

    // 물리 속성
    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float ThrowForce = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float LinearDamping = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Physics")
    float AngularDamping = 0.5f;
};