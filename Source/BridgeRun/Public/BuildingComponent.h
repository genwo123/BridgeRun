#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
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

protected:
    // �Ǽ� �ý��� ������
    UPROPERTY(EditAnywhere, Category = "Building")
    class UStaticMeshComponent* BuildPreviewMesh;

    UPROPERTY(EditAnywhere, Category = "Building")
    float MaxBuildDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Building")
    float BuildRotationStep = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial;

    // �Ǽ� ������ Ŭ����
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem> PlankClass;

    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem> TentClass;

    // ������� �޽�
    UPROPERTY()
    UStaticMesh* PlankMesh;

    UPROPERTY()
    UStaticMesh* TentMesh;

    // ���� ���õ� �Ǽ� ������
    EInventorySlot CurrentBuildingItem;

    bool IsValidBuildLocation(const FVector& Location) const;

private:
    UPROPERTY()
    class ACitizen* OwnerCitizen;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    bool bCanBuildNow = true;
    FTimerHandle BuildDelayTimerHandle;
    bool bIsValidPlacement = false;
};