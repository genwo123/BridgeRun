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
    void FinishBuild();  // ���� �߰��� �Լ�

protected:
    // �Ǽ� �ý��� ������
    UPROPERTY(EditAnywhere, Category = "Building")
    class UStaticMeshComponent* BuildPreviewMesh;

    UPROPERTY(EditAnywhere, Category = "Building")
    float MaxBuildDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Building")
    float BuildRotationStep = 15.0f;

    // �Ǽ� ������ Ŭ����
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Plank> PlankClass;

    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Tent> TentClass;

    // ������� �޽�
    UPROPERTY()
    UStaticMesh* PlankMesh;

    UPROPERTY()
    UStaticMesh* TentMesh;

    // ��Ƽ����
    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial;

    // ��ġ �Ÿ� (���� �߰�)
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankPlacementDistance = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentPlacementDistance = 50.0f;

    // ��ġ �ð� (���� �߰�)
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankBuildTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentBuildTime = 2.0f;

    // ���� ���õ� �Ǽ� ������
    UPROPERTY()
    EInventorySlot CurrentBuildingItem;

private:
    UPROPERTY()
    class ACitizen* OwnerCitizen;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    bool bCanBuildNow = true;
    bool bIsBuilding = false;  // ���� �߰�
    FTimerHandle BuildDelayTimerHandle;
    bool bIsValidPlacement = false;
};