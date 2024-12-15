// BuildingComponent.cpp
#include "BuildingComponent.h"
#include "Citizen.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "BuildableZone.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "Item_Plank.h"
#include "Item_Tent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerModeComponent.h"  

UBuildingComponent::UBuildingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBuildingComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCitizen = Cast<ACitizen>(GetOwner());
    if (OwnerCitizen)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Initializing BuildPreviewMesh"));
        BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
        if (BuildPreviewMesh)
        {
            BuildPreviewMesh->SetupAttachment(OwnerCitizen->GetRootComponent());
            BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            BuildPreviewMesh->SetVisibility(false);
            BuildPreviewMesh->RegisterComponent();

            if (PlankClass)
            {
                AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
                if (DefaultPlank && DefaultPlank->MeshComponent)
                {
                    PlankMesh = DefaultPlank->MeshComponent->GetStaticMesh();
                    FVector PlankScale = DefaultPlank->MeshComponent->GetRelativeScale3D();
                    BuildPreviewMesh->SetRelativeScale3D(PlankScale);
                }
            }
        }
    }
    bCanBuildNow = true;
}

void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (BuildPreviewMesh && BuildPreviewMesh->IsVisible())
    {
        UpdateBuildPreview();
    }
}

void UBuildingComponent::OnBuildModeEntered()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();

        switch (CurrentBuildingItem)
        {
        case EInventorySlot::Plank:
            if (PlankClass)
            {
                AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
                if (DefaultPlank && DefaultPlank->MeshComponent)
                {
                    BuildPreviewMesh->SetStaticMesh(DefaultPlank->MeshComponent->GetStaticMesh());
                    BuildPreviewMesh->SetRelativeScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
                }
            }
            break;

        case EInventorySlot::Tent:
            if (TentClass)
            {
                AItem_Tent* DefaultTent = Cast<AItem_Tent>(TentClass.GetDefaultObject());
                if (DefaultTent && DefaultTent->MeshComponent)
                {
                    BuildPreviewMesh->SetStaticMesh(DefaultTent->MeshComponent->GetStaticMesh());
                    BuildPreviewMesh->SetRelativeScale3D(DefaultTent->MeshComponent->GetRelativeScale3D());
                }
            }
            break;
        }
    }

    BuildPreviewMesh->SetVisibility(true);
}

