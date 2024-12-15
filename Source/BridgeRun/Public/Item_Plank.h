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

    // ������ ���� �Ӽ� �߰�
    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    void OnPlaced();

    // ������ ��Ƽ���� getter �Լ���
    UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
    UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }
};