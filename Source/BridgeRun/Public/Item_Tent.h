// Item_Tent.h
#pragma once
#include "CoreMinimal.h"
#include "Item.h"
#include "Item_Tent.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API AItem_Tent : public AItem
{
	GENERATED_BODY()
public:
	AItem_Tent();

	UPROPERTY(EditAnywhere, Category = "Tent")
	bool bIsBuiltTent;

	UPROPERTY(EditAnywhere, Category = "Tent")
	float DamageReduction;

	UPROPERTY(EditAnywhere, Category = "Tent")
	bool bBlocksVision;

	// ������ ���� �Ӽ� �߰�
	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* ValidPlacementMaterial;

	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* InvalidPlacementMaterial;

	void OnPlaced();
	void OnBulletHit();

	// ������ ��Ƽ���� getter �Լ���
	UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
	UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }
};