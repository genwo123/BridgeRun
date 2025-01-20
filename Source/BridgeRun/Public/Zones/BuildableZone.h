// Copyright BridgeRun Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableZone.generated.h"

UENUM(BlueprintType)
enum class EBuildableTeam : uint8
{
    Team1 UMETA(DisplayName = "Team 1"),
    Team2 UMETA(DisplayName = "Team 2"),
    Team3 UMETA(DisplayName = "Team 3"),
    Team4 UMETA(DisplayName = "Team 4"),
    Team5 UMETA(DisplayName = "Team 5"),
    Team6 UMETA(DisplayName = "Team 6"),
    Team7 UMETA(DisplayName = "Team 7"),
    Team8 UMETA(DisplayName = "Team 8"),
    None UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FZoneSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    bool bAllowPlankBuilding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    bool bAllowTentBuilding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    int32 CurrentRound = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings", meta = (ClampMin = "1", ClampMax = "8"))
    int32 MaxTeams = 8;
};

UCLASS()
class BRIDGERUN_API ABuildableZone : public AActor
{
    GENERATED_BODY()

public:
    ABuildableZone();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Rope Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* LeftBottomRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* RightBottomRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* LeftTopRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* RightTopRope;

    // Rope Meshes
    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> LeftBottomRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> RightBottomRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> LeftTopRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> RightTopRopeMeshes;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings", ReplicatedUsing = OnRep_ZoneSettings)
    FZoneSettings ZoneSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Zone Settings")
    TArray<EBuildableTeam> ActiveTeams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    float PlankWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    float PlankHeight = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    float BridgeWidth = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
    float MinPlankSpacing = 50.0f;

    // Visual Settings
    UPROPERTY(EditAnywhere, Category = "Visuals")
    class UStaticMesh* RopeMeshAsset;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    UMaterialInterface* RopeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FVector2D RopeScale = FVector2D(0.1f, 0.1f);

    // Building Functions
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

    // Team Management
    UFUNCTION(BlueprintCallable, Category = "Teams")
    bool IsTeamActive(EBuildableTeam Team) const;

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Teams")
    void SetTeamActive(EBuildableTeam Team, bool bActive);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Teams")
    void EliminateTeam(EBuildableTeam Team);

    // Round Management
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Rounds")
    void StartNewRound();

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Rounds")
    void EndCurrentRound();

    // Network Functions
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestPlacePlank(const FVector& StartPoint, const FVector& EndPoint, class APlayerState* RequestingPlayer);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestPlaceTent(const FVector& StartPoint, const FVector& EndPoint, class APlayerState* RequestingPlayer);

    // Visual Updates
    UFUNCTION(BlueprintCallable, Category = "Visuals")
    void UpdateSplineMeshes();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRoundStarted(int32 RoundNumber);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRoundEnded(int32 RoundNumber);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTeamEliminated(EBuildableTeam EliminatedTeam);

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION()
    void OnRep_ZoneSettings();

    // Helpers
    bool ValidateTeamAccess(class APlayerState* PlayerState) const;
    float GetDistanceFromRope(const FVector& Point, class USplineComponent* Rope) const;
    bool IsPointNearRope(const FVector& Point, class USplineComponent* Rope, float Tolerance = 50.0f) const;

private:
    void InitializeComponents();
    void SetupRopeDefaults();
    void UpdateZoneState();

    UPROPERTY(Replicated)
    TArray<FVector> PlacedPlankPositions;

    UPROPERTY(Replicated)
    TArray<FVector> PlacedTentPositions;
};