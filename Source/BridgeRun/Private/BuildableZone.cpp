// BuildableZone.cpp
#include "BuildableZone.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "DrawDebugHelpers.h"

ABuildableZone::ABuildableZone()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트 씬 컴포넌트 생성
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // 스플라인 컴포넌트들을 모두 루트의 직계 자식으로 생성
    LeftBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftBottomRope"));
    LeftBottomRope->SetupAttachment(RootComponent);
    LeftBottomRope->SetRelativeLocation(FVector::ZeroVector);

    RightBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightBottomRope"));
    RightBottomRope->SetupAttachment(RootComponent);
    RightBottomRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 0.0f));

    LeftTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftTopRope"));
    LeftTopRope->SetupAttachment(RootComponent);
    LeftTopRope->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

    RightTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightTopRope"));
    RightTopRope->SetupAttachment(RootComponent);
    RightTopRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 200.0f));
    // 각 로프의 초기 포인트 설정
    for (auto* Rope : { LeftBottomRope, RightBottomRope, LeftTopRope, RightTopRope })
    {
        if (Rope)
        {
            Rope->ClearSplinePoints();
            Rope->AddSplinePoint(FVector(0.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
            Rope->AddSplinePoint(FVector(1000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
            Rope->SetSplinePointType(0, ESplinePointType::Linear);
            Rope->SetSplinePointType(1, ESplinePointType::Linear);
        }
    }
}

bool ABuildableZone::IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    bool bStartValid = IsPointNearRope(StartPoint, LeftBottomRope);
    bool bEndValid = IsPointNearRope(EndPoint, RightBottomRope);

    if (!bStartValid || !bEndValid)
        return false;

    float Distance = FVector::Distance(StartPoint, EndPoint);
    float MaxAllowedLength = BridgeWidth * 1.5f;

    return Distance <= MaxAllowedLength;
}

bool ABuildableZone::IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    bool bStartValid = IsPointNearRope(StartPoint, LeftTopRope);
    bool bEndValid = IsPointNearRope(EndPoint, RightTopRope);

    return bStartValid && bEndValid;
}

float ABuildableZone::GetDistanceFromRope(const FVector& Point, USplineComponent* Rope)
{
    if (!Rope) return MAX_FLT;

    FVector ClosestPoint = Rope->FindLocationClosestToWorldLocation(Point, ESplineCoordinateSpace::World);
    return FVector::Distance(Point, ClosestPoint);
}

bool ABuildableZone::IsPointNearRope(const FVector& Point, USplineComponent* Rope, float Tolerance)
{
    return GetDistanceFromRope(Point, Rope) <= Tolerance;
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

            // 기존 메시 정리
            for (auto* Mesh : MeshArray)
            {
                if (Mesh) Mesh->DestroyComponent();
            }
            MeshArray.Empty();

            int32 NumPoints = Spline->GetNumberOfSplinePoints();
            if (NumPoints < 2) return;

            // 전체 스플라인을 따라 하나의 연속된 메시 생성
            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
            if (SplineMesh)
            {
                SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
                SplineMesh->RegisterComponent();
                SplineMesh->SetMobility(EComponentMobility::Movable);
                SplineMesh->SetStaticMesh(RopeMeshAsset);
                SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

                // 머티리얼 설정
                if (RopeMaterial)
                {
                    SplineMesh->SetMaterial(0, RopeMaterial);
                }

                // 시작점과 끝점 가져오기
                FVector StartPoint = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
                FVector EndPoint = Spline->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);
                FVector StartTangent = Spline->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
                FVector EndTangent = Spline->GetTangentAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);

                // 로프 메시 설정
                SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent);

                // RopeScale 사용하여 로프 두께 설정
                SplineMesh->SetStartScale(RopeScale);
                SplineMesh->SetEndScale(RopeScale);

                // 부모 설정
                SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
                MeshArray.Add(SplineMesh);
            }
        };

    // 각 로프 업데이트
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