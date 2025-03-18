// Public/Item/Item_Plank.h
#pragma once
#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item_Plank.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API AItem_Plank : public AItem
{
    GENERATED_BODY()

public:
    AItem_Plank();

    // 판자 상태 속성
    UPROPERTY(ReplicatedUsing = OnRep_IsBuilt, EditAnywhere, Category = "Plank")
    bool bIsBuiltPlank;

    UPROPERTY(Replicated, EditAnywhere, Category = "Plank")
    float MaxPlankLength;

    // 프리뷰 머티리얼
    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Preview Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    // 액션 함수
    UFUNCTION(Server, Reliable)
    void OnPlaced();

    // 네트워크 동기화 함수
    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetPlankPhysicsState(EComponentMobility::Type NewMobility);

    // 접근자 함수
    UMaterialInterface* GetValidPlacementMaterial() const { return ValidPlacementMaterial; }
    UMaterialInterface* GetInvalidPlacementMaterial() const { return InvalidPlacementMaterial; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 복제 이벤트
    UFUNCTION()
    void OnRep_IsBuilt();

    // 헬퍼 함수
    void SetupCollisionSettings();
    void ApplyBuiltPlankState();
};