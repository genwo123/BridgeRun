// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Zones/BuildableZone.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "DrawDebugHelpers.h"
#include "Engine/ActorChannel.h"

ABuildableZone::ABuildableZone()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    InitializeComponents();
    SetupRopeDefaults();
}

void ABuildableZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABuildableZone, ZoneSettings);
    DOREPLIFETIME(ABuildableZone, ActiveTeams);
    DOREPLIFETIME(ABuildableZone, PlacedPlankPositions);
    DOREPLIFETIME(ABuildableZone, PlacedTentPositions);
}

bool ABuildableZone::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    // ���ö��� ������Ʈ ����
    if (LeftBottomRope) WroteSomething |= Channel->ReplicateSubobject(LeftBottomRope, *Bunch, *RepFlags);
    if (RightBottomRope) WroteSomething |= Channel->ReplicateSubobject(RightBottomRope, *Bunch, *RepFlags);
    if (LeftTopRope) WroteSomething |= Channel->ReplicateSubobject(LeftTopRope, *Bunch, *RepFlags);
    if (RightTopRope) WroteSomething |= Channel->ReplicateSubobject(RightTopRope, *Bunch, *RepFlags);

    return WroteSomething;
}

void ABuildableZone::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // ��Ƽ�� �� �ʱ�ȭ
        if (ActiveTeams.Num() == 0)
        {
            for (int32 i = 0; i < ZoneSettings.MaxTeams; ++i)
            {
                ActiveTeams.Add(static_cast<EBuildableTeam>(i));
            }
        }
        UpdateZoneState();
    }
}

void ABuildableZone::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    UpdateSplineMeshes();

    if (GetWorld() && !IsRunningCommandlet())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugRopes();
    }
}

void ABuildableZone::OnRep_ZoneSettings()
{
    UpdateZoneState();
    UpdateSplineMeshes();
}

void ABuildableZone::InitializeComponents()
{
    // ��Ʈ ������Ʈ ����
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // ���� ������Ʈ ����
    LeftBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftBottomRope"));
    RightBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightBottomRope"));
    LeftTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftTopRope"));
    RightTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightTopRope"));

    // �� ���� ����
    SetupRopeComponents();

    // Ư�� ��ġ ����
    SetRopePositions();
}

void ABuildableZone::SetupRopeComponents()
{
    TArray<USplineComponent*> Ropes = { LeftBottomRope, RightBottomRope, LeftTopRope, RightTopRope };
    for (auto* Rope : Ropes)
    {
        if (Rope)
        {
            Rope->SetupAttachment(RootComponent);
            Rope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Rope->SetVisibility(true);
            Rope->SetCollisionProfileName(TEXT("OverlapAll"));
            Rope->SetGenerateOverlapEvents(true);
        }
    }
}

void ABuildableZone::SetRopePositions()
{
    if (RightBottomRope) RightBottomRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 0.0f));
    if (LeftTopRope) LeftTopRope->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    if (RightTopRope) RightTopRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 200.0f));
}

