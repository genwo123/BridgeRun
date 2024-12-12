// Citizen.cpp
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Item_Telescope.h"  
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "InvenComponent.h"
#include "PlayerModeComponent.h"
#include "BuildingComponent.h"
#include "Item_Trophy.h"
#include "CombatComponent.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    InvenComponent = CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));
    PlayerModeComponent = CreateDefaultSubobject<UPlayerModeComponent>(TEXT("PlayerModeComponent"));
    BuildingComponent = CreateDefaultSubobject<UBuildingComponent>(TEXT("BuildingComponent"));
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void ACitizen::BeginPlay()
{
    Super::BeginPlay();

    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }

    if (PlayerModeComponent)
    {
        PlayerModeComponent->OnPlayerModeChanged.AddDynamic(this, &ACitizen::OnPlayerModeChanged);
    }

    AddItem(EInventorySlot::Telescope);
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACitizen::OnPlayerModeChanged(EPlayerMode NewMode, EPlayerMode OldMode)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("Mode Changed: %s -> %s"),
            *UEnum::GetValueAsString(OldMode),
            *UEnum::GetValueAsString(NewMode)));

    switch (NewMode)
    {
    case EPlayerMode::Build:
        if (BuildingComponent)
        {
            BuildingComponent->OnBuildModeEntered();
        }
        break;

    case EPlayerMode::Combat:
        if (CombatComponent)
        {
            CombatComponent->OnCombatModeEntered();
        }
        break;

    case EPlayerMode::Normal:
        if (BuildingComponent)
        {
            BuildingComponent->DeactivateBuildMode();
        }
        if (CombatComponent)
        {
            CombatComponent->OnCombatModeExited();
        }
        GetCharacterMovement()->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
        break;
    }
}

void ACitizen::SelectInventorySlot(EInventorySlot Slot)
{
    if (!InvenComponent || !PlayerModeComponent) return;

    // 현재 선택된 슬롯이면 해제 로직을 먼저 처리
    if (InvenComponent->GetCurrentSelectedSlot() == Slot)
    {
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);

        // 모든 장착된 아이템 해제
        if (CombatComponent)
        {
            if (CombatComponent->EquippedTelescope)
            {
                CombatComponent->OnTelescopeUnequipped();
            }
            if (CombatComponent->EquippedGun)
            {
                CombatComponent->OnGunUnequipped();
            }
        }
        return;
    }

    if (PlayerModeComponent->GetCurrentMode() != EPlayerMode::Normal)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Must return to Normal mode before selecting new item"));
        return;
    }

    FItemData* ItemData = InvenComponent->GetItemData(Slot);
    if (!ItemData || ItemData->Count <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            FString::Printf(TEXT("Cannot select slot: No items available in slot %d"), static_cast<int32>(Slot)));
        return;
    }

    InvenComponent->SetCurrentSelectedSlot(Slot);
    switch (Slot)
    {
    case EInventorySlot::Plank:
    case EInventorySlot::Tent:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Build);
        break;

    case EInventorySlot::Gun:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Combat);
        if (CombatComponent && CombatComponent->GunClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            AItem_Gun* NewGun = GetWorld()->SpawnActor<AItem_Gun>(CombatComponent->GunClass,
                GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
            if (NewGun)
            {
                CombatComponent->OnGunEquipped(NewGun);
            }
        }
        break;

    case EInventorySlot::Telescope:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Combat);
        if (CombatComponent && CombatComponent->TelescopeClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            AItem_Telescope* NewTelescope = GetWorld()->SpawnActor<AItem_Telescope>(
                CombatComponent->TelescopeClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
            if (NewTelescope)
            {
                CombatComponent->OnTelescopeEquipped(NewTelescope);
            }
        }
        break;

    case EInventorySlot::Trophy:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        break;
    }
}


void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);

    PlayerInputComponent->BindAction("Slot1", IE_Pressed, this, &ACitizen::SelectSlot1);
    PlayerInputComponent->BindAction("Slot2", IE_Pressed, this, &ACitizen::SelectSlot2);
    PlayerInputComponent->BindAction("Slot3", IE_Pressed, this, &ACitizen::SelectSlot3);
    PlayerInputComponent->BindAction("Slot4", IE_Pressed, this, &ACitizen::SelectSlot4);
    PlayerInputComponent->BindAction("Slot5", IE_Pressed, this, &ACitizen::SelectSlot5);

    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);

    // 건설 관련 입력
    if (BuildingComponent)
    {
        PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, BuildingComponent, &UBuildingComponent::RotateBuildPreview);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, BuildingComponent, &UBuildingComponent::AttemptBuild);
    }

    // 전투 관련 입력
    if (CombatComponent)
    {
        PlayerInputComponent->BindAction("Zoom", IE_Pressed, CombatComponent, &UCombatComponent::HandleAim);  // HandleZoom을 HandleAim으로 변경
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, CombatComponent, &UCombatComponent::HandleShoot);
        PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, CombatComponent, &UCombatComponent::DropCurrentWeapon);
    }

   
}

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
    AddControllerYawInput(Value);
}

void ACitizen::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ACitizen::StartJump()
{
    bPressedJump = true;
}

void ACitizen::StopJump()
{
    bPressedJump = false;
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
            switch (Slot)
            {
            case EInventorySlot::Plank:
            case EInventorySlot::Tent:
                ItemData->Count -= Amount;
                InvenComponent->UpdateItemCount(Slot, -Amount);
                if (ItemData->Count == 0 && BuildingComponent)
                {
                    BuildingComponent->DeactivateBuildMode();
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;

            case EInventorySlot::Telescope:
            case EInventorySlot::Gun:
                break;

            case EInventorySlot::Trophy:
                ItemData->Count -= Amount;
                InvenComponent->UpdateItemCount(Slot, -Amount);
                if (ItemData->Count == 0)
                {
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;
            }
            return true;
        }
    }
    return false;
}

void ACitizen::Interact()
{
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
        return;
    }

    FVector Start = CameraComponent->GetComponentLocation();
    FVector Forward = CameraComponent->GetForwardVector();
    FVector End = Start + (Forward * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(HitResult.GetActor()))
        {
            Trophy->PickUp(this);
            HeldTrophy = Trophy;
        }
        else if (AItem_Gun* Gun = Cast<AItem_Gun>(HitResult.GetActor()))
        {
            FItemData* GunData = InvenComponent->GetItemData(EInventorySlot::Gun);
            if (!GunData || GunData->Count == 0)
            {
                AddItem(EInventorySlot::Gun, 1);
                Gun->Destroy();

                UE_LOG(LogTemp, Warning, TEXT("Picked up gun with ammo: %d"), Gun->GetCurrentAmmo());
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Already has a gun"));
            }
        }
        else if (AItem* Item = Cast<AItem>(HitResult.GetActor()))
        {
            AddItem(Item->ItemType, Item->Amount);
            Item->Destroy();
        }
    }
}