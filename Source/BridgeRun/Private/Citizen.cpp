// Citizen.cpp
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "InvenComponent.h"
#include "BuildableZone.h" 

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;

    // ĳ���� ȸ�� ����
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // ĳ���Ͱ� �̵� �������� �ڿ������� ȸ���ϵ��� ����
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // �������� ����
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    // ī�޶� ����
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    // �κ��丮 ������Ʈ ����
    InvenComponent = CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));

    // BuildPreviewMesh �ʱ�ȭ
    BuildPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildPreviewMesh"));
    BuildPreviewMesh->SetupAttachment(RootComponent);
    BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BuildPreviewMesh->SetVisibility(false);

    // �⺻�� �ʱ�ȭ�� InvenComponent���� ó��
}

void ACitizen::BeginPlay()
{
    Super::BeginPlay();

    // �κ��丮 UI ����
    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // �Ǽ� ���� �������� ���õǾ��� ���� ������ ������Ʈ
    if (InvenComponent &&
        (InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Plank ||
            InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Tent))
    {
        UpdateBuildPreview();
    }
}

// �̵� ���� �Լ����� �״�� ����
void ACitizen::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::Turn(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value);
    }
}

void ACitizen::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerPitchInput(Value);
    }
}

void ACitizen::StartJump()
{
    bPressedJump = true;
}

void ACitizen::StopJump()
{
    bPressedJump = false;
}

void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // �̵�
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // ����
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);

    // �κ��丮 ���� ����
    PlayerInputComponent->BindAction("Slot1", IE_Pressed, this, &ACitizen::SelectSlot1);
    PlayerInputComponent->BindAction("Slot2", IE_Pressed, this, &ACitizen::SelectSlot2);
    PlayerInputComponent->BindAction("Slot3", IE_Pressed, this, &ACitizen::SelectSlot3);
    PlayerInputComponent->BindAction("Slot4", IE_Pressed, this, &ACitizen::SelectSlot4);
    PlayerInputComponent->BindAction("Slot5", IE_Pressed, this, &ACitizen::SelectSlot5);

    // �Ǽ� ����
    PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, this, &ACitizen::RotateBuildPreview);
    PlayerInputComponent->BindAction("Build", IE_Pressed, this, &ACitizen::AttemptBuild);

    //��ȣ �ۿ�
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);
}

void ACitizen::SelectInventorySlot(EInventorySlot Slot)
{
    if (InvenComponent)
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count > 0)
        {
            if (InvenComponent->GetCurrentSelectedSlot() == Slot)
            {
                InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                BuildPreviewMesh->SetVisibility(false);
            }
            else
            {
                InvenComponent->SetCurrentSelectedSlot(Slot);
                if (Slot == EInventorySlot::Plank || Slot == EInventorySlot::Tent)
                {
                    BuildPreviewMesh->SetVisibility(true);
                }
                else
                {
                    BuildPreviewMesh->SetVisibility(false);
                }
            }
        }
    }
}