void ABuildableZone::SetupRopeDefaults()
{
    TArray<USplineComponent*> Ropes = { LeftBottomRope, RightBottomRope, LeftTopRope, RightTopRope };
    for (auto* Rope : Ropes)
    {
        if (Rope)
        {
            Rope->ClearSplinePoints();
            Rope->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local);
            Rope->AddSplinePoint(FVector(1000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
            Rope->SetSplinePointType(0, ESplinePointType::Linear);
            Rope->SetSplinePointType(1, ESplinePointType::Linear);
        }
    }
}

void ABuildableZone::DrawDebugRopes()
{
    auto DrawRope = [this](USplineComponent* Rope, FColor Color)
        {
            if (!Rope) return;
            for (int32 i = 0; i < Rope->GetNumberOfSplinePoints() - 1; i++)
            {
                FVector Start = Rope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
                FVector End = Rope->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
                DrawDebugLine(GetWorld(), Start, End, Color, false, -1.0f, 0, 2.0f);
            }
        };

    DrawRope(LeftBottomRope, FColor::Red);
    DrawRope(RightBottomRope, FColor::Red);
    DrawRope(LeftTopRope, FColor::Blue);
    DrawRope(RightTopRope, FColor::Blue);
}

bool ABuildableZone::IsTeamActive(EBuildableTeam Team) const
{
    return ActiveTeams.Contains(Team);
}

void ABuildableZone::SetTeamActive(EBuildableTeam Team, bool bActive)
{
    if (!HasAuthority()) return;

    if (bActive)
    {
        ActiveTeams.AddUnique(Team);
    }
    else
    {
        ActiveTeams.Remove(Team);
    }
    UpdateZoneState();
}

void ABuildableZone::EliminateTeam(EBuildableTeam Team)
{
    if (!HasAuthority()) return;

    if (ActiveTeams.Remove(Team) > 0)
    {
        OnTeamEliminated(Team);
        UpdateZoneState();
    }
}

void ABuildableZone::StartNewRound()
{
    if (!HasAuthority()) return;

    ZoneSettings.CurrentRound++;
    ZoneSettings.bIsActive = true;

    // ���� ���� ������ ����
    PlacedPlankPositions.Empty();
    PlacedTentPositions.Empty();

    UpdateZoneState();
    OnRoundStarted(ZoneSettings.CurrentRound);
}

void ABuildableZone::EndCurrentRound()
{
    if (!HasAuthority()) return;

    ZoneSettings.bIsActive = false;
    UpdateZoneState();
    OnRoundEnded(ZoneSettings.CurrentRound);
}

bool ABuildableZone::ValidateTeamAccess(APlayerState* PlayerState) const
{
    if (!PlayerState || !ZoneSettings.bIsActive) return false;

    // TODO: ������ �� ���� ���� ����
    // ����: return PlayerState->GetTeam() == CurrentTeam;
    return true;
}

bool ABuildableZone::ServerRequestPlacePlank_Validate(const FVector& StartPoint, const FVector& EndPoint, APlayerState* RequestingPlayer)
{
    return true; // �⺻ ����, �ʿ�� �߰� ���� ����
}

void ABuildableZone::ServerRequestPlacePlank_Implementation(const FVector& StartPoint, const FVector& EndPoint, APlayerState* RequestingPlayer)
{
    if (!HasAuthority() || !ValidateTeamAccess(RequestingPlayer) || !ZoneSettings.bAllowPlankBuilding)
        return;

    if (IsPlankPlacementValid(StartPoint, EndPoint))
    {
        PlacedPlankPositions.Add(StartPoint);
        PlacedPlankPositions.Add(EndPoint);
        UpdateZoneState();
    }
}

bool ABuildableZone::ServerRequestPlaceTent_Validate(const FVector& StartPoint, const FVector& EndPoint, APlayerState* RequestingPlayer)
{
    return true; // �⺻ ����, �ʿ�� �߰� ���� ����
}

void ABuildableZone::ServerRequestPlaceTent_Implementation(const FVector& StartPoint, const FVector& EndPoint, APlayerState* RequestingPlayer)
{
    if (!HasAuthority() || !ValidateTeamAccess(RequestingPlayer) || !ZoneSettings.bAllowTentBuilding)
        return;

    if (IsTentPlacementValid(StartPoint, EndPoint))
    {
        FVector TentCenter = (StartPoint + EndPoint) * 0.5f;
        PlacedTentPositions.Add(TentCenter);
        UpdateZoneState();
    }
}

bool ABuildableZone::IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    if (!LeftBottomRope || !RightBottomRope || !ZoneSettings.bIsActive)
        return false;

    // ���� ����Ʈ ��������
    FVector LeftStart = LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector LeftEnd = LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector RightStart = RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RightEnd = RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    // ���� ���� ���
    FVector LeftRopeDir = (LeftEnd - LeftStart).GetSafeNormal();
    FVector RightRopeDir = (RightEnd - RightStart).GetSafeNormal();

    // �������� ������ ���� ��� ��ġ Ȯ��
    FVector PointToLeftStart = StartPoint - LeftStart;
    FVector PointToRightStart = EndPoint - RightStart;

    float LeftProjection = FVector::DotProduct(PointToLeftStart, LeftRopeDir);
    float RightProjection = FVector::DotProduct(PointToRightStart, RightRopeDir);

    float LeftRopeLength = FVector::Distance(LeftStart, LeftEnd);
    float RightRopeLength = FVector::Distance(RightStart, RightEnd);

    // ���� ���� ���� �ִ��� Ȯ��
    bool bInLeftRopeRange = (LeftProjection >= 0 && LeftProjection <= LeftRopeLength);
    bool bInRightRopeRange = (RightProjection >= 0 && RightProjection <= RightRopeLength);

    // �������� �Ÿ� ���
    FVector LeftProjectedPoint = LeftStart + LeftRopeDir * LeftProjection;
    FVector RightProjectedPoint = RightStart + RightRopeDir * RightProjection;

    float LeftDistance = FVector::Distance(StartPoint, LeftProjectedPoint);
    float RightDistance = FVector::Distance(EndPoint, RightProjectedPoint);

    // ���� ���ڿ��� ���� Ȯ��
    if (!CheckPlankSpacing(StartPoint, EndPoint))
        return false;

    return bInLeftRopeRange && bInRightRopeRange &&
        LeftDistance <= BridgeWidth * 0.5f &&
        RightDistance <= BridgeWidth * 0.5f;
}

