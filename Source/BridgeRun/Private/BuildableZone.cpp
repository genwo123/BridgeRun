// BuildableZone.cpp
#include "BuildableZone.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "DrawDebugHelpers.h"

ABuildableZone::ABuildableZone()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    LeftBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftBottomRope"));
    LeftBottomRope->SetupAttachment(RootComponent);
    LeftBottomRope->SetRelativeLocation(FVector::ZeroVector);
    LeftBottomRope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LeftBottomRope->SetVisibility(true);
    LeftBottomRope->ComponentTags.Add(FName("LeftRope"));
    LeftBottomRope->SetCollisionProfileName(TEXT("OverlapAll"));

    RightBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightBottomRope"));
    RightBottomRope->SetupAttachment(RootComponent);
    RightBottomRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 0.0f));
    RightBottomRope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RightBottomRope->SetVisibility(true);
    RightBottomRope->ComponentTags.Add(FName("RightRope"));
    RightBottomRope->SetCollisionProfileName(TEXT("OverlapAll"));

    LeftTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftTopRope"));
    LeftTopRope->SetupAttachment(RootComponent);
    LeftTopRope->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    LeftTopRope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LeftTopRope->SetVisibility(true);
    LeftTopRope->ComponentTags.Add(FName("LeftRope"));
    LeftTopRope->SetCollisionProfileName(TEXT("OverlapAll"));

    RightTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightTopRope"));
    RightTopRope->SetupAttachment(RootComponent);
    RightTopRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 200.0f));
    RightTopRope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RightTopRope->SetVisibility(true);
    RightTopRope->ComponentTags.Add(FName("RightRope"));
    RightTopRope->SetCollisionProfileName(TEXT("OverlapAll"));

    for (auto* Rope : { LeftBottomRope, RightBottomRope, LeftTopRope, RightTopRope })
    {
        if (Rope)
        {
            Rope->ClearSplinePoints();
            Rope->AddSplinePoint(FVector(0.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
            Rope->AddSplinePoint(FVector(1000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
            Rope->SetSplinePointType(0, ESplinePointType::Linear);
            Rope->SetSplinePointType(1, ESplinePointType::Linear);
            Rope->SetGenerateOverlapEvents(true);
        }
    }
}

bool ABuildableZone::IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    if (!LeftBottomRope || !RightBottomRope) return false;

    // ���� ����/���� ��������
    FVector LeftStart = LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector LeftEnd = LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector RightStart = RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RightEnd = RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    // ���� ���� ����
    FVector LeftRopeDir = (LeftEnd - LeftStart).GetSafeNormal();
    FVector RightRopeDir = (RightEnd - RightStart).GetSafeNormal();

    // �������� ������ ������ ���� ��ó�� �ִ��� Ȯ��
    FVector PointToLeftStart = StartPoint - LeftStart;
    FVector PointToRightStart = EndPoint - RightStart;

    float LeftProjection = FVector::DotProduct(PointToLeftStart, LeftRopeDir);
    float RightProjection = FVector::DotProduct(PointToRightStart, RightRopeDir);

    float LeftRopeLength = FVector::Distance(LeftStart, LeftEnd);
    float RightRopeLength = FVector::Distance(RightStart, RightEnd);

    // ������ ���� ���� ���� �ִ��� Ȯ��
    bool bInLeftRopeRange = (LeftProjection >= 0 && LeftProjection <= LeftRopeLength);
    bool bInRightRopeRange = (RightProjection >= 0 && RightProjection <= RightRopeLength);

    // �����κ����� ���� �Ÿ� ���
    FVector LeftProjectedPoint = LeftStart + LeftRopeDir * LeftProjection;
    FVector RightProjectedPoint = RightStart + RightRopeDir * RightProjection;

    float LeftDistance = FVector::Distance(StartPoint, LeftProjectedPoint);
    float RightDistance = FVector::Distance(EndPoint, RightProjectedPoint);

    // ��ġ ���� ���� Ȯ��
    bool bValidLeft = bInLeftRopeRange && (LeftDistance <= BridgeWidth * 0.5f);
    bool bValidRight = bInRightRopeRange && (RightDistance <= BridgeWidth * 0.5f);

    return bValidLeft && bValidRight;
}

bool ABuildableZone::IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    if (!LeftTopRope || !LeftBottomRope || !RightTopRope || !RightBottomRope) return false;

    // ���� ��ġ
    bool bLeftValid = false;
    {
        FVector TopPoint = LeftTopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
        FVector BottomPoint = LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
        FVector TentPos = (StartPoint + EndPoint) * 0.5f;

        FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
        FVector PointToBottom = TentPos - BottomPoint;
        float Projection = FVector::DotProduct(PointToBottom, RopeDir);

        if (Projection >= 0 && Projection <= FVector::Distance(TopPoint, BottomPoint))
        {
            FVector ProjectedPoint = BottomPoint + RopeDir * Projection;
            float Distance = FVector::Distance(TentPos, ProjectedPoint);
            bLeftValid = Distance <= BridgeWidth * 0.25f;
        }
    }

    // ������ ��ġ
    bool bRightValid = false;
    {
        FVector TopPoint = RightTopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
        FVector BottomPoint = RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
        FVector TentPos = (StartPoint + EndPoint) * 0.5f;

        FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
        FVector PointToBottom = TentPos - BottomPoint;
        float Projection = FVector::DotProduct(PointToBottom, RopeDir);

        if (Projection >= 0 && Projection <= FVector::Distance(TopPoint, BottomPoint))
        {
            FVector ProjectedPoint = BottomPoint + RopeDir * Projection;
            float Distance = FVector::Distance(TentPos, ProjectedPoint);
            bRightValid = Distance <= BridgeWidth * 0.25f;
        }
    }

    return bLeftValid || bRightValid;
}

float ABuildableZone::GetDistanceFromRope(const FVector& Point, USplineComponent* Rope)
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

bool ABuildableZone::IsPointNearRope(const FVector& Point, USplineComponent* Rope, float Tolerance)
{
    if (!Rope) return false;

    float Distance = GetDistanceFromRope(Point, Rope);
    return Distance <= Tolerance && Distance != MAX_FLT;
}

void ABuildableZone::UpdateSplineMeshes()
{
    if (!RopeMeshAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("RopeMeshAsset is not set!"));
        return;
    }

    auto UpdateRopeMeshes = [this](USplineComponent* Spline, TArray<USplineMeshComponent*>& MeshArray)
        {
            if (!Spline) return;

            // ���� �޽� ����
            for (auto* Mesh : MeshArray)
            {
                if (Mesh) Mesh->DestroyComponent();
            }
            MeshArray.Empty();

            int32 NumPoints = Spline->GetNumberOfSplinePoints();
            if (NumPoints < 2) return;

            // ��ü ���ö����� ���� �ϳ��� ���ӵ� �޽� ����
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

                // �������� ���� ��������
                FVector StartPoint = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
                FVector EndPoint = Spline->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);
                FVector StartTangent = Spline->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
                FVector EndTangent = Spline->GetTangentAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);

                // ���� �޽� ����
                SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent);

                // RopeScale ����Ͽ� ���� �β� ����
                SplineMesh->SetStartScale(RopeScale);
                SplineMesh->SetEndScale(RopeScale);

                // �θ� ����
                SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
                MeshArray.Add(SplineMesh);
            }
        };

    // �� ���� ������Ʈ
    UpdateRopeMeshes(LeftBottomRope, LeftBottomRopeMeshes);
    UpdateRopeMeshes(RightBottomRope, RightBottomRopeMeshes);
    UpdateRopeMeshes(LeftTopRope, LeftTopRopeMeshes);
    UpdateRopeMeshes(RightTopRope, RightTopRopeMeshes);
}

void ABuildableZone::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UpdateSplineMeshes();

    if (GetWorld() && !IsRunningCommandlet())
    {
        FlushPersistentDebugLines(GetWorld());
        auto DrawRope = [this](USplineComponent* Rope, FColor Color)
            {
                if (!Rope) return;
                for (int32 i = 0; i < Rope->GetNumberOfSplinePoints() - 1; i++)
                {
                    FVector Start = Rope->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
                    FVector End = Rope->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
                    DrawDebugLine(GetWorld(), Start, End, Color, false, 0.0f);
                }
            };

        DrawRope(LeftBottomRope, FColor::Red);
        DrawRope(RightBottomRope, FColor::Red);
        DrawRope(LeftTopRope, FColor::Blue);
        DrawRope(RightTopRope, FColor::Blue);
    }
}