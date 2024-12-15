#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "Item_Plank.h"
#include "Item_Tent.h"
#include "InvenComponent.h"
#include "BuildingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UBuildingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBuildingComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void OnBuildModeEntered();
    void DeactivateBuildMode();
    void RotateBuildPreview();
    void AttemptBuild();
    void UpdateBuildPreview();
    void ResetBuildDelay();
    void FinishBuild();  // 새로 추가된 함수

protected:
    // 건설 시스템 변수들
    UPROPERTY(EditAnywhere, Category = "Building")
    class UStaticMeshComponent* BuildPreviewMesh;

    UPROPERTY(EditAnywhere, Category = "Building")
    float MaxBuildDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Building")
    float BuildRotationStep = 15.0f;

    // 건설 아이템 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Plank> PlankClass;

    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Tent> TentClass;

    // 프리뷰용 메시
    UPROPERTY()
    UStaticMesh* PlankMesh;

    UPROPERTY()
    UStaticMesh* TentMesh;

    // 머티리얼
    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial;

    // 설치 거리 (새로 추가)
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankPlacementDistance = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentPlacementDistance = 50.0f;

    // 설치 시간 (새로 추가)
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankBuildTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentBuildTime = 2.0f;

    // 현재 선택된 건설 아이템
    UPROPERTY()
    EInventorySlot CurrentBuildingItem;

private:
    UPROPERTY()
    class ACitizen* OwnerCitizen;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    bool bCanBuildNow = true;
    bool bIsBuilding = false;  // 새로 추가
    FTimerHandle BuildDelayTimerHandle;
    bool bIsValidPlacement = false;
};