bool ABuildableZone::CheckPlankSpacing(const FVector& StartPoint, const FVector& EndPoint)
{
    for (int32 i = 0; i < PlacedPlankPositions.Num(); i += 2)
    {
        FVector ExistingStart = PlacedPlankPositions[i];
        FVector ExistingEnd = PlacedPlankPositions[i + 1];

        float StartDist = FVector::Distance(StartPoint, ExistingStart);
        float EndDist = FVector::Distance(EndPoint, ExistingEnd);

        if (StartDist < MinPlankSpacing || EndDist < MinPlankSpacing)
            return false;
    }
    return true;
}

bool ABuildableZone::IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    if (!LeftTopRope || !LeftBottomRope || !RightTopRope || !RightBottomRope || !ZoneSettings.bIsActive)
        return false;

    // ��Ʈ �߾��� ���
    FVector TentCenter = (StartPoint + EndPoint) * 0.5f;

    // ��Ʈ�� ���� ��� - ���Ʒ� �������� ��ġ�Ǿ�� ��
    FVector TentDirection = (EndPoint - StartPoint).GetSafeNormal();

    // ���� ���� Ȯ��
    bool bLeftValid = CheckTentAlignment(TentCenter, TentDirection, LeftTopRope, LeftBottomRope);

    // ������ ������ ���ؼ��� ���� ����
    bool bRightValid = CheckTentAlignment(TentCenter, TentDirection, RightTopRope, RightBottomRope);

    // �� ������ ��Ȯ�� ��ġ�ؾ߸� ��ȿ
    return (bLeftValid || bRightValid) && ZoneSettings.bAllowTentBuilding;
}

bool ABuildableZone::CheckTentAlignment(const FVector& TentCenter, const FVector& TentDirection,
    USplineComponent* TopRope, USplineComponent* BottomRope)
{
    if (!TopRope || !BottomRope)
        return false;

    FVector TopPoint = TopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector BottomPoint = BottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();

    // ��Ʈ ����� ���� ������ ���� �������� Ȯ�� (��Ʈ�� ������ ���� ������ ���ؾ� ��)
    float DirAlignment = FMath::Abs(FVector::DotProduct(TentDirection, RopeDir));

    // ��Ʈ ��ġ�� ���� ���� ���� ��Ȯ�� �ִ��� Ȯ��
    FVector PointToRope = TentCenter - BottomPoint;
    FVector CrossDir = FVector::CrossProduct(RopeDir, PointToRope).GetSafeNormal();
    float Distance = FVector::DotProduct(PointToRope, CrossDir);

    // ���� �Ϻ��ϰ� ���� ���� �ִ��� Ȯ�� (���� ���� ��� ����)
    return (FMath::Abs(Distance) <= 5.0f) && (DirAlignment >= 0.95f);
}

FBox ABuildableZone::CalculateBuildableArea() const
{
    FBox Area(ForceInit);

    // ��� ������ �������� ����Ͽ� ���� ���
    if (LeftBottomRope && RightBottomRope && LeftTopRope && RightTopRope)
    {
        for (int32 i = 0; i < LeftBottomRope->GetNumberOfSplinePoints(); i++)
        {
            Area += LeftBottomRope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
            Area += RightBottomRope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
            Area += LeftTopRope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
            Area += RightTopRope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        }
    }

    // �ణ �� ū �������� Ȯ��
    return Area.ExpandBy(50.0f);
}