void ACitizen::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !InvenComponent) return;

    EInventorySlot CurrentSlot = InvenComponent->GetCurrentSelectedSlot();
    if (CurrentSlot != EInventorySlot::Plank && CurrentSlot != EInventorySlot::Tent)
    {
        BuildPreviewMesh->SetVisibility(false);
        return;
    }
    BuildPreviewMesh->SetVisibility(true);

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    FVector Start, Dir;
    PC->DeprojectMousePositionToWorld(Start, Dir);
    FVector End = Start + (Dir * MaxBuildDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    bool bValidPlacement = false;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (ABuildableZone* Zone = Cast<ABuildableZone>(HitResult.GetActor()))
        {
            // �¿� ��ġ ����Ʈ ���
            FVector LeftPoint = HitResult.Location;
            FVector RightPoint = LeftPoint + FVector(0.0f, Zone->BridgeWidth, 0.0f);

            // ���ڿ� õ���� ��ġ ���� ���� üũ
            if (CurrentSlot == EInventorySlot::Plank)  // CurrentSelectedSlot�� CurrentSlot���� ����
            {
                bValidPlacement = Zone->IsPlankPlacementValid(LeftPoint, RightPoint);
            }
            else if (CurrentSlot == EInventorySlot::Tent)  // CurrentSelectedSlot�� CurrentSlot���� ����
            {
                bValidPlacement = Zone->IsTentPlacementValid(LeftPoint, RightPoint);
            }

            // ������ ��ġ �� ȸ�� ����
            BuildPreviewMesh->SetWorldLocation(LeftPoint);
            BuildPreviewMesh->SetWorldRotation(FRotator::ZeroRotator);

            // ����׿� �ð�ȭ (���� �߿��� ���)
            DrawDebugLine(GetWorld(), LeftPoint, RightPoint,
                bValidPlacement ? FColor::Green : FColor::Red,
                false, -1.0f, 0, 2.0f);
        }
    }

    // ������ ��Ƽ���� ������Ʈ
    if (ValidPlacementMaterial && InvalidPlacementMaterial)
    {
        BuildPreviewMesh->SetMaterial(0, bValidPlacement ?
            ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    // ����� �޽���
    GEngine->AddOnScreenDebugMessage(-1, 0.1f,
        bValidPlacement ? FColor::Green : FColor::Red,
        FString::Printf(TEXT("Can Place: %s"),
            bValidPlacement ? TEXT("Yes") : TEXT("No")));
}

void ACitizen::RotateBuildPreview()
{
    if (InvenComponent && InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Plank)
    {
        FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
        NewRotation.Yaw += BuildRotationStep;
        BuildPreviewMesh->SetWorldRotation(NewRotation);
    }
}

void ACitizen::AttemptBuild()
{
    if (!InvenComponent) return;

    EInventorySlot CurrentSlot = InvenComponent->GetCurrentSelectedSlot();
    if (CurrentSlot != EInventorySlot::Plank && CurrentSlot != EInventorySlot::Tent) return;

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    if (IsValidBuildLocation(Location))
    {
        if (UseItem(CurrentSlot))
        {
            // TODO: ���� �Ǽ� ����
        }
    }
}

void ACitizen::AddItem(EInventorySlot Slot, int32 Amount)
{
    if (InvenComponent)
    {
        InvenComponent->UpdateItemCount(Slot, Amount);
    }
}

bool ACitizen::UseItem(EInventorySlot Slot, int32 Amount)
{
    if (InvenComponent)
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count >= Amount)
        {
            ItemData->Count -= Amount;
            InvenComponent->UpdateItemCount(Slot, -Amount);
            return true;
        }
    }
    return false;
}

bool ACitizen::IsValidBuildLocation(const FVector& Location) const
{
    // TODO: BuildableZone üũ ����
    return true;
}


void ACitizen::Interact()
{
    // ����ĳ��Ʈ �������� ���� üũ
    FVector Start = CameraComponent->GetComponentLocation();
    FVector Forward = CameraComponent->GetForwardVector();
    FVector End = Start + (Forward * InteractionRange);

    // ����� ���� �׸���
    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Red,
        false,  // persistent lines
        5.0f,   // lifetime
        0,      // depth priority
        2.0f    // thickness
    );

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        // ��Ʈ �߻� Ȯ��
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName()));

        if (AItem* Item = Cast<AItem>(HitResult.GetActor()))
        {
            // ������ ĳ���� ���� Ȯ��
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
                TEXT("Found Item - Adding to Inventory"));

            // ������ ȹ��
            AddItem(Item->ItemType, Item->Amount);
            Item->Destroy();

            // ������ ���� Ȯ��
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White,
                TEXT("Item Destroyed"));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                TEXT("Hit Actor is not an Item"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("No Hit Result"));
    }
}