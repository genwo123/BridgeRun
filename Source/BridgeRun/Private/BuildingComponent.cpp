// BuildingComponent.cpp
#include "BuildingComponent.h"
#include "Citizen.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "BuildableZone.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "Kismet/GameplayStatics.h"

UBuildingComponent::UBuildingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;  // tick 활성화
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
                AItem* DefaultPlank = PlankClass.GetDefaultObject();
                if (DefaultPlank && DefaultPlank->MeshComponent)
                {
                    PlankMesh = DefaultPlank->MeshComponent->GetStaticMesh();
                    // RelativeScale3D 사용 및 디버그 메시지 추가
                    FVector PlankScale = DefaultPlank->MeshComponent->GetRelativeScale3D();
                    BuildPreviewMesh->SetRelativeScale3D(PlankScale);
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                        FString::Printf(TEXT("Setting Preview Scale: %s"), *PlankScale.ToString()));
                }
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("PlankClass not set!"));
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


void UBuildingComponent::DeactivateBuildMode()
{
    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->SetVisibility(false);
    }
}

void UBuildingComponent::OnBuildModeEntered()
{
    if (!BuildPreviewMesh)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BuildPreviewMesh is null!"));
        return;
    }

    // 캐릭터 회전 설정
    if (OwnerCitizen)
    {
        MovementComponent = OwnerCitizen->GetCharacterMovement();
        if (MovementComponent)
        {
            MovementComponent->bOrientRotationToMovement = true;
            MovementComponent->bUseControllerDesiredRotation = false;
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Build Mode Entered"));

    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();

        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            if (PlankMesh)
            {
                BuildPreviewMesh->SetStaticMesh(PlankMesh);
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Set Plank Mesh to Preview"));

                if (BuildPreviewMesh->GetStaticMesh())
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Static Mesh Set Successfully"));
                }
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("PlankMesh is null"));
            }
        }

        BuildPreviewMesh->SetVisibility(true);
    }
}

void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
    {
        return;
    }

    // 카메라 시점과 방향 가져오기
    APlayerController* PC = Cast<APlayerController>(OwnerCitizen->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    // 플레이어 위치를 기준으로 프리뷰 위치 계산
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();

    // 프리뷰를 플레이어 앞에 배치
    const float FixedPreviewDistance = 200.0f;
    FVector PreviewLocation = PlayerLocation + (CameraForward * FixedPreviewDistance);

    // 프리뷰 회전
    FRotator PreviewRotation = CameraRotation;
    PreviewRotation.Pitch = 0.0f;
    PreviewRotation.Roll = 0.0f;

    // 프리뷰 위치와 회전 설정
    BuildPreviewMesh->SetWorldLocation(PreviewLocation);
    BuildPreviewMesh->SetWorldRotation(PreviewRotation);
    BuildPreviewMesh->SetVisibility(true);

    // BuildableZone 체크
    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableZone::StaticClass(), FoundZones);

    bIsValidPlacement = false;

    for (AActor* Actor : FoundZones)
    {
        if (ABuildableZone* Zone = Cast<ABuildableZone>(Actor))
        {
            if (Zone->LeftBottomRope && Zone->RightBottomRope)
            {
                FVector LeftPoint = Zone->LeftBottomRope->GetComponentLocation();
                FVector RightPoint = Zone->RightBottomRope->GetComponentLocation();
                PreviewLocation.Z = Zone->LeftBottomRope->GetComponentLocation().Z;


                // 로프 방향과 길이 계산
                FVector RopeDirection = (RightPoint - LeftPoint).GetSafeNormal();
                float RopeLength = FVector::Distance(LeftPoint, RightPoint);

                // 프리뷰 위치에서 왼쪽 로프까지의 벡터를 로프 방향으로 투영
                FVector PreviewToLeft = PreviewLocation - LeftPoint;
                float ProjectedDistance = FVector::DotProduct(PreviewToLeft, RopeDirection);

                // 프리뷰가 로프 사이 영역 안에 있는지만 체크
                bIsValidPlacement = (ProjectedDistance >= 0 && ProjectedDistance <= RopeLength);

                // 로프 사이의 중간점을 찾아서 거기에 맞춰 프리뷰 조정
                if (bIsValidPlacement)
                {
                    // 원래 프리뷰 위치 유지 (플레이어 시선 기준)
                    FVector AdjustedPreviewLocation = PreviewLocation;
                    // Z 높이만 로프 높이로 고정
                    AdjustedPreviewLocation.Z = LeftPoint.Z;

                    BuildPreviewMesh->SetWorldLocation(AdjustedPreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PreviewRotation);

                    // 디버그 시각화
                    DrawDebugLine(GetWorld(), LeftPoint, RightPoint,
                        FColor::Green, false, -1.0f, 0, 2.0f);
                    DrawDebugLine(GetWorld(), PreviewLocation, AdjustedPreviewLocation,
                        FColor::Green, false, -1.0f, 0, 2.0f);
                    break;
                }
                else
                {
                    DrawDebugLine(GetWorld(), LeftPoint, RightPoint,
                        FColor::Red, false, -1.0f, 0, 2.0f);
                }
            }
        }
    }

    // 프리뷰 머티리얼 업데이트
    if (ValidPlacementMaterial && InvalidPlacementMaterial)
    {
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    // 디버그 정보 표시
    GEngine->AddOnScreenDebugMessage(-1, 0.0f,
        bIsValidPlacement ? FColor::Green : FColor::Red,
        FString::Printf(TEXT("Valid Placement: %s"), bIsValidPlacement ? TEXT("Yes") : TEXT("No")));
}

void UBuildingComponent::AttemptBuild()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cannot build now"));
        return;
    }

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

            AItem* SpawnedPlank = GetWorld()->SpawnActor<AItem>(
                PlankClass,
                Location,
                Rotation,
                SpawnParams
            );

            if(SpawnedPlank)
            {
                SpawnedPlank->bIsBuiltItem = true;

                if (SpawnedPlank->MeshComponent)
                {
                    // 물리 시뮬레이션 비활성화
                    SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
                    SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

                    // Root 컴포넌트 이동성 설정
                    SpawnedPlank->MeshComponent->SetMobility(EComponentMobility::Static);
                }

                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Plank placed successfully!"));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Not enough planks!"));
        }
    }
}




void UBuildingComponent::ResetBuildDelay()
{
    bCanBuildNow = true;
}

void UBuildingComponent::RotateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen) return;

    FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
    NewRotation.Yaw += BuildRotationStep;
    BuildPreviewMesh->SetWorldRotation(NewRotation);
}