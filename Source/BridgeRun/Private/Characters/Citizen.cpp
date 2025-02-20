// Private/Characters/Citizen.cpp
#include "Characters/Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Item/Item_Telescope.h"  
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "Modes/InvenComponent.h"
#include "Modes/PlayerModeComponent.h"
#include "Modes/BuildingComponent.h"
#include "Item/Item_Trophy.h"
#include "Modes/CombatComponent.h"
#include "Item/Item_Plank.h"
#include "GameFramework/PlayerStart.h"
#include "Item/Item_Tent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;  // ��Ʈ��ũ ���� Ȱ��ȭ

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

    // UI ������ ���� �÷��̾���� ����
    if (IsLocallyControlled())
    {
        if (InventoryWidgetClass)
        {
            InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
            if (InventoryWidget)
            {
                InventoryWidget->AddToViewport();
            }
        }
    }

    // �÷��̾� ��� ���� �̺�Ʈ ���ε�
    // ��� �ӽſ��� �ʿ��ϹǷ� ���� üũ ���� ����
    if (PlayerModeComponent)
    {
        PlayerModeComponent->OnPlayerModeChanged.AddDynamic(this, &ACitizen::OnPlayerModeChanged);
    }

    // �ʱ� ������ ������ ���������� ����
    if (HasAuthority())
    {
        // �������� �÷��̾�� 1���� ����
        FItemData* TelescopeData = InvenComponent ? InvenComponent->GetItemData(EInventorySlot::Telescope) : nullptr;
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }
}

void ACitizen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACitizen, bIsDead);  // ���� HeldTrophy ���� �Ʒ��� �߰�
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ACitizen::OnRep_IsDead()
{
    // �׾��� ��
    if (bIsDead)
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
        // ��� �ִ� Ʈ���� ���
        if (HeldTrophy)
        {
            HeldTrophy->Drop();
            HeldTrophy = nullptr;
        }
    }
    // ���������� ��
    else
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void ACitizen::MulticastHandleDeath_Implementation()
{
    bIsDead = true;

    // ������ ����
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement(); 
    }

    // �Է� ��Ȱ��ȭ �߰�
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
}

void ACitizen::ServerRespawn_Implementation(const FVector& RespawnLocation)
{
    if (!HasAuthority()) return;

    // ��ġ ����
    SetActorLocation(RespawnLocation);
    bIsDead = false;

    // �̵� ������Ʈ �ʱ�ȭ
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->GravityScale = 1.0f;
    }

    // ��� �ʱ�ȭ
    if (PlayerModeComponent)
    {
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
    }

    // ������Ʈ �ʱ�ȭ
    if (BuildingComponent)
    {
        BuildingComponent->DeactivateBuildMode();
    }

    if (CombatComponent)
    {
        // ������ ����/��� ����
        if (CombatComponent->EquippedGun)
        {
            CombatComponent->OnGunUnequipped();
        }
        if (CombatComponent->EquippedTelescope)
        {
            CombatComponent->OnTelescopeUnequipped();
        }
    }

    // �κ��丮 �ʱ�ȭ
    if (InvenComponent)
    {
        // ��� ������ ����
        InvenComponent->UpdateItemCount(EInventorySlot::Plank, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Tent, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Gun, 0);

        // ������ ����
        FItemData* TelescopeData = InvenComponent->GetItemData(EInventorySlot::Telescope);
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }

    // ��� �ִ� Ʈ���� ���
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
    }

    // �Է� ��Ȱ��ȭ
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        EnableInput(PC);
    }
}

