#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "InvenComponent.h"
#include "Item_Plank.h"
#include "Item_Tent.h"
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
	void FinishBuild();

protected:
	// ������ ����
	UPROPERTY(EditAnywhere, Category = "Building|Preview")
	class UStaticMeshComponent* BuildPreviewMesh;

	UPROPERTY(EditAnywhere, Category = "Building|Preview")
	UMaterialInterface* ValidPlacementMaterial;

	UPROPERTY(EditAnywhere, Category = "Building|Preview")
	UMaterialInterface* InvalidPlacementMaterial;

	// ������� �޽� ĳ��
	UPROPERTY()
	UStaticMesh* PlankMesh;

	UPROPERTY()
	UStaticMesh* TentMesh;

	// �Ǽ� ������ Ŭ����
	UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
	TSubclassOf<AItem_Plank> PlankClass;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
	TSubclassOf<AItem_Tent> TentClass;

	// �Ǽ� ����
	UPROPERTY(EditAnywhere, Category = "Building|Settings")
	float MaxBuildDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Building|Settings")
	float BuildRotationStep = 15.0f;

	// �����ۺ� ����
	UPROPERTY(EditAnywhere, Category = "Building|Plank")
	float PlankPlacementDistance = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Building|Plank")
	float PlankBuildTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Building|Tent")
	float TentPlacementDistance = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Building|Tent")
	float TentBuildTime = 2.0f;

	// ���� ���õ� ������
	UPROPERTY()
	EInventorySlot CurrentBuildingItem;

private:
	UPROPERTY()
	class ACitizen* OwnerCitizen;

	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	bool bCanBuildNow = true;
	bool bIsBuilding = false;
	bool bIsValidPlacement = false;
	FTimerHandle BuildDelayTimerHandle;
};