void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    APlayerController* PC = Cast<APlayerController>(OwnerCitizen->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    FVector PlayerLocation = OwnerCitizen->GetActorLocation();
    float CurrentPreviewDistance = (CurrentBuildingItem == EInventorySlot::Plank) ?
        PlankPlacementDistance : TentPlacementDistance;

    FVector PreviewLocation = PlayerLocation + (CameraForward * CurrentPreviewDistance);
    FRotator PreviewRotation = CameraRotation;
    PreviewRotation.Pitch = 0.0f;
    PreviewRotation.Roll = 0.0f;

    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableZone::StaticClass(), FoundZones);

    bIsValidPlacement = false;

    for (AActor* Actor : FoundZones)
    {
        ABuildableZone* Zone = Cast<ABuildableZone>(Actor);
        if (!Zone) continue;

        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            if (Zone->LeftBottomRope && Zone->RightBottomRope)
            {
                FVector LeftStart = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector RightStart = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector RightEnd = Zone->RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

                FVector LocalPosition = PreviewLocation - LeftStart;
                FVector RopeDirection = (LeftEnd - LeftStart).GetSafeNormal();
                FVector WidthDirection = (RightStart - LeftStart).GetSafeNormal();

                float LengthProjection = FVector::DotProduct(LocalPosition, RopeDirection);
                float WidthProjection = FVector::DotProduct(LocalPosition, WidthDirection);
                float ZoneLength = FVector::Distance(LeftStart, LeftEnd);
                float ZoneWidth = FVector::Distance(LeftStart, RightStart);

                bIsValidPlacement = (LengthProjection >= 0 && LengthProjection <= ZoneLength &&
                    WidthProjection >= 0 && WidthProjection <= ZoneWidth);

                if (bIsValidPlacement)
                {
                    PreviewLocation.Z = LeftStart.Z;
                    BuildPreviewMesh->SetWorldLocation(PreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PreviewRotation);
                    BuildPreviewMesh->SetMaterial(0, ValidPlacementMaterial);
                }
            }
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            if (Zone->LeftTopRope && Zone->LeftBottomRope && Zone->RightTopRope && Zone->RightBottomRope)
            {
                // 왼쪽 로프와의 거리 계산
                FVector LeftBottom = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector ToLeftStart = PreviewLocation - LeftBottom;
                FVector LeftDir = (LeftEnd - LeftBottom).GetSafeNormal();
                float LeftProj = FVector::DotProduct(ToLeftStart, LeftDir);
                LeftProj = FMath::Clamp(LeftProj, 0.0f, FVector::Distance(LeftBottom, LeftEnd));
                FVector LeftClosest = LeftBottom + LeftDir * LeftProj;

                // 오른쪽 로프와의 거리 계산
                FVector RightBottom = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector RightEnd = Zone->RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector ToRightStart = PreviewLocation - RightBottom;
                FVector RightDir = (RightEnd - RightBottom).GetSafeNormal();
                float RightProj = FVector::DotProduct(ToRightStart, RightDir);
                RightProj = FMath::Clamp(RightProj, 0.0f, FVector::Distance(RightBottom, RightEnd));
                FVector RightClosest = RightBottom + RightDir * RightProj;

                // 2D 거리 계산 (Z 값 제외)
                FVector ToLeft = PreviewLocation - LeftClosest;
                FVector ToRight = PreviewLocation - RightClosest;
                ToLeft.Z = 0;
                ToRight.Z = 0;
                float DistToLeft = ToLeft.Size();
                float DistToRight = ToRight.Size();

                // 가까운 쪽 선택
                bool bUseLeftSide = DistToLeft < DistToRight;
                FVector BottomPoint = bUseLeftSide ? LeftClosest : RightClosest;
                USplineComponent* TopRope = bUseLeftSide ? Zone->LeftTopRope : Zone->RightTopRope;
                float Ratio = bUseLeftSide ? (LeftProj / FVector::Distance(LeftBottom, LeftEnd)) :
                    (RightProj / FVector::Distance(RightBottom, RightEnd));

                // 상단 위치 계산
                FVector TopStart = TopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector TopEnd = TopRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector TopPoint = FMath::Lerp(TopStart, TopEnd, Ratio);

                // 최종 스냅 위치 계산
                FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
                FVector VerticalOffset = RopeDir * (TopPoint - BottomPoint).Size() * 0.5f;
                FVector SnapLocation = BottomPoint + VerticalOffset;

                // 플레이어와의 거리 체크
                FVector PlayerToSnap = SnapLocation - PlayerLocation;
                PlayerToSnap.Z = 0;
                float DistanceToPlayer = PlayerToSnap.Size();

                bIsValidPlacement = (DistanceToPlayer >= 100.0f && DistanceToPlayer <= MaxBuildDistance);

                FRotator SnapRotation = bUseLeftSide ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);
                BuildPreviewMesh->SetWorldLocation(SnapLocation);
                BuildPreviewMesh->SetWorldRotation(SnapRotation);
                BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
            }
        }

        if (!bIsValidPlacement)
        {
            BuildPreviewMesh->SetWorldLocation(PreviewLocation);
            BuildPreviewMesh->SetWorldRotation(PreviewRotation);
            BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
        }

        if (bIsValidPlacement) break;
    }

    BuildPreviewMesh->SetVisibility(true);
}
void UBuildingComponent::AttemptBuild()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding)
        return;

    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(
        BuildDelayTimerHandle,
        this,
        &UBuildingComponent::ResetBuildDelay,
        2.0f,
        false
    );

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;

            AItem_Plank* SpawnedPlank = GetWorld()->SpawnActor<AItem_Plank>(
                PlankClass,
                Location,
                Rotation,
                SpawnParams
            );

            if (SpawnedPlank)
            {
                SpawnedPlank->bIsBuiltItem = true;
                if (SpawnedPlank->MeshComponent)
                {
                    SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
                    SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                    SpawnedPlank->MeshComponent->SetMobility(EComponentMobility::Static);
                }

                // 아이템 개수 체크
                if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
                {
                    FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Plank);
                    if (ItemData && ItemData->Count <= 0)
                    {
                        if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
                        {
                            DeactivateBuildMode();
                            ModeComp->SetPlayerMode(EPlayerMode::Normal);
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No more planks. Switching to Normal mode."));
                        }
                    }
                }
            }
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Tent))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;

            AItem_Tent* SpawnedTent = GetWorld()->SpawnActor<AItem_Tent>(
                TentClass,
                Location,
                Rotation,
                SpawnParams
            );

            if (SpawnedTent)
            {
                SpawnedTent->bIsBuiltItem = true;
                if (SpawnedTent->MeshComponent)
                {
                    SpawnedTent->MeshComponent->SetSimulatePhysics(false);
                    SpawnedTent->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedTent->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                    SpawnedTent->MeshComponent->SetMobility(EComponentMobility::Static);
                }

                // 아이템 개수 체크
                if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
                {
                    FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Tent);
                    if (ItemData && ItemData->Count <= 0)
                    {
                        if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
                        {
                            DeactivateBuildMode();
                            ModeComp->SetPlayerMode(EPlayerMode::Normal);
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No more tents. Switching to Normal mode."));
                        }
                    }
                }
            }
        }
    }

    if (bIsValidPlacement)
    {
        bIsBuilding = false;
        bCanBuildNow = true;
    }
}

void UBuildingComponent::DeactivateBuildMode()
{
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
}

void UBuildingComponent::RotateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen) return;

    FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
    NewRotation.Yaw += BuildRotationStep;
    BuildPreviewMesh->SetWorldRotation(NewRotation);
}

void UBuildingComponent::ResetBuildDelay()
{
    bCanBuildNow = true;
}
