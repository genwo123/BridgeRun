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
    // ������ �� �⺻ �Լ�
    ABuildableZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    // ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // ���� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* LeftBottomRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* RightBottomRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* LeftTopRope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Ropes")
    class USplineComponent* RightTopRope;

    // ���� �޽�
    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> LeftBottomRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> RightBottomRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> LeftTopRopeMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Components|Visuals")
    TArray<class USplineMeshComponent*> RightTopRopeMeshes;

    // ����
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

    // �ð��� ����
    UPROPERTY(EditAnywhere, Category = "Visuals")
    class UStaticMesh* RopeMeshAsset;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    UMaterialInterface* RopeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FVector2D RopeScale = FVector2D(0.1f, 0.1f);

    // �Ǽ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

    FBox CalculateBuildableArea() const;

    // �� ����
    UFUNCTION(BlueprintCallable, Category = "Teams")
    bool IsTeamActive(EBuildableTeam Team) const;

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Teams")
    void SetTeamActive(EBuildableTeam Team, bool bActive);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Teams")
    void EliminateTeam(EBuildableTeam Team);

    // ���� ����
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Rounds")
    void StartNewRound();

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Rounds")
    void EndCurrentRound();

    // ��Ʈ��ũ �Լ�
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestPlacePlank(const FVector& StartPoint, const FVector& EndPoint, class APlayerState* RequestingPlayer);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestPlaceTent(const FVector& StartPoint, const FVector& EndPoint, class APlayerState* RequestingPlayer);

    // �ð��� ������Ʈ
    UFUNCTION(BlueprintCallable, Category = "Visuals")
    void UpdateSplineMeshes();

    // �̺�Ʈ
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

    // ����� �Լ�
    bool ValidateTeamAccess(class APlayerState* PlayerState) const;
    float GetDistanceFromRope(const FVector& Point, class USplineComponent* Rope) const;
    bool IsPointNearRope(const FVector& Point, class USplineComponent* Rope, float Tolerance = 50.0f) const;
    bool CheckPlankSpacing(const FVector& StartPoint, const FVector& EndPoint);
    bool CheckTentAlignment(const FVector& TentCenter, const FVector& TentDirection,
        USplineComponent* TopRope, USplineComponent* BottomRope);

private:
    // �ʱ�ȭ �� ����
    void InitializeComponents();
    void SetupRopeComponents();
    void SetRopePositions();
    void SetupRopeDefaults();
    void DrawDebugRopes();

    // ���� ������Ʈ
    void UpdateZoneState();
    void UpdateRopeVisibility(bool bVisible);
    void UpdateRopeMeshSet(USplineComponent* Spline, TArray<USplineMeshComponent*>& MeshArray);
    void ClearAllRopeMeshes();

    // ������ ����
    UPROPERTY(Replicated)
    TArray<FVector> PlacedPlankPositions;

    UPROPERTY(Replicated)
    TArray<FVector> PlacedTentPositions;
};