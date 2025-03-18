// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Item.h"
#include "Modes/InvenComponent.h" 
#include "Item/Item_Plank.h"
#include "Item/Item_Tent.h"
#include "BuildingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UBuildingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBuildingComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Network RPCs
    UFUNCTION(Server, Reliable)
    void OnBuildModeEntered();
    UFUNCTION(Server, Reliable)
    void DeactivateBuildMode();
    UFUNCTION(Server, Reliable)
    void RotateBuildPreview();
    UFUNCTION(Server, Reliable)
    void AttemptBuild();

    // Building Functions
    void UpdateBuildPreview();
    void StartBuildTimer(float BuildTime);
    void CancelBuild();
    bool ValidateBuildLocation(const FVector& Location);
    void FinishBuild();

protected:
    // Implementation ÇÔ¼öµé
    virtual void AttemptBuild_Implementation();
    virtual void OnBuildModeEntered_Implementation();
    virtual void DeactivateBuildMode_Implementation();
    virtual void RotateBuildPreview_Implementation();
    virtual void MulticastOnBuildComplete_Implementation();

    // Components
    UPROPERTY(ReplicatedUsing = OnRep_BuildPreviewMesh)
    class UStaticMeshComponent* BuildPreviewMesh;

    // Materials
    UPROPERTY(EditAnywhere, Category = "Building|Preview")
    UMaterialInterface* ValidPlacementMaterial;
    UPROPERTY(EditAnywhere, Category = "Building|Preview")
    UMaterialInterface* InvalidPlacementMaterial;

    // Meshes
    UPROPERTY()
    UStaticMesh* PlankMesh;
    UPROPERTY()
    UStaticMesh* TentMesh;

    // Item Classes
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Plank> PlankClass;
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Tent> TentClass;

    // Settings
    UPROPERTY(EditAnywhere, Category = "Building|Settings")
    float MaxBuildDistance = 300.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Settings")
    float BuildRotationStep = 15.0f;

    // Plank Settings
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankPlacementDistance = 50.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankBuildTime = 2.0f;

    // Tent Settings
    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentPlacementDistance = 50.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentBuildTime = 2.0f;

    // Replicated States
    UPROPERTY(Replicated)
    EInventorySlot CurrentBuildingItem;

    // Visual Feedback
    UPROPERTY(EditAnywhere, Category = "Building|Feedback")
    class USoundCue* BuildingBlockedSound;
    UPROPERTY(EditAnywhere, Category = "Building|Feedback")
    class UParticleSystem* BuildingBlockedEffect;

private:
    // References
    UPROPERTY()
    class ACitizen* OwnerCitizen;
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Build States
    UPROPERTY(ReplicatedUsing = OnRep_BuildState)
    bool bCanBuildNow = true;
    UPROPERTY(ReplicatedUsing = OnRep_BuildState)
    bool bIsBuilding = false;
    UPROPERTY(ReplicatedUsing = OnRep_ValidPlacement)
    bool bIsValidPlacement = false;

    // Timers
    FTimerHandle BuildDelayTimerHandle;
    FTimerHandle BuildTimerHandle;

    // Functions
    UFUNCTION()
    void OnRep_BuildState();
    UFUNCTION()
    void OnRep_BuildPreviewMesh();
    UFUNCTION()
    void OnRep_ValidPlacement();
    bool ValidatePlankPlacement(const FVector& Location);
    bool ValidateTentPlacement(const FVector& Location);
    void ResetBuildDelay();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnBuildComplete();
};