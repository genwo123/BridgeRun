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

    // 복제 우선순위 설정
    DOREPLIFETIME(UBuildingComponent, bIsValidPlacement);
    DOREPLIFETIME(UBuildingComponent, BuildPreviewMesh);
    DOREPLIFETIME(UBuildingComponent, CurrentBuildingItem);
    DOREPLIFETIME(UBuildingComponent, bCanBuildNow);
    DOREPLIFETIME(UBuildingComponent, bIsBuilding);

    // 새로 추가
    DOREPLIFETIME(UBuildingComponent, FixedBuildLocation);
    DOREPLIFETIME(UBuildingComponent, FixedBuildRotation);
}

void UBuildingComponent::BeginPlay()
{
    Super::BeginPlay();

    // 오너 레퍼런스 가져오기
    OwnerCitizen = Cast<ACitizen>(GetOwner());

    // 프리뷰 메시 컴포넌트 초기화
    InitializeBuildPreviewMesh();

    // 초기 상태 설정
    bCanBuildNow = true;
}

void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 프리뷰 메시가 보일 때만 위치 업데이트
    if (BuildPreviewMesh && BuildPreviewMesh->IsVisible())
    {
        UpdateBuildPreview();
    }

    // 건설 중일 때 처리 (조건 단순화)
    if (bIsBuilding && GetOwner()->HasAuthority())
    {
        // 매 프레임마다 로그는 너무 많으니 조건부로만
        static float LastLogTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();

        if (CurrentTime - LastLogTime > 0.5f) // 0.5초마다 로그
        {
            UE_LOG(LogTemp, Error, TEXT("=== TICK: bIsBuilding=TRUE, HasAuthority=TRUE ==="));
            LastLogTime = CurrentTime;
        }

        float BuildTime = (CurrentBuildingItem == EInventorySlot::Plank) ? PlankBuildTime : TentBuildTime;
        float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(BuildTimerHandle);

        if (RemainingTime > 0.0f)
        {
            float NewProgress = 1.0f - (RemainingTime / BuildTime);
            NewProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);

            // 진행도 변경 시에만 Delegate 호출
            if (FMath::Abs(CurrentBuildProgress - NewProgress) > 0.01f)
            {
                CurrentBuildProgress = NewProgress;
                UE_LOG(LogTemp, Error, TEXT("=== BROADCASTING PROGRESS: %.2f, RemainingTime: %.2f ==="),
                    CurrentBuildProgress, RemainingTime);
                OnBuildProgressChanged.Broadcast(CurrentBuildProgress, bIsBuilding);
            }
        }

        // 플레이어 이동 감지
        if (OwnerCitizen)
        {
            FVector CurrentPlayerLocation = OwnerCitizen->GetActorLocation();
            float MovementDistance = FVector::Distance(BuildStartPlayerLocation, CurrentPlayerLocation);

            if (MovementDistance > MaxMovementDuringBuild)
            {
                UE_LOG(LogTemp, Warning, TEXT("플레이어가 너무 멀리 이동하여 건설 취소! 거리: %.1fcm"), MovementDistance);
                CancelBuild();
                return;
            }
        }
    }

    // 건설 중에는 프리뷰를 고정 위치에 표시
    if (bIsBuilding && BuildPreviewMesh)
    {
        BuildPreviewMesh->SetWorldLocation(FixedBuildLocation);
        BuildPreviewMesh->SetWorldRotation(FixedBuildRotation);
        BuildPreviewMesh->SetVisibility(true);

        if (ValidPlacementMaterial)
        {
            BuildPreviewMesh->SetMaterial(0, ValidPlacementMaterial);
        }
    }
}
// 프리뷰 메시 컴포넌트 초기화
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

        // 기본 플랭크 메시 가져오기
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

// 건설 모드 진입 처리
void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    if (!OwnerCitizen)
        return;

    // 프리뷰 메시가 없으면 초기화
    if (!BuildPreviewMesh)
    {
        InitializeBuildPreviewMesh();
        if (!BuildPreviewMesh)
            return;
    }

    // 현재 선택된 아이템에 따라 메시 설정
    SetupPreviewMeshForCurrentItem();

    // 프리뷰 상태 초기화 및 시각화
    if (BuildPreviewMesh)
    {
        bIsValidPlacement = false;
        BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
        BuildPreviewMesh->SetVisibility(true);
        UpdateBuildPreview();
    }

    // 네트워크 업데이트
    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }
}

// 현재 선택된 아이템에 맞게 프리뷰 메시 설정
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

