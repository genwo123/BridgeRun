// CombatComponent.cpp
#include "CombatComponent.h"
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerModeTypes.h"
#include "Camera/CameraComponent.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCitizen = Cast<ACitizen>(GetOwner());
}

void UCombatComponent::OnCombatModeEntered()
{
    if (OwnerCitizen)
    {
        if (EquippedGun)  // 총을 들고 있을 때
        {
            OwnerCitizen->bUseControllerRotationYaw = true;  // 마우스 방향으로 회전
            if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
            {
                MovementComp->bOrientRotationToMovement = false;  // 이동 방향으로 회전하지 않음
            }
        }
        else  // 망원경 모드일 때
        {
            OwnerCitizen->bUseControllerRotationPitch = false;
            OwnerCitizen->bUseControllerRotationYaw = false;
            OwnerCitizen->bUseControllerRotationRoll = false;
        }
    }
}

void UCombatComponent::OnCombatModeExited()
{
    if (EquippedTelescope)
    {
        OnTelescopeUnequipped();
    }
    if (EquippedGun)
    {
        OnGunUnequipped();
    }

    if (OwnerCitizen)
    {
        if (UCharacterMovementComponent* MovementComponent = OwnerCitizen->GetCharacterMovement())
        {
            MovementComponent->bOrientRotationToMovement = true;
            OwnerCitizen->bUseControllerRotationYaw = false;
        }
    }
}

void UCombatComponent::OnTelescopeEquipped(AItem_Telescope* Telescope)
{
    EquippedTelescope = Telescope;
    if (EquippedTelescope)
    {
        EquippedTelescope->PickUp(OwnerCitizen);
    }
}

void UCombatComponent::OnTelescopeUnequipped()
{
    if (EquippedTelescope)
    {
        if (EquippedTelescope->bIsZoomed)
        {
            EquippedTelescope->ToggleZoom();
        }
        ResetCameraSettings();
        EquippedTelescope->Destroy();
        EquippedTelescope = nullptr;
    }
}

void UCombatComponent::OnGunEquipped(AItem_Gun* Gun)
{
    if (!Gun || !OwnerCitizen || !GetWorld()) return;

    if (IsValid(EquippedGun) && EquippedGun != Gun)
    {
        if (EquippedGun->IsAiming())
        {
            EquippedGun->ToggleAim();
        }
        EquippedGun->SetActorHiddenInGame(true);
        EquippedGun->SetActorEnableCollision(false);
    }

    EquippedGun = Gun;
    EquippedGun->PickUp(OwnerCitizen);

    UE_LOG(LogTemp, Warning, TEXT("Gun equipped with ammo: %d"), EquippedGun->GetCurrentAmmo());
}

void UCombatComponent::OnGunUnequipped()
{
    if (!IsValid(EquippedGun)) return;

    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    EquippedGun->SetActorHiddenInGame(true);
    EquippedGun->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Warning, TEXT("Gun unequipped with ammo: %d"), EquippedGun->GetCurrentAmmo());
    EquippedGun = nullptr;
}

void UCombatComponent::DropCurrentWeapon()
{
    if (!EquippedGun) return;

    UE_LOG(LogTemp, Warning, TEXT("Dropping gun with ammo: %d"), EquippedGun->GetCurrentAmmo());

    if (OwnerCitizen && OwnerCitizen->GetInvenComponent())
    {
        OwnerCitizen->GetInvenComponent()->UpdateItemCount(EInventorySlot::Gun, -1);
    }

    EquippedGun->ThrowForward();
    bHasGun = false;
    EquippedGun = nullptr;

    if (OwnerCitizen)
    {
        OwnerCitizen->GetPlayerModeComponent()->SetPlayerMode(EPlayerMode::Normal);
        OwnerCitizen->GetInvenComponent()->SetCurrentSelectedSlot(EInventorySlot::None);
    }
}

void UCombatComponent::HandleShoot()
{
    UPlayerModeComponent* PlayerMode = OwnerCitizen ? OwnerCitizen->GetPlayerModeComponent() : nullptr;
    if (!OwnerCitizen || !PlayerMode || PlayerMode->GetCurrentMode() != EPlayerMode::Combat)
        return;

    if (EquippedGun)
    {
        EquippedGun->Fire();
    }
}

void UCombatComponent::HandleAim()
{
    UPlayerModeComponent* PlayerMode = OwnerCitizen ? OwnerCitizen->GetPlayerModeComponent() : nullptr;
    if (!OwnerCitizen || !PlayerMode || PlayerMode->GetCurrentMode() != EPlayerMode::Combat)
        return;

    if (EquippedTelescope)
    {
        EquippedTelescope->ToggleZoom();
    }
    else if (EquippedGun)
    {
        EquippedGun->ToggleAim();
    }
}

void UCombatComponent::ResetCameraSettings()
{
    if (OwnerCitizen)
    {
        OwnerCitizen->bUseControllerRotationPitch = false;
        OwnerCitizen->bUseControllerRotationYaw = false;
        OwnerCitizen->bUseControllerRotationRoll = false;

        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = true;
            MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
            MovementComp->bUseControllerDesiredRotation = false;
        }

        if (USpringArmComponent* SpringArm = OwnerCitizen->FindComponentByClass<USpringArmComponent>())
        {
            SpringArm->bUsePawnControlRotation = true;
            SpringArm->TargetArmLength = 300.0f;
        }

        if (UCameraComponent* Camera = OwnerCitizen->GetFollowCamera())
        {
            Camera->bUsePawnControlRotation = false;
        }
    }
}