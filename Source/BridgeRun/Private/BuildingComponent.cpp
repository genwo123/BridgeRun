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
    PrimaryComponentTick.bCanEverTick = true;  // tick Ȱ��ȭ
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
                    // RelativeScale3D ��� �� ����� �޽��� �߰�
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

    // ĳ���� ȸ�� ����
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

    // ī�޶� ������ ���� ��������
    APlayerController* PC = Cast<APlayerController>(OwnerCitizen->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    // �÷��̾� ��ġ�� �������� ������ ��ġ ���
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();

    // �����並 �÷��̾� �տ� ��ġ
    const float FixedPreviewDistance = 200.0f;
    FVector PreviewLocation = PlayerLocation + (CameraForward * FixedPreviewDistance);

    // ������ ȸ��
    FRotator PreviewRotation = CameraRotation;
    PreviewRotation.Pitch = 0.0f;
    PreviewRotation.Roll = 0.0f;

    // ������ ��ġ�� ȸ�� ����
    BuildPreviewMesh->SetWorldLocation(PreviewLocation);
    BuildPreviewMesh->SetWorldRotation(PreviewRotation);
    BuildPreviewMesh->SetVisibility(true);

    // BuildableZone üũ
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


                // ���� ����� ���� ���
                FVector RopeDirection = (RightPoint - LeftPoint).GetSafeNormal();
                float RopeLength = FVector::Distance(LeftPoint, RightPoint);

                // ������ ��ġ���� ���� ���������� ���͸� ���� �������� ����
                FVector PreviewToLeft = PreviewLocation - LeftPoint;
                float ProjectedDistance = FVector::DotProduct(PreviewToLeft, RopeDirection);

                // �����䰡 ���� ���� ���� �ȿ� �ִ����� üũ
                bIsValidPlacement = (ProjectedDistance >= 0 && ProjectedDistance <= RopeLength);

                // ���� ������ �߰����� ã�Ƽ� �ű⿡ ���� ������ ����
                if (bIsValidPlacement)
                {
                    // ���� ������ ��ġ ���� (�÷��̾� �ü� ����)
                    FVector AdjustedPreviewLocation = PreviewLocation;
                    // Z ���̸� ���� ���̷� ����
                    AdjustedPreviewLocation.Z = LeftPoint.Z;

                    BuildPreviewMesh->SetWorldLocation(AdjustedPreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PreviewRotation);

                    // ����� �ð�ȭ
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

    // ������ ��Ƽ���� ������Ʈ
    if (ValidPlacementMaterial && InvalidPlacementMaterial)
    {
        BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    // ����� ���� ǥ��
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
                    // ���� �ùķ��̼� ��Ȱ��ȭ
                    SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
                    SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

                    // Root ������Ʈ �̵��� ����
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