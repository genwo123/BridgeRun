// BuildingComponent.cpp
#include "BuildingComponent.h"
#include "Citizen.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "BuildableZone.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Item_Plank.h"
#include "Item_Tent.h"

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
        BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
        if (BuildPreviewMesh)
        {
            BuildPreviewMesh->SetupAttachment(nullptr);
            BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            BuildPreviewMesh->SetVisibility(false);
            BuildPreviewMesh->RegisterComponent();
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
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BuildPreviewMesh or OwnerCitizen is null"));
        return;
    }

    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();

        // 초기 프리뷰 위치 설정
        FVector PlayerLocation = OwnerCitizen->GetActorLocation();
        FVector ForwardVector = OwnerCitizen->GetActorForwardVector();
        ForwardVector.Z = 0.0f;
        ForwardVector.Normalize();

        FVector InitialPreviewLocation;
        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            InitialPreviewLocation = PlayerLocation + (ForwardVector * PlankPlacementDistance);
            InitialPreviewLocation.Z = PlayerLocation.Z; // 발 밑 높이
        }
        else // Tent
        {
            InitialPreviewLocation = PlayerLocation + (ForwardVector * TentPlacementDistance);
            InitialPreviewLocation.Z = PlayerLocation.Z + 100.0f; // 몸통 높이
        }

        BuildPreviewMesh->SetWorldLocation(InitialPreviewLocation);

        switch (CurrentBuildingItem)
        {
        case EInventorySlot::Plank:
            if (PlankClass)
            {
                AItem_Plank* DefaultPlank = PlankClass.GetDefaultObject();
                if (DefaultPlank && DefaultPlank->MeshComponent)
                {
                    PlankMesh = DefaultPlank->MeshComponent->GetStaticMesh();
                    BuildPreviewMesh->SetStaticMesh(PlankMesh);
                    // 실제 판자와 동일한 스케일 사용
                    BuildPreviewMesh->SetRelativeScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
                    BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
                    BuildPreviewMesh->SetVisibility(true);
                }
            }
            break;

        case EInventorySlot::Tent:
            if (TentClass)
            {
                AItem_Tent* DefaultTent = TentClass.GetDefaultObject();
                if (DefaultTent && DefaultTent->MeshComponent)
                {
                    TentMesh = DefaultTent->MeshComponent->GetStaticMesh();
                    BuildPreviewMesh->SetStaticMesh(TentMesh);
                    // 실제 텐트와 동일한 스케일 사용
                    BuildPreviewMesh->SetRelativeScale3D(DefaultTent->MeshComponent->GetRelativeScale3D());
                    BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
                    BuildPreviewMesh->SetVisibility(true);
                }
            }
            break;
        }
    }
}