// 널빤지 프리뷰 메시 설정
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

// 텐트 프리뷰 메시 설정
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

// 프리뷰 메시 업데이트
void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // 카메라 시점 가져오기
    AController* Controller = OwnerCitizen->GetController();
    if (!Controller) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    // 플레이어 기준 위치 계산
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();
    float CurrentPreviewDistance = (CurrentBuildingItem == EInventorySlot::Plank) ?
        PlankPlacementDistance : TentPlacementDistance;

    // 멤버 변수에 직접 값 할당
    PreviewLocation = PlayerLocation + (CameraForward * CurrentPreviewDistance);
    PreviewRotation = CameraRotation;
    PreviewRotation.Pitch = 0.0f;
    PreviewRotation.Roll = 0.0f;

    // 건설 가능 구역 검사 (업데이트된 함수 시그니처 사용)
    bool NewValidPlacement = DetermineValidPlacement(PreviewLocation, PreviewRotation);

    // 서버에서만 상태 업데이트
    if (GetOwner()->HasAuthority())
    {
        if (bIsValidPlacement != NewValidPlacement)
        {
            bIsValidPlacement = NewValidPlacement;
            GetOwner()->ForceNetUpdate();
        }
    }

    // 시각적 피드백 업데이트
    UpdatePreviewVisuals(NewValidPlacement);
}

// 프리뷰 시각 효과 업데이트
void UBuildingComponent::UpdatePreviewVisuals(bool bValid)
{
    if (!BuildPreviewMesh)
        return;

    // 위치 및 회전 적용
    BuildPreviewMesh->SetWorldLocation(PreviewLocation);
    BuildPreviewMesh->SetWorldRotation(PreviewRotation);

    // 로컬 플레이어일 경우 즉시 시각 효과 적용
    if (OwnerCitizen->IsLocallyControlled())
    {
        BuildPreviewMesh->SetMaterial(0, bValid ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }
    else
    {
        // 복제된 상태 기반으로 시각 효과 적용
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    BuildPreviewMesh->SetVisibility(true);
}

// 건설 가능 위치 판정
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
            // InLocation 매개변수 사용
            if (ValidatePlankZonePlacement(Zone, InLocation))
                return true;
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            // InLocation, InRotation 매개변수 사용
            if (ValidateTentZonePlacement(Zone, InLocation, InRotation))
                return true;
        }
    }

    return false;
}


// 널빤지 배치 가능 영역 검증
bool UBuildingComponent::ValidatePlankZonePlacement(ABuildableZone* Zone, FVector& InLocation)
{
    // 기존 로직 유지, PreviewLocation -> InLocation 변경
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

// 텐트 배치 가능 영역 검증
bool UBuildingComponent::ValidateTentZonePlacement(ABuildableZone* Zone, FVector& InLocation, FRotator& InRotation)
{
    if (!Zone->LeftTopRope || !Zone->LeftBottomRope || !Zone->RightTopRope || !Zone->RightBottomRope)
        return false;

    FVector LeftBottom = Zone->LeftBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector LeftEnd = Zone->LeftBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
    FVector RightBottom = Zone->RightBottomRope->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    FVector RightEnd = Zone->RightBottomRope->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

    // 왼쪽 로프에서 가장 가까운 지점 찾기
    FVector ToLeftStart = PreviewLocation - LeftBottom;
    FVector LeftDir = (LeftEnd - LeftBottom).GetSafeNormal();
    float LeftProj = FVector::DotProduct(ToLeftStart, LeftDir);
    LeftProj = FMath::Clamp(LeftProj, 0.0f, FVector::Distance(LeftBottom, LeftEnd));
    FVector LeftClosest = LeftBottom + LeftDir * LeftProj;

    // 오른쪽 로프에서 가장 가까운 지점 찾기
    FVector ToRightStart = PreviewLocation - RightBottom;
    FVector RightDir = (RightEnd - RightBottom).GetSafeNormal();
    float RightProj = FVector::DotProduct(ToRightStart, RightDir);
    RightProj = FMath::Clamp(RightProj, 0.0f, FVector::Distance(RightBottom, RightEnd));
    FVector RightClosest = RightBottom + RightDir * RightProj;

    // 더 가까운 쪽 선택
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

    // 로프 방향 및 위치 계산
    FVector RopeDir = (TopPoint - BottomPoint).GetSafeNormal();
    FVector VerticalOffset = RopeDir * (TopPoint - BottomPoint).Size() * 0.5f;
    FVector SnapLocation = BottomPoint + VerticalOffset;

    // 플레이어와의 거리 검사
    FVector PlayerToSnap = SnapLocation - OwnerCitizen->GetActorLocation();
    PlayerToSnap.Z = 0;
    float DistanceToPlayer = PlayerToSnap.Size();

    bool bValidDistance = (DistanceToPlayer >= 100.0f && DistanceToPlayer <= MaxBuildDistance);

    if (bValidDistance)
    {
        // 스냅 위치 및 회전 설정
        FRotator SnapRotation = bUseLeftSide ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);
        PreviewRotation = SnapRotation;
        PreviewLocation = SnapLocation;
        return true;
    }

    return false;
}



