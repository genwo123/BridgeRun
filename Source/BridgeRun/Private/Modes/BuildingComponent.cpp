// Private/Modes/Components/BuildingComponent.cpp
#include "Modes/BuildingComponent.h"
#include "Characters/Citizen.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Zones/BuildableZone.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Modes/PlayerModeComponent.h"

UBuildingComponent::UBuildingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UBuildingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ���� �켱���� ����
    DOREPLIFETIME(UBuildingComponent, bIsValidPlacement);
    DOREPLIFETIME(UBuildingComponent, BuildPreviewMesh);
    DOREPLIFETIME(UBuildingComponent, CurrentBuildingItem);
    DOREPLIFETIME(UBuildingComponent, bCanBuildNow);
    DOREPLIFETIME(UBuildingComponent, bIsBuilding);
}

void UBuildingComponent::BeginPlay()
{
    Super::BeginPlay();

    // ���� ���۷��� ��������
    OwnerCitizen = Cast<ACitizen>(GetOwner());

    // ������ �޽� ������Ʈ �ʱ�ȭ
    InitializeBuildPreviewMesh();

    // �ʱ� ���� ����
    bCanBuildNow = true;
}

void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ������ �޽ð� ���� ���� ��ġ ������Ʈ
    if (BuildPreviewMesh && BuildPreviewMesh->IsVisible())
    {
        UpdateBuildPreview();
    }
}

// ������ �޽� ������Ʈ �ʱ�ȭ
void UBuildingComponent::InitializeBuildPreviewMesh()
{
    BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetupAttachment(OwnerCitizen->GetRootComponent());
        BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BuildPreviewMesh->SetVisibility(false);
        BuildPreviewMesh->SetIsReplicated(true);
        BuildPreviewMesh->bOnlyOwnerSee = false;
        BuildPreviewMesh->SetEnableGravity(false);
        BuildPreviewMesh->RegisterComponent();

        // �⺻ �÷�ũ �޽� ��������
        if (PlankClass)
        {
            AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
            if (DefaultPlank && DefaultPlank->MeshComponent)
            {
                PlankMesh = DefaultPlank->MeshComponent->GetStaticMesh();
                BuildPreviewMesh->SetRelativeScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
            }
        }
    }
}

// �Ǽ� ��� ���� ó��
void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    if (!OwnerCitizen)
        return;

    // ������ �޽ð� ������ �ʱ�ȭ
    if (!BuildPreviewMesh)
    {
        InitializeBuildPreviewMesh();
        if (!BuildPreviewMesh)
            return;
    }

    // ���� ���õ� �����ۿ� ���� �޽� ����
    SetupPreviewMeshForCurrentItem();

    // ������ ���� �ʱ�ȭ �� �ð�ȭ
    if (BuildPreviewMesh)
    {
        bIsValidPlacement = false;
        BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
        BuildPreviewMesh->SetVisibility(true);
        UpdateBuildPreview();
    }

    // ��Ʈ��ũ ������Ʈ
    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }
}

// ���� ���õ� �����ۿ� �°� ������ �޽� ����
void UBuildingComponent::SetupPreviewMeshForCurrentItem()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();

        switch (CurrentBuildingItem)
        {
        case EInventorySlot::Plank:
            SetupPlankPreviewMesh();
            break;

        case EInventorySlot::Tent:
            SetupTentPreviewMesh();
            break;
        }
    }
}

// �κ��� ������ �޽� ����
void UBuildingComponent::SetupPlankPreviewMesh()
{
    if (!BuildPreviewMesh || !PlankClass)
        return;

    AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
    if (DefaultPlank && DefaultPlank->MeshComponent)
    {
        BuildPreviewMesh->SetStaticMesh(DefaultPlank->MeshComponent->GetStaticMesh());
        BuildPreviewMesh->SetWorldScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
        BuildPreviewMesh->SetRelativeLocation(DefaultPlank->MeshComponent->GetRelativeLocation());
        BuildPreviewMesh->SetRelativeRotation(DefaultPlank->MeshComponent->GetRelativeRotation());
    }
}