void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // 1. 플레이어 위치와 방향 가져오기
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();
    FRotator PlayerRotation = OwnerCitizen->GetActorRotation();

    // 캐릭터의 전방 벡터 사용
    FVector ForwardVector = OwnerCitizen->GetActorForwardVector();
    ForwardVector.Z = 0.0f;
    ForwardVector.Normalize();

    // 고정된 프리뷰 거리 설정
    float PreviewDistance = (CurrentBuildingItem == EInventorySlot::Plank) ? PlankPlacementDistance : TentPlacementDistance;
    FVector BasePreviewLocation = PlayerLocation + (ForwardVector * PreviewDistance);

    // 기본 높이 설정 (BuildableZone 밖에서의 프리뷰 높이)
    if (CurrentBuildingItem == EInventorySlot::Plank)
    {
        BasePreviewLocation.Z = PlayerLocation.Z; // 발 밑 높이
    }
    else // Tent
    {
        BasePreviewLocation.Z = PlayerLocation.Z + 100.0f; // 몸통 높이
    }

    // 2. BuildableZone 체크
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
                FVector LeftPoint = Zone->LeftBottomRope->GetComponentLocation();
                FVector RightPoint = Zone->RightBottomRope->GetComponentLocation();

                // 로프 방향과 길이 계산
                FVector RopeDirection = (RightPoint - LeftPoint).GetSafeNormal();
                float RopeLength = FVector::Distance(LeftPoint, RightPoint);

                // 프리뷰 위치가 로프 영역 내에 있는지 확인
                FBox BuildableArea = FBox(
                    FVector(
                        FMath::Min(LeftPoint.X, RightPoint.X) - Zone->BridgeWidth * 0.5f,
                        FMath::Min(LeftPoint.Y, RightPoint.Y) - Zone->BridgeWidth * 0.5f,
                        LeftPoint.Z - 50.0f
                    ),
                    FVector(
                        FMath::Max(LeftPoint.X, RightPoint.X) + Zone->BridgeWidth * 0.5f,
                        FMath::Max(LeftPoint.Y, RightPoint.Y) + Zone->BridgeWidth * 0.5f,
                        LeftPoint.Z + 50.0f
                    )
                );

                // 프리뷰가 설치 가능 영역 내에 있는지 확인
                if (BuildableArea.IsInsideOrOn(BasePreviewLocation))
                {
                    bIsValidPlacement = true;

                    // 높이만 로프 높이로 조정
                    FVector NewLocation = BasePreviewLocation;
                    NewLocation.Z = LeftPoint.Z;

                    BuildPreviewMesh->SetWorldLocation(NewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, ValidPlacementMaterial);

                    // 디버그 시각화
                    DrawDebugBox(GetWorld(), BuildableArea.GetCenter(), BuildableArea.GetExtent(),
                        FQuat::Identity, FColor::Green, false, -1.0f, 0, 2.0f);
                }
                else
                {
                    BuildPreviewMesh->SetWorldLocation(BasePreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);

                    // 디버그 시각화
                    DrawDebugBox(GetWorld(), BuildableArea.GetCenter(), BuildableArea.GetExtent(),
                        FQuat::Identity, FColor::Red, false, -1.0f, 0, 2.0f);
                }
            }
        }
        else if (CurrentBuildingItem == EInventorySlot::Tent)
        {
            if (Zone->LeftTopRope && Zone->LeftBottomRope)
            {
                FVector TopPoint = Zone->LeftTopRope->GetComponentLocation();
                FVector BottomPoint = Zone->LeftBottomRope->GetComponentLocation();

                // 수직 로프와의 거리 계산
                FVector VerticalDirection = (TopPoint - BottomPoint).GetSafeNormal();
                FVector ToPreview = BasePreviewLocation - BottomPoint;

                // 수직 로프 길이
                float RopeHeight = FVector::Distance(TopPoint, BottomPoint);

                // 텐트 설치 가능 영역 체크 - 전체 영역
                float HorizontalDistance = FVector::Distance(
                    FVector(BasePreviewLocation.X, BasePreviewLocation.Y, BottomPoint.Z),
                    FVector(BottomPoint.X, BottomPoint.Y, BottomPoint.Z));

                if (HorizontalDistance <= Zone->BridgeWidth * 0.5f)
                {
                    bIsValidPlacement = true;

                    // 프리뷰 높이 유지 (바닥에 붙지 않게)
                    FVector NewLocation = BasePreviewLocation;
                    // 원하는 높이로 조정 (예: 바닥에서 100 유닛 위)
                    NewLocation.Z = BottomPoint.Z + 100.0f;

                    BuildPreviewMesh->SetWorldLocation(NewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, ValidPlacementMaterial);
                }
                else
                {
                    BuildPreviewMesh->SetWorldLocation(BasePreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
                }
            }
        }

        if (bIsValidPlacement) break;
    }

    BuildPreviewMesh->SetVisibility(true);

    // Debug 정보 표시
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("Preview Position: %s"), *BuildPreviewMesh->GetComponentLocation().ToString()));
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("Valid Placement: %s"), bIsValidPlacement ? TEXT("Yes") : TEXT("No")));
}


void UBuildingComponent::AttemptBuild()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cannot build now"));
        return;
    }

    bIsBuilding = true;
    float BuildTime = (CurrentBuildingItem == EInventorySlot::Plank) ? PlankBuildTime : TentBuildTime;

    GetWorld()->GetTimerManager().SetTimer(
        BuildDelayTimerHandle,
        this,
        &UBuildingComponent::FinishBuild,
        BuildTime,
        false
    );
}

void UBuildingComponent::FinishBuild()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bIsValidPlacement)
        return;

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    switch (CurrentBuildingItem)
    {
    case EInventorySlot::Plank:
        if (PlankClass && OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;

            if (AItem_Plank* SpawnedPlank = GetWorld()->SpawnActor<AItem_Plank>(
                PlankClass, Location, Rotation, SpawnParams))
            {
                SpawnedPlank->OnPlaced();
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Plank placed successfully!"));
            }
        }
        break;

    case EInventorySlot::Tent:
        if (TentClass && OwnerCitizen->UseItem(EInventorySlot::Tent))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;

            if (AItem_Tent* SpawnedTent = GetWorld()->SpawnActor<AItem_Tent>(
                TentClass, Location, Rotation, SpawnParams))
            {
                SpawnedTent->OnPlaced();
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Tent placed successfully!"));
            }
        }
        break;
    }

    bIsBuilding = false;
    bCanBuildNow = true;
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