// 건설 모드 비활성화
void UBuildingComponent::DeactivateBuildMode_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
}

// 프리뷰 회전
void UBuildingComponent::RotateBuildPreview_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !GetOwner()->HasAuthority()) return;

    FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
    NewRotation.Yaw += BuildRotationStep;
    BuildPreviewMesh->SetWorldRotation(NewRotation);
}

// 건설 시도
void UBuildingComponent::AttemptBuild_Implementation()
{
    UE_LOG(LogTemp, Error, TEXT("=== AttemptBuild_Implementation STARTED ==="));

    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("=== AttemptBuild VALIDATION FAILED ==="));
        return;
    }

    // 아이템 보유 여부 체크
    bool bHasItem = false;
    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Plank);
            bHasItem = ItemData && ItemData->Count > 0;
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Tent);
            bHasItem = ItemData && ItemData->Count > 0;
        }
    }

    if (!bHasItem)
    {
        UE_LOG(LogTemp, Error, TEXT("=== NO ITEM AVAILABLE ==="));
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("=== BUILDING START PROCESS ==="));

    // 건설 시작 - 위치/회전 고정
    bIsBuilding = true;
    CurrentBuildProgress = 0.0f;

    // 현재 프리뷰 위치/회전을 고정
    FixedBuildLocation = BuildPreviewMesh->GetComponentLocation();
    FixedBuildRotation = BuildPreviewMesh->GetComponentRotation();

    // 플레이어 시작 위치 저장 (이동 감지용)
    BuildStartPlayerLocation = OwnerCitizen->GetActorLocation();

    // 건설 시간 결정
    float BuildTime = (CurrentBuildingItem == EInventorySlot::Plank) ? PlankBuildTime : TentBuildTime;
    UE_LOG(LogTemp, Error, TEXT("=== BUILD TIME SET: %.2f seconds ==="), BuildTime);

    // 건설 완료 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        BuildTimerHandle,
        this,
        &UBuildingComponent::FinishBuild,
        BuildTime,
        false
    );

    // 딜레이는 건설 완료 후에 설정
    bCanBuildNow = false;

    // 항상 Delegate 호출 (IsLocallyControlled 조건 제거)
    UE_LOG(LogTemp, Error, TEXT("=== BROADCASTING BUILD START (FORCED): Progress=%.2f, IsBuilding=%s ==="),
        CurrentBuildProgress, bIsBuilding ? TEXT("TRUE") : TEXT("FALSE"));
    OnBuildProgressChanged.Broadcast(CurrentBuildProgress, bIsBuilding);
    UE_LOG(LogTemp, Error, TEXT("=== BROADCAST COMPLETED ==="));

    // 네트워크 업데이트
    GetOwner()->ForceNetUpdate();
    UE_LOG(LogTemp, Error, TEXT("=== AttemptBuild_Implementation COMPLETED ==="));
}

void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, const FVector& Location, const FRotator& Rotation)
{
    if (!MeshComp)
        return;

    // 모빌리티를 먼저 Movable로 설정 (물리 시뮬레이션 가능하게)
    MeshComp->SetMobility(EComponentMobility::Movable);

    // 물리/충돌 설정
    MeshComp->SetSimulatePhysics(false);  // 일단 false로 설정
    MeshComp->SetEnableGravity(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // 물리 상태 복제 설정
    MeshComp->SetIsReplicated(true);
    MeshComp->bReplicatePhysicsToAutonomousProxy = true;

    // 위치 고정
    MeshComp->SetWorldLocation(Location);
    MeshComp->SetWorldRotation(Rotation);
    MeshComp->UpdateComponentToWorld();
}

// 건설 후 인벤토리 상태 확인
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

    // 네트워크 업데이트 강제
    BuiltItem->ForceNetUpdate();
}