// ��Ʈ ������ �޽� ����
void UBuildingComponent::SetupTentPreviewMesh()
{
    if (!BuildPreviewMesh || !TentClass)
        return;

    AItem_Tent* DefaultTent = Cast<AItem_Tent>(TentClass.GetDefaultObject());
    if (DefaultTent && DefaultTent->MeshComponent)
    {
        BuildPreviewMesh->SetStaticMesh(DefaultTent->MeshComponent->GetStaticMesh());
        BuildPreviewMesh->SetWorldScale3D(DefaultTent->MeshComponent->GetRelativeScale3D());
        BuildPreviewMesh->SetRelativeLocation(DefaultTent->MeshComponent->GetRelativeLocation());
        BuildPreviewMesh->SetRelativeRotation(DefaultTent->MeshComponent->GetRelativeRotation());
    }
}

// ������ �޽� ������Ʈ
void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // ī�޶� ���� ��������
    AController* Controller = OwnerCitizen->GetController();
    if (!Controller) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    // �÷��̾� ���� ��ġ ���
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();
    float CurrentPreviewDistance = (CurrentBuildingItem == EInventorySlot::Plank) ?
        PlankPlacementDistance : TentPlacementDistance;

    // ��� ������ ���� �� �Ҵ�
    PreviewLocation = PlayerLocation + (CameraForward * CurrentPreviewDistance);
    PreviewRotation = CameraRotation;
    PreviewRotation.Pitch = 0.0f;
    PreviewRotation.Roll = 0.0f;

    // �Ǽ� ���� ���� �˻� (������Ʈ�� �Լ� �ñ״�ó ���)
    bool NewValidPlacement = DetermineValidPlacement(PreviewLocation, PreviewRotation);

    // ���������� ���� ������Ʈ
    if (GetOwner()->HasAuthority())
    {
        if (bIsValidPlacement != NewValidPlacement)
        {
            bIsValidPlacement = NewValidPlacement;
            GetOwner()->ForceNetUpdate();
        }
    }

    // �ð��� �ǵ�� ������Ʈ
    UpdatePreviewVisuals(NewValidPlacement);
}

// ������ �ð� ȿ�� ������Ʈ
void UBuildingComponent::UpdatePreviewVisuals(bool bValid)
{
    if (!BuildPreviewMesh)
        return;

    // ��ġ �� ȸ�� ����
    BuildPreviewMesh->SetWorldLocation(PreviewLocation);
    BuildPreviewMesh->SetWorldRotation(PreviewRotation);

    // ���� �÷��̾��� ��� ��� �ð� ȿ�� ����
    if (OwnerCitizen->IsLocallyControlled())
    {
        BuildPreviewMesh->SetMaterial(0, bValid ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }
    else
    {
        // ������ ���� ������� �ð� ȿ�� ����
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    BuildPreviewMesh->SetVisibility(true);
}

// �Ǽ� ���� ��ġ ����
bool UBuildingComponent::DetermineValidPlacement(FVector& InLocation, FRotator& InRotation)
{
    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableZone::StaticClass(), FoundZones);

    for (AActor* Actor : FoundZones)
    {
        ABuildableZone* Zone = Cast<ABuildableZone>(Actor);
        if (!Zone) continue;

        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            // InLocation �Ű����� ���
            if (ValidatePlankZonePlacement(Zone, InLocation))
                return true;
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            // InLocation, InRotation �Ű����� ���
            if (ValidateTentZonePlacement(Zone, InLocation, InRotation))
                return true;
        }
    }

    return false;
}


// �κ��� ��ġ ���� ���� ����
bool UBuildingComponent::ValidatePlankZonePlacement(ABuildableZone* Zone, FVector& InLocation)
{
    // ���� ���� ����, PreviewLocation -> InLocation ����
    if (!Zone->LeftBottomRope || !Zone->RightBottomRope)
        return false;

    FVector LeftStart = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector RightStart = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RightEnd = Zone->RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    FVector LocalPosition = InLocation - LeftStart;
    FVector RopeDirection = (LeftEnd - LeftStart).GetSafeNormal();
    FVector WidthDirection = (RightStart - LeftStart).GetSafeNormal();

    float LengthProjection = FVector::DotProduct(LocalPosition, RopeDirection);
    float WidthProjection = FVector::DotProduct(LocalPosition, WidthDirection);
    float ZoneLength = FVector::Distance(LeftStart, LeftEnd);
    float ZoneWidth = FVector::Distance(LeftStart, RightStart);

    bool bIsInZone = (LengthProjection >= 0 && LengthProjection <= ZoneLength &&
        WidthProjection >= 0 && WidthProjection <= ZoneWidth);

    if (bIsInZone)
    {
        InLocation.Z = LeftStart.Z;
        return true;
    }

    return false;
}

