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

	UFUNCTION(Server, Reliable)
	void OnPlaced();

	UFUNCTION(Server, Reliable)
	void OnBulletHit();

	UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
	UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};