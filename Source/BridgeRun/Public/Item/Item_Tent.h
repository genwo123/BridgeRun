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

	UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
	bool bIsBuiltTent;

	UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
	float DamageReduction;

	UPROPERTY(Replicated, EditAnywhere, Category = "Tent")
	bool bBlocksVision;

	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* ValidPlacementMaterial;

	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* InvalidPlacementMaterial;

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

	// Functions
	UFUNCTION(Server, Reliable)
	void OnPlaced();

	UFUNCTION(Server, Reliable)
	void OnBulletHit();

	UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
	UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION()
	void DestroyTent();

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetPhysicsState();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitEffect();
};