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

	// 프리뷰 관련 속성 추가
	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* ValidPlacementMaterial;

	UPROPERTY(EditAnywhere, Category = "Preview Materials")
	UMaterialInterface* InvalidPlacementMaterial;

	void OnPlaced();
	void OnBulletHit();

	// 프리뷰 머티리얼 getter 함수들
	UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
	UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }
};