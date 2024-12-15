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

        // �ʱ� ������ ��ġ ����
        FVector PlayerLocation = OwnerCitizen->GetActorLocation();
        FVector ForwardVector = OwnerCitizen->GetActorForwardVector();
        ForwardVector.Z = 0.0f;
        ForwardVector.Normalize();

        FVector InitialPreviewLocation;
        if (CurrentBuildingItem == EInventorySlot::Plank)
        {
            InitialPreviewLocation = PlayerLocation + (ForwardVector * PlankPlacementDistance);
            InitialPreviewLocation.Z = PlayerLocation.Z; // �� �� ����
        }
        else // Tent
        {
            InitialPreviewLocation = PlayerLocation + (ForwardVector * TentPlacementDistance);
            InitialPreviewLocation.Z = PlayerLocation.Z + 100.0f; // ���� ����
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
                    // ���� ���ڿ� ������ ������ ���
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
                    // ���� ��Ʈ�� ������ ������ ���
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

    // 1. �÷��̾� ��ġ�� ���� ��������
    FVector PlayerLocation = OwnerCitizen->GetActorLocation();
    FRotator PlayerRotation = OwnerCitizen->GetActorRotation();

    // ĳ������ ���� ���� ���
    FVector ForwardVector = OwnerCitizen->GetActorForwardVector();
    ForwardVector.Z = 0.0f;
    ForwardVector.Normalize();

    // ������ ������ �Ÿ� ����
    float PreviewDistance = (CurrentBuildingItem == EInventorySlot::Plank) ? PlankPlacementDistance : TentPlacementDistance;
    FVector BasePreviewLocation = PlayerLocation + (ForwardVector * PreviewDistance);

    // �⺻ ���� ���� (BuildableZone �ۿ����� ������ ����)
    if (CurrentBuildingItem == EInventorySlot::Plank)
    {
        BasePreviewLocation.Z = PlayerLocation.Z; // �� �� ����
    }
    else // Tent
    {
        BasePreviewLocation.Z = PlayerLocation.Z + 100.0f; // ���� ����
    }

    // 2. BuildableZone üũ
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

                // ���� ����� ���� ���
                FVector RopeDirection = (RightPoint - LeftPoint).GetSafeNormal();
                float RopeLength = FVector::Distance(LeftPoint, RightPoint);

                // ������ ��ġ�� ���� ���� ���� �ִ��� Ȯ��
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

                // �����䰡 ��ġ ���� ���� ���� �ִ��� Ȯ��
                if (BuildableArea.IsInsideOrOn(BasePreviewLocation))
                {
                    bIsValidPlacement = true;

                    // ���̸� ���� ���̷� ����
                    FVector NewLocation = BasePreviewLocation;
                    NewLocation.Z = LeftPoint.Z;

                    BuildPreviewMesh->SetWorldLocation(NewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, ValidPlacementMaterial);

                    // ����� �ð�ȭ
                    DrawDebugBox(GetWorld(), BuildableArea.GetCenter(), BuildableArea.GetExtent(),
                        FQuat::Identity, FColor::Green, false, -1.0f, 0, 2.0f);
                }
                else
                {
                    BuildPreviewMesh->SetWorldLocation(BasePreviewLocation);
                    BuildPreviewMesh->SetWorldRotation(PlayerRotation);
                    BuildPreviewMesh->SetMaterial(0, InvalidPlacementMaterial);

                    // ����� �ð�ȭ
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

                // ���� �������� �Ÿ� ���
                FVector VerticalDirection = (TopPoint - BottomPoint).GetSafeNormal();
                FVector ToPreview = BasePreviewLocation - BottomPoint;

                // ���� ���� ����
                float RopeHeight = FVector::Distance(TopPoint, BottomPoint);

                // ��Ʈ ��ġ ���� ���� üũ - ��ü ����
                float HorizontalDistance = FVector::Distance(
                    FVector(BasePreviewLocation.X, BasePreviewLocation.Y, BottomPoint.Z),
                    FVector(BottomPoint.X, BottomPoint.Y, BottomPoint.Z));

                if (HorizontalDistance <= Zone->BridgeWidth * 0.5f)
                {
                    bIsValidPlacement = true;

                    // ������ ���� ���� (�ٴڿ� ���� �ʰ�)
                    FVector NewLocation = BasePreviewLocation;
                    // ���ϴ� ���̷� ���� (��: �ٴڿ��� 100 ���� ��)
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

    // Debug ���� ǥ��
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