// Item_Plank.h
#pragma once
#include "CoreMinimal.h"
#include "Item.h"
#include "Item_Plank.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API AItem_Plank : public AItem
{
    GENERATED_BODY()
public:
    AItem_Plank();

    UPROPERTY(EditAnywhere, Category = "Plank")
    bool bIsBuiltPlank;

    UPROPERTY(EditAnywhere, Category = "Plank")
    float MaxPlankLength;

    // 프리뷰 관련 속성 추가
    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    void OnPlaced();

    // 프리뷰 머티리얼 getter 함수들
    UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
    UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }
};