// ��Ʈ ��ġ ���� ���� ����
bool UBuildingComponent::ValidateTentZonePlacement(ABuildableZone* Zone, FVector& InLocation, FRotator& InRotation)
{
    if (!Zone->LeftTopRope || !Zone->LeftBottomRope || !Zone->RightTopRope || !Zone->RightBottomRope)
        return false;

    FVector LeftBottom = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector RightBottom = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RightEnd = Zone->RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    // ���� �������� ���� ����� ���� ã��
    FVector ToLeftStart = PreviewLocation - LeftBottom;
    FVector LeftDir = (LeftEnd - LeftBottom).GetSafeNormal();
    float LeftProj = FVector::DotProduct(ToLeftStart, LeftDir);
    LeftProj = FMath::Clamp(LeftProj, 0.0f, FVector::Distance(LeftBottom, LeftEnd));
    FVector LeftClosest = LeftBottom + LeftDir * LeftProj;

    // ������ �������� ���� ����� ���� ã��
    FVector ToRightStart = PreviewLocation - RightBottom;
    FVector RightDir = (RightEnd - RightBottom).GetSafeNormal();
    float RightProj = FVector::DotProduct(ToRightStart, RightDir);
    RightProj = FMath::Clamp(RightProj, 0.0f, FVector::Distance(RightBottom, RightEnd));
    FVector RightClosest = RightBottom + RightDir * RightProj;

    // �� ����� �� ����
    FVector ToLeft = PreviewLocation - LeftClosest;
    FVector ToRight = PreviewLocation - RightClosest;
    ToLeft.Z = 0;
    ToRight.Z = 0;
    float DistToLeft = ToLeft.Size();
    float DistToRight = ToRight.Size();

    bool bUseLeftSide = DistToLeft < DistToRight;
    FVector BottomPoint = bUseLeftSide ? LeftClosest : RightClosest;
    USplineComponent* TopRope = bUseLeftSide ? Zone->LeftTopRope : Zone->RightTopRope;
    float Ratio = bUseLeftSide ? (LeftProj / FVector::Distance(LeftBottom, LeftEnd)) :
        (RightProj / FVector::Distance(RightBottom, RightEnd));

    FVector TopStart = TopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector TopEnd = TopRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector TopPoint = FMath::Lerp(TopStart, TopEnd, Ratio);

    // ���� ���� �� ��ġ ���
    FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
    FVector VerticalOffset = RopeDir * (TopPoint - BottomPoint).Size() * 0.5f;
    FVector SnapLocation = BottomPoint + VerticalOffset;

    // �÷��̾���� �Ÿ� �˻�
    FVector PlayerToSnap = SnapLocation - OwnerCitizen->GetActorLocation();
    PlayerToSnap.Z = 0;
    float DistanceToPlayer = PlayerToSnap.Size();

    bool bValidDistance = (DistanceToPlayer >= 100.0f && DistanceToPlayer <= MaxBuildDistance);

    if (bValidDistance)
    {
        // ���� ��ġ �� ȸ�� ����
        FRotator SnapRotation = bUseLeftSide ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);
        PreviewRotation = SnapRotation;
        PreviewLocation = SnapLocation;
        return true;
    }

    return false;
}



// �Ǽ� ��� ��Ȱ��ȭ
void UBuildingComponent::DeactivateBuildMode_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
}

// ������ ȸ��
void UBuildingComponent::RotateBuildPreview_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !GetOwner()->HasAuthority()) return;

    FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
    NewRotation.Yaw += BuildRotationStep;
    BuildPreviewMesh->SetWorldRotation(NewRotation);
}

