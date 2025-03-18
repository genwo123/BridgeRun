// Public/Items/Item_Tent.h
#pragma once
#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Tent.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API AItem_Tent : public AItem
{
    GENERATED_BODY()

public:
    AItem_Tent();

    // 텐트 기본 속성
    UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
    bool bIsBuiltTent;

    UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
    float DamageReduction;

    UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
    bool bBlocksVision;

    // 프리뷰 머티리얼
    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    // 네트워크 통신 함수
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnTentPlaced();

    // 체력 시스템
    UPROPERTY(EditDefaultsOnly, Replicated, Category = "Tent|Properties")
    float MaxHealth = 3.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float CurrentHealth;

    // 피격 효과
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Tent|Hit Effect")
    float GlowDuration = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Tent|Hit Effect")
    float GlowIntensity = 5.0f;

    // 액션 함수
    UFUNCTION(Server, Reliable)
    void OnPlaced();

    UFUNCTION(Server, Reliable)
    void OnBulletHit();

    // 접근자 함수
    UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
    UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 상태 관리 함수
    UFUNCTION()
    void DestroyTent();

    UFUNCTION()
    void OnRep_CurrentHealth();

    // 네트워크 동기화 함수
    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetPhysicsState();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayHitEffect();

    // 헬퍼 함수
    void SetupCollisionSettings();
    void InitializeMaterials();
    void ApplyBuiltTentState();
    void PlayGlowEffect(float Intensity);
};