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

    DOREPLIFETIME(UBuildingComponent, BuildPreviewMesh);
    DOREPLIFETIME(UBuildingComponent, CurrentBuildingItem);
    DOREPLIFETIME(UBuildingComponent, bCanBuildNow);
    DOREPLIFETIME(UBuildingComponent, bIsBuilding);
    DOREPLIFETIME(UBuildingComponent, bIsValidPlacement);
}

void UBuildingComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCitizen = Cast<ACitizen>(GetOwner());
    // 권한 체크 제거하여 클라이언트에서도 BuildPreviewMesh 생성
    if (OwnerCitizen)
    {
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

void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    // OwnerCitizen 검사
    if (!OwnerCitizen)
        return;

    // BuildPreviewMesh가 없다면 생성
    if (!BuildPreviewMesh)
    {
        BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
        if (!BuildPreviewMesh)
            return;

        BuildPreviewMesh->SetupAttachment(OwnerCitizen->GetRootComponent());
        BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BuildPreviewMesh->SetVisibility(false);
        BuildPreviewMesh->RegisterComponent();
    }

    // 인벤토리 체크 및 아이템 설정
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
                    // 메시와 크기를 BP에서 직접 가져옴
                    BuildPreviewMesh->SetStaticMesh(DefaultPlank->MeshComponent->GetStaticMesh());
                    BuildPreviewMesh->SetWorldScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
                    BuildPreviewMesh->SetRelativeLocation(DefaultPlank->MeshComponent->GetRelativeLocation());
                    BuildPreviewMesh->SetRelativeRotation(DefaultPlank->MeshComponent->GetRelativeRotation());
                }
            }
            break;

        case EInventorySlot::Tent:
            if (TentClass)
            {
                AItem_Tent* DefaultTent = Cast<AItem_Tent>(TentClass.GetDefaultObject());
                if (DefaultTent && DefaultTent->MeshComponent)
                {
                    // 메시와 크기를 BP에서 직접 가져옴
                    BuildPreviewMesh->SetStaticMesh(DefaultTent->MeshComponent->GetStaticMesh());
                    BuildPreviewMesh->SetWorldScale3D(DefaultTent->MeshComponent->GetRelativeScale3D());
                    BuildPreviewMesh->SetRelativeLocation(DefaultTent->MeshComponent->GetRelativeLocation());
                    BuildPreviewMesh->SetRelativeRotation(DefaultTent->MeshComponent->GetRelativeRotation());
                }
            }
            break;
        }
    }

    // 프리뷰 메시 표시 및 업데이트
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(true);
        UpdateBuildPreview();
    }

    // 네트워크 상태 업데이트
    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }
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





void UBuildingComponent::DeactivateBuildMode_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
}

void UBuildingComponent::RotateBuildPreview_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !GetOwner()->HasAuthority()) return;

    FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
    NewRotation.Yaw += BuildRotationStep;
    BuildPreviewMesh->SetWorldRotation(NewRotation);
}

void UBuildingComponent::ResetBuildDelay()
{
    if (GetOwner()->HasAuthority())
    {
        bCanBuildNow = true;
    }
}



void UBuildingComponent::FinishBuild()
{
    if (GetOwner()->HasAuthority())
    {
        bIsBuilding = false;
        bCanBuildNow = true;
    }
}


void UBuildingComponent::CancelBuild()
{
    if (GetOwner()->HasAuthority())
    {
        GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);
        bIsBuilding = false;
        bCanBuildNow = true;
    }
}



bool UBuildingComponent::ValidatePlankPlacement(const FVector& Location)
{
    // 이미 있는 구조물과의 충돌 체크
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCitizen);

    return !GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_GameTraceChannel2,  // Building Channel
        FCollisionShape::MakeBox(FVector(50.0f)),
        QueryParams
    );
}

bool UBuildingComponent::ValidateTentPlacement(const FVector& Location)
{
    // 텐트 설치 유효성 검사
    return !GetWorld()->OverlapAnyTestByChannel(
        Location,
        FQuat::Identity,
        ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(100.0f)
    );
}