// 건설 딜레이 초기화
void UBuildingComponent::ResetBuildDelay()
{
    if (GetOwner()->HasAuthority())
    {
        bCanBuildNow = true;
    }
}

// 건설 타이머 시작
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

// 건설 완료
void UBuildingComponent::FinishBuild()
{
    if (!bIsBuilding || !OwnerCitizen || !GetOwner()->HasAuthority())
        return;

    UE_LOG(LogTemp, Error, TEXT("=== FinishBuild CALLED ==="));

    // 고정된 위치/회전으로 아이템 생성
    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            SpawnBuildingItem<AItem_Plank>(PlankClass, FixedBuildLocation, FixedBuildRotation);
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Tent))
        {
            SpawnBuildingItem<AItem_Tent>(TentClass, FixedBuildLocation, FixedBuildRotation);
        }
    }

    // 상태 초기화
    bIsBuilding = false;
    CurrentBuildProgress = 1.0f;

    // 항상 Delegate 호출 (조건 제거)
    UE_LOG(LogTemp, Error, TEXT("=== BROADCASTING BUILD FINISH: Progress=%.2f, IsBuilding=%s ==="),
        CurrentBuildProgress, bIsBuilding ? TEXT("TRUE") : TEXT("FALSE"));
    OnBuildProgressChanged.Broadcast(CurrentBuildProgress, bIsBuilding);

    // 건설 완료 후 딜레이 설정
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 0.5f, false);

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);

    // 완료 이벤트
    MulticastOnBuildComplete();

    // 네트워크 업데이트
    GetOwner()->ForceNetUpdate();

    UE_LOG(LogTemp, Error, TEXT("=== FinishBuild COMPLETED ==="));
}
// 건설 취소
void UBuildingComponent::CancelBuild()
{
    if (!GetOwner()->HasAuthority() || !bIsBuilding)
        return;

    UE_LOG(LogTemp, Error, TEXT("=== CancelBuild CALLED ==="));

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);

    // 상태 초기화
    bIsBuilding = false;
    CurrentBuildProgress = 0.0f;

    // 항상 Delegate 호출 (조건 제거)
    UE_LOG(LogTemp, Error, TEXT("=== BROADCASTING BUILD CANCEL: Progress=%.2f, IsBuilding=%s ==="),
        CurrentBuildProgress, bIsBuilding ? TEXT("TRUE") : TEXT("FALSE"));
    OnBuildProgressChanged.Broadcast(CurrentBuildProgress, bIsBuilding);

    // 즉시 다시 건설 가능하도록 (취소되었으므로)
    bCanBuildNow = true;

    // 네트워크 업데이트
    GetOwner()->ForceNetUpdate();

    UE_LOG(LogTemp, Error, TEXT("=== CancelBuild COMPLETED ==="));
}
// 널빤지 배치 검증
bool UBuildingComponent::ValidatePlankPlacement(const FVector& Location)
{
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCitizen);

    // 플랭크 전용 채널을 사용하여 다른 플랭크와의 충돌만 체크
    FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(50.0f));
    return !GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_GameTraceChannel2, // Plank 전용 채널 사용
        BoxShape,
        QueryParams
    );
}

// 텐트 배치 검증
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

// 건설 위치 검증
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
                    return ValidatePlankPlacement(Location);
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

                    if (Distance <= Zone->BridgeWidth * 0.25f)
                    {
                        return ValidateTentPlacement(Location);
                    }
                }
            }
        }
    }

    return false;  // 유효한 설치 영역을 찾지 못함
}

// 건설 상태 복제 처리
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

    // 소유자 상태 업데이트
    UpdateOwnerBuildState();
}

// 소유자 건설 상태 업데이트
void UBuildingComponent::UpdateOwnerBuildState()
{
    if (!OwnerCitizen)
        return;

    // 건설 상태에 따른 캐릭터 이동 제한
    if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
    {
        MovementComp->SetMovementMode(bIsBuilding ? MOVE_None : MOVE_Walking);
    }

    // 시각 피드백
    if (!bCanBuildNow && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Cannot build now!"));
    }

    // 컴포넌트 시각 상태 업데이트
    MarkRenderStateDirty();
}

// 프리뷰 메시 복제 처리
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

// 유효 배치 상태 복제 처리
void UBuildingComponent::OnRep_ValidPlacement()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // 로컬 플레이어가 아닐 경우에만 머티리얼 업데이트
    if (!OwnerCitizen->IsLocallyControlled())
    {
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }
}

// 건설 완료 멀티캐스트
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