// �Ǽ� �õ�
void UBuildingComponent::AttemptBuild_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
        return;

    // �Ǽ� ������ ����
    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 2.0f, false);

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // ������ ������ ���� �Ǽ� ó��
    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            SpawnBuildingItem<AItem_Plank>(PlankClass, Location, Rotation);
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Tent))
        {
            SpawnBuildingItem<AItem_Tent>(TentClass, Location, Rotation);
        }
    }

    // �Ǽ� ���� ������Ʈ
    if (bIsValidPlacement)
    {
        bIsBuilding = false;
        bCanBuildNow = true;
        MulticastOnBuildComplete();

        if (GetOwner())
        {
            GetOwner()->ForceNetUpdate();
        }

        if (BuildPreviewMesh)
        {
            BuildPreviewMesh->MarkRenderStateDirty();
        }
    }
}

// �Ǽ� �������� ���� �� �浹 ����
void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, const FVector& Location, const FRotator& Rotation)
{
    if (!MeshComp)
        return;

    // ����/�浹 ����
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetEnableGravity(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // ���� ���� ���� ����
    MeshComp->SetIsReplicated(true);
    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->bReplicatePhysicsToAutonomousProxy = true;

    // ��ġ ������ ���� ����
    MeshComp->SetWorldLocation(Location);
    MeshComp->SetWorldRotation(Rotation);
}

// �Ǽ� �� �κ��丮 ���� Ȯ��
void UBuildingComponent::CheckInventoryAfterBuilding(AItem* BuiltItem)
{
    if (!BuiltItem || !OwnerCitizen)
        return;

    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        FItemData* ItemData = InvenComp->GetItemData(CurrentBuildingItem);
        if (ItemData && ItemData->Count <= 0)
        {
            if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
            {
                DeactivateBuildMode();
                ModeComp->SetPlayerMode(EPlayerMode::Normal);
            }
        }
    }

    // ��Ʈ��ũ ������Ʈ ����
    BuiltItem->ForceNetUpdate();
}

// �Ǽ� ������ �ʱ�ȭ
void UBuildingComponent::ResetBuildDelay()
{
    if (GetOwner()->HasAuthority())
    {
        bCanBuildNow = true;
    }
}

// �Ǽ� Ÿ�̸� ����
void UBuildingComponent::StartBuildTimer(float BuildTime)
{
    if (GetOwner()->HasAuthority())
    {
        bIsBuilding = true;
        GetWorld()->GetTimerManager().SetTimer(
            BuildTimerHandle,
            this,
            &UBuildingComponent::FinishBuild,
            BuildTime,
            false
        );
    }
}

// �Ǽ� �Ϸ�
void UBuildingComponent::FinishBuild()
{
    if (GetOwner()->HasAuthority())
    {
        bIsBuilding = false;
        bCanBuildNow = true;
    }
}

// �Ǽ� ���
void UBuildingComponent::CancelBuild()
{
    if (GetOwner()->HasAuthority())
    {
        GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);
        bIsBuilding = false;
        bCanBuildNow = true;
    }
}

// �κ��� ��ġ ����
bool UBuildingComponent::ValidatePlankPlacement(const FVector& Location)
{
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCitizen);

    // �÷�ũ ���� ä���� ����Ͽ� �ٸ� �÷�ũ���� �浹�� üũ
    FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(50.0f));
    return !GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_GameTraceChannel2, // Plank ���� ä�� ���
        BoxShape,
        QueryParams
    );
}

// ��Ʈ ��ġ ����
bool UBuildingComponent::ValidateTentPlacement(const FVector& Location)
{
    // ��Ʈ ��ġ ��ȿ�� �˻�
    return !GetWorld()->OverlapAnyTestByChannel(
        Location,
        FQuat::Identity,
        ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(100.0f)
    );
}