void ABuildableZone::UpdateSplineMeshes()
{
    if (!RopeMeshAsset) return;

    UpdateRopeMeshSet(LeftBottomRope, LeftBottomRopeMeshes);
    UpdateRopeMeshSet(RightBottomRope, RightBottomRopeMeshes);
    UpdateRopeMeshSet(LeftTopRope, LeftTopRopeMeshes);
    UpdateRopeMeshSet(RightTopRope, RightTopRopeMeshes);
}

void ABuildableZone::UpdateRopeMeshSet(USplineComponent* Spline, TArray<USplineMeshComponent*>& MeshArray)
{
    if (!Spline) return;

    // ���� �޽� ����
    for (auto* Mesh : MeshArray)
    {
        if (Mesh) Mesh->DestroyComponent();
    }
    MeshArray.Empty();

    // �� �޽� ����
    USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
    if (SplineMesh)
    {
        SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
        SplineMesh->RegisterComponent();
        SplineMesh->SetMobility(EComponentMobility::Movable);
        SplineMesh->SetStaticMesh(RopeMeshAsset);
        SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // ��Ƽ���� ����
        if (RopeMaterial)
        {
            SplineMesh->SetMaterial(0, RopeMaterial);
        }

        // �������� ���� ����
        FVector StartPoint = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
        FVector EndPoint = Spline->GetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
        FVector StartTangent = Spline->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
        FVector EndTangent = Spline->GetTangentAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);

        // ���ö��� �޽� ����
        SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent);
        SplineMesh->SetStartScale(FVector2D(RopeScale.X, RopeScale.Y));
        SplineMesh->SetEndScale(FVector2D(RopeScale.X, RopeScale.Y));

        // �θ� ����
        SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
        MeshArray.Add(SplineMesh);
    }
}

float ABuildableZone::GetDistanceFromRope(const FVector& Point, USplineComponent* Rope) const
{
    if (!Rope) return MAX_FLT;

    FVector Start = Rope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector End = Rope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    FVector RopeDir = (End - Start).GetSafeNormal();
    FVector PointToStart = Point - Start;
    float Projection = FVector::DotProduct(PointToStart, RopeDir);

    if (Projection >= 0 && Projection <= FVector::Distance(Start, End))
    {
        FVector ProjectedPoint = Start + RopeDir * Projection;
        return FVector::Distance(Point, ProjectedPoint);
    }

    return MAX_FLT;
}

bool ABuildableZone::IsPointNearRope(const FVector& Point, USplineComponent* Rope, float Tolerance) const
{
    if (!Rope) return false;

    float Distance = GetDistanceFromRope(Point, Rope);
    return Distance <= Tolerance && Distance != MAX_FLT;
}

void ABuildableZone::UpdateZoneState()
{
    // ��Ƽ�� ���¿� ���� ���ü� ������Ʈ
    bool bShouldBeVisible = ZoneSettings.bIsActive && ActiveTeams.Num() > 0;

    UpdateRopeVisibility(bShouldBeVisible);

    // �޽� ������Ʈ
    if (bShouldBeVisible)
    {
        UpdateSplineMeshes();
    }
    else
    {
        ClearAllRopeMeshes();
    }
}

void ABuildableZone::UpdateRopeVisibility(bool bVisible)
{
    TArray<USplineComponent*> Ropes = { LeftBottomRope, RightBottomRope, LeftTopRope, RightTopRope };
    for (auto* Rope : Ropes)
    {
        if (Rope)
        {
            Rope->SetVisibility(bVisible);
            Rope->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }
    }
}

void ABuildableZone::ClearAllRopeMeshes()
{
    // ��Ȱ�� ������ �� ��� �޽� ����
    for (auto* Mesh : LeftBottomRopeMeshes) if (Mesh) Mesh->DestroyComponent();
    for (auto* Mesh : RightBottomRopeMeshes) if (Mesh) Mesh->DestroyComponent();
    for (auto* Mesh : LeftTopRopeMeshes) if (Mesh) Mesh->DestroyComponent();
    for (auto* Mesh : RightTopRopeMeshes) if (Mesh) Mesh->DestroyComponent();

    LeftBottomRopeMeshes.Empty();
    RightBottomRopeMeshes.Empty();
    LeftTopRopeMeshes.Empty();
    RightTopRopeMeshes.Empty();
}