bool ACitizen::ServerRespawn_Validate(const FVector& RespawnLocation)
{
    return true;
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
            // ������ Ŭ���̾�Ʈ ��ο��� ó���ǵ��� ����
            if (HasAuthority())
            {
                BuildingComponent->OnBuildModeEntered();
            }
            else if (IsLocallyControlled())  // ���� Ŭ���̾�Ʈ�� ��쿡��
            {
                BuildingComponent->OnBuildModeEntered();  // _Implementation ����
            }
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
    // ���ÿ��� ���� ó��
    if (IsLocallyControlled())
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count > 0)
        {
            if (Slot == EInventorySlot::Plank || Slot == EInventorySlot::Tent)
            {
                OnPlayerModeChanged(EPlayerMode::Build, PlayerModeComponent->GetCurrentMode());
            }
        }
    }

    // �������� ����
    if (HasAuthority())
    {
        ServerSelectInventorySlot_Implementation(Slot);
    }
    else
    {
        ServerSelectInventorySlot(Slot);
    }
}

bool ACitizen::ServerSelectInventorySlot_Validate(EInventorySlot Slot)
{
    return true;
}

void ACitizen::ServerSelectInventorySlot_Implementation(EInventorySlot Slot)
{
    if (!InvenComponent || !PlayerModeComponent) return;

    if (InvenComponent->GetCurrentSelectedSlot() == Slot)
    {
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
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
        // Ŭ���̾�Ʈ���� ���� ��� ���� ó��
        if (!HasAuthority() && IsLocallyControlled())
        {
            OnPlayerModeChanged(EPlayerMode::Build, PlayerModeComponent->GetCurrentMode());
            if (BuildingComponent)
            {
                BuildingComponent->OnBuildModeEntered();  // _Implementation ����
            }
        }
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

    if (BuildingComponent)
    {
        PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, BuildingComponent, &UBuildingComponent::RotateBuildPreview);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, BuildingComponent, &UBuildingComponent::AttemptBuild);
    }

    if (CombatComponent)
    {
        PlayerInputComponent->BindAction("Zoom", IE_Pressed, CombatComponent, &UCombatComponent::HandleAim);
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
    if (HasAuthority())
    {
        ServerInteract_Implementation();
    }
    else
    {
        ServerInteract();
    }
}

bool ACitizen::ServerInteract_Validate()
{
    return true;
}

void ACitizen::ServerInteract_Implementation()
{
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
        return;
    }

    TArray<AActor*> OverlappedActors;

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        InteractionRange,
        32,
        FColor::Green,
        false,
        0.1f,
        0,
        1.0f
    );

    if (UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        InteractionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AItem::StaticClass(),
        TArray<AActor*>(),
        OverlappedActors))
    {
        AActor* ClosestItem = nullptr;
        float ClosestDistance = InteractionRange;

        for (AActor* Actor : OverlappedActors)
        {
            float Distance = FVector::Distance(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                DrawDebugLine(
                    GetWorld(),
                    GetActorLocation(),
                    Actor->GetActorLocation(),
                    FColor::Yellow,
                    false,
                    0.1f,
                    0,
                    1.0f
                );

                ClosestDistance = Distance;
                ClosestItem = Actor;
            }
        }

        if (ClosestItem)
        {
            DrawDebugLine(
                GetWorld(),
                GetActorLocation(),
                ClosestItem->GetActorLocation(),
                FColor::Red,
                false,
                0.1f,
                0,
                2.0f
            );

            if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(ClosestItem))
            {
                Trophy->PickUp(this);
                HeldTrophy = Trophy;
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Picked up Trophy"));
            }
            else if (AItem_Gun* Gun = Cast<AItem_Gun>(ClosestItem))
            {
                FItemData* GunData = InvenComponent->GetItemData(EInventorySlot::Gun);
                if (!GunData || GunData->Count == 0)
                {
                    AddItem(EInventorySlot::Gun, 1);
                    Gun->Destroy();
                    UE_LOG(LogTemp, Warning, TEXT("Picked up gun with ammo: %d"), Gun->GetCurrentAmmo());
                    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                        FString::Printf(TEXT("Picked up gun with %d ammo"), Gun->GetCurrentAmmo()));
                }
                else
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Already has a gun"));
                }
            }
            else if (AItem* Item = Cast<AItem>(ClosestItem))
            {
                AddItem(Item->ItemType, Item->Amount);
                Item->Destroy();
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Picked up Item"));
            }
        }
    }
}