// �Ǽ� ��ġ ����
bool UBuildingComponent::ValidateBuildLocation(const FVector& Location)
{
    if (!GetOwner()->HasAuthority()) return false;

    TArray<AActor*> BuildableZones;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableZone::StaticClass(), BuildableZones);

    for (AActor* Actor : BuildableZones)
    {
        ABuildableZone* Zone = Cast<ABuildableZone>(Actor);
        if (!Zone) continue;

        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            if (Zone->LeftBottomRope && Zone->RightBottomRope)
            {
                // ���� ��ġ ��������
                FVector LeftStart = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector RightStart = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

                // ��ġ ��ġ�� ���� ���� ���� �ִ��� �˻�
                FVector LocalPosition = Location - LeftStart;
                FVector RopeDirection = (LeftEnd - LeftStart).GetSafeNormal();
                FVector WidthDirection = (RightStart - LeftStart).GetSafeNormal();

                float LengthProjection = FVector::DotProduct(LocalPosition, RopeDirection);
                float WidthProjection = FVector::DotProduct(LocalPosition, WidthDirection);
                float ZoneLength = FVector::Distance(LeftStart, LeftEnd);
                float ZoneWidth = FVector::Distance(LeftStart, RightStart);

                // ��ȿ�� ��ġ ��ġ���� Ȯ��
                bool bIsValidLocation = (LengthProjection >= 0 && LengthProjection <= ZoneLength &&
                    WidthProjection >= 0 && WidthProjection <= ZoneWidth);

                if (bIsValidLocation)
                {
                    return ValidatePlankPlacement(Location);
                }
            }
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            if (Zone->LeftTopRope && Zone->LeftBottomRope)
            {
                // ��Ʈ ��ġ ���� ����
                FVector TopPoint = Zone->LeftTopRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector BottomPoint = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

                FVector TentPos = Location;
                FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
                FVector PointToBottom = TentPos - BottomPoint;
                float Projection = FVector::DotProduct(PointToBottom, RopeDir);

                if (Projection >= 0 && Projection <= FVector::Distance(TopPoint, BottomPoint))
                {
                    FVector ProjectedPoint = BottomPoint + RopeDir * Projection;
                    float Distance = FVector::Distance(TentPos, ProjectedPoint);

                    if (Distance <= Zone->BridgeWidth * 0.25f)
                    {
                        return ValidateTentPlacement(Location);
                    }
                }
            }
        }
    }

    return false;  // ��ȿ�� ��ġ ������ ã�� ����
}

// �Ǽ� ���� ���� ó��
void UBuildingComponent::OnRep_BuildState()
{
    // �Ǽ� ���� ���� ������Ʈ
    if (BuildPreviewMesh)
    {
        // �Ǽ� ���� ��
        if (bIsBuilding)
        {
            BuildPreviewMesh->SetVisibility(false);

            // ����/�浹 ���� ������Ʈ
            if (BuildPreviewMesh->GetStaticMesh())
            {
                BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                BuildPreviewMesh->SetSimulatePhysics(false);
            }
        }
        // �Ǽ� ���� ������ ��
        else if (bCanBuildNow)
        {
            // ������ �޽� ������Ʈ
            UpdateBuildPreview();

            // ������ �޽� �ð�ȭ ����
            BuildPreviewMesh->SetVisibility(true);
            BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
        }
        // �Ǽ� �Ұ��� ������ ��
        else
        {
            BuildPreviewMesh->SetVisibility(false);
        }
    }

    // ������ ���� ������Ʈ
    UpdateOwnerBuildState();
}

// ������ �Ǽ� ���� ������Ʈ
void UBuildingComponent::UpdateOwnerBuildState()
{
    if (!OwnerCitizen)
        return;

    // �Ǽ� ���¿� ���� ĳ���� �̵� ����
    if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
    {
        MovementComp->SetMovementMode(bIsBuilding ? MOVE_None : MOVE_Walking);
    }

    // �ð� �ǵ��
    if (!bCanBuildNow && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Cannot build now!"));
    }

    // ������Ʈ �ð� ���� ������Ʈ
    MarkRenderStateDirty();
}

// ������ �޽� ���� ó��
void UBuildingComponent::OnRep_BuildPreviewMesh()
{
    if (!BuildPreviewMesh)
        return;

    BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BuildPreviewMesh->SetEnableGravity(false);
    BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);

    if (BuildPreviewMesh->GetStaticMesh())
    {
        BuildPreviewMesh->SetVisibility(true);
    }
}

// ��ȿ ��ġ ���� ���� ó��
void UBuildingComponent::OnRep_ValidPlacement()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // ���� �÷��̾ �ƴ� ��쿡�� ��Ƽ���� ������Ʈ
    if (!OwnerCitizen->IsLocallyControlled())
    {
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }
}

// �Ǽ� �Ϸ� ��Ƽĳ��Ʈ
void UBuildingComponent::MulticastOnBuildComplete_Implementation()
{
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
        BuildPreviewMesh->SetVisibility(true);
        UpdateBuildPreview();
    }
    bIsBuilding = false;
    bCanBuildNow = true;
}