void UBuildingComponent::OnRep_BuildState()
{
    // 건설 가능 상태 업데이트
    if (BuildPreviewMesh)
    {
        // 건설 중일 때
        if (bIsBuilding)
        {
            BuildPreviewMesh->SetVisibility(false);

            // 물리/충돌 설정 업데이트
            if (BuildPreviewMesh->GetStaticMesh())
            {
                BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                BuildPreviewMesh->SetSimulatePhysics(false);
            }
        }
        // 건설 가능 상태일 때
        else if (bCanBuildNow)
        {
            // 프리뷰 메시 업데이트
            UpdateBuildPreview();

            // 프리뷰 메시 시각화 설정
            BuildPreviewMesh->SetVisibility(true);
            BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
        }
        // 건설 불가능 상태일 때
        else
        {
            BuildPreviewMesh->SetVisibility(false);
        }
    }

    // 소유자 업데이트
    if (OwnerCitizen)
    {
        // 건설 상태에 따른 캐릭터 이동 제한
        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(bIsBuilding ? MOVE_None : MOVE_Walking);
        }

        // UI 업데이트나 다른 시각적 피드백이 필요한 경우 여기에 추가
    }

    // 상태 변경 시 시각적/오디오 피드백
    if (!bCanBuildNow)
    {
        // 예: 건설 불가 상태 표시
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Cannot build now!"));
        }
    }

    // 컴포넌트 시각 상태 업데이트
    MarkRenderStateDirty();
}

void UBuildingComponent::MulticastOnBuildComplete_Implementation()
{
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
    bIsBuilding = false;
    bCanBuildNow = true;
}

void UBuildingComponent::AttemptBuild_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
        return;

    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 2.0f, false);

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AItem_Plank* SpawnedPlank = GetWorld()->SpawnActor<AItem_Plank>(
                PlankClass,
                Location,
                Rotation,
                SpawnParams
            );

            if (SpawnedPlank)
            {
                SpawnedPlank->SetReplicates(true);
                SpawnedPlank->SetReplicateMovement(true);
                SpawnedPlank->bIsBuiltItem = true;

                if (SpawnedPlank->MeshComponent)
                {
                    // BP의 기본 크기와 설정을 가져옴
                    AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
                    if (DefaultPlank && DefaultPlank->MeshComponent)
                    {
                        // 메시와 크기 설정을 BP 그대로 복사
                        SpawnedPlank->MeshComponent->SetStaticMesh(DefaultPlank->MeshComponent->GetStaticMesh());
                        SpawnedPlank->MeshComponent->SetWorldScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());

                        // BP의 위치와 회전도 유지
                        FTransform NewTransform = DefaultPlank->MeshComponent->GetRelativeTransform();
                        NewTransform.SetLocation(Location);
                        NewTransform.SetRotation(Rotation.Quaternion());

                        SpawnedPlank->SetActorTransform(NewTransform);
                    }

                    // 네트워크와 물리 설정
                    SpawnedPlank->MeshComponent->SetIsReplicated(true);
                    SpawnedPlank->MeshComponent->SetMobility(EComponentMobility::Movable);
                    SpawnedPlank->MeshComponent->SetSimulatePhysics(true);
                    SpawnedPlank->MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
                    SpawnedPlank->MeshComponent->SetEnableGravity(true);
                    SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                    SpawnedPlank->MeshComponent->SetGenerateOverlapEvents(true);

                    // 물리 상태 업데이트
                    SpawnedPlank->MeshComponent->WakeAllRigidBodies();
                    SpawnedPlank->MeshComponent->UpdateComponentToWorld();
                }

                // 인벤토리 체크 및 모드 전환
                if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
                {
                    FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Plank);
                    if (ItemData && ItemData->Count <= 0)
                    {
                        if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
                        {
                            DeactivateBuildMode();
                            ModeComp->SetPlayerMode(EPlayerMode::Normal);
                        }
                    }
                }

                // 네트워크 상태 업데이트
                SpawnedPlank->ForceNetUpdate();
            }
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Tent))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AItem_Tent* SpawnedTent = GetWorld()->SpawnActor<AItem_Tent>(
                TentClass,
                Location,
                Rotation,
                SpawnParams
            );

            if (SpawnedTent)
            {
                SpawnedTent->SetReplicates(true);
                SpawnedTent->SetReplicateMovement(true);
                SpawnedTent->bIsBuiltItem = true;

                if (SpawnedTent->MeshComponent)
                {
                    // BP의 기본 크기와 설정을 가져옴
                    AItem_Tent* DefaultTent = Cast<AItem_Tent>(TentClass.GetDefaultObject());
                    if (DefaultTent && DefaultTent->MeshComponent)
                    {
                        // 메시와 크기 설정을 BP 그대로 복사
                        SpawnedTent->MeshComponent->SetStaticMesh(DefaultTent->MeshComponent->GetStaticMesh());
                        SpawnedTent->MeshComponent->SetWorldScale3D(DefaultTent->MeshComponent->GetRelativeScale3D());

                        // BP의 위치와 회전도 유지
                        FTransform NewTransform = DefaultTent->MeshComponent->GetRelativeTransform();
                        NewTransform.SetLocation(Location);
                        NewTransform.SetRotation(Rotation.Quaternion());

                        SpawnedTent->SetActorTransform(NewTransform);
                    }

                    // 네트워크와 물리 설정
                    SpawnedTent->MeshComponent->SetIsReplicated(true);
                    SpawnedTent->MeshComponent->SetMobility(EComponentMobility::Movable);
                    SpawnedTent->MeshComponent->SetSimulatePhysics(true);
                    SpawnedTent->MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
                    SpawnedTent->MeshComponent->SetEnableGravity(true);
                    SpawnedTent->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedTent->MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
                    SpawnedTent->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                    SpawnedTent->MeshComponent->SetGenerateOverlapEvents(true);

                    // 물리 상태 업데이트
                    SpawnedTent->MeshComponent->WakeAllRigidBodies();
                    SpawnedTent->MeshComponent->UpdateComponentToWorld();
                }

                if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
                {
                    FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Tent);
                    if (ItemData && ItemData->Count <= 0)
                    {
                        if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
                        {
                            DeactivateBuildMode();
                            ModeComp->SetPlayerMode(EPlayerMode::Normal);
                        }
                    }
                }

                SpawnedTent->ForceNetUpdate();
            }
        }
    }

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
                // 로프 위치 가져오기
                FVector LeftStart = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
                FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
                FVector RightStart = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

                // 설치 위치가 로프 영역 내에 있는지 검사
                FVector LocalPosition = Location - LeftStart;
                FVector RopeDirection = (LeftEnd - LeftStart).GetSafeNormal();
                FVector WidthDirection = (RightStart - LeftStart).GetSafeNormal();

                float LengthProjection = FVector::DotProduct(LocalPosition, RopeDirection);
                float WidthProjection = FVector::DotProduct(LocalPosition, WidthDirection);
                float ZoneLength = FVector::Distance(LeftStart, LeftEnd);
                float ZoneWidth = FVector::Distance(LeftStart, RightStart);

                // 유효한 설치 위치인지 확인
                bool bIsValidLocation = (LengthProjection >= 0 && LengthProjection <= ZoneLength &&
                    WidthProjection >= 0 && WidthProjection <= ZoneWidth);

                if (bIsValidLocation)
                {
                    // 추가 검증: 이미 존재하는 구조물과의 충돌 체크
                    TArray<FOverlapResult> Overlaps;
                    FCollisionQueryParams QueryParams;
                    QueryParams.AddIgnoredActor(OwnerCitizen);

                    bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
                        Overlaps,
                        Location,
                        FQuat::Identity,
                        ECC_GameTraceChannel2,  // Building Channel
                        FCollisionShape::MakeBox(FVector(50.0f)),
                        QueryParams
                    );

                    return !bHasOverlap;  // 충돌이 없으면 true 반환
                }
            }
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            if (Zone->LeftTopRope && Zone->LeftBottomRope)
            {
                // 텐트 설치 검증 로직
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
                    return Distance <= Zone->BridgeWidth * 0.25f;
                }
            }
        }
    }

    return false;  // 유효한 설치 영역을 찾지 못함
}
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