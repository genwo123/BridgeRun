// Private/Modes/Components/CombatComponent.cpp 
#include "Modes/CombatComponent.h"
#include "Characters/Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Modes/PlayerModeTypes.h"
#include "Modes/PlayerModeComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, bHasGun);
    DOREPLIFETIME(UCombatComponent, EquippedTelescope);
    DOREPLIFETIME(UCombatComponent, EquippedGun);
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
        if (EquippedGun)
        {
            OwnerCitizen->bUseControllerRotationYaw = true;
            if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
            {
                MovementComp->bOrientRotationToMovement = false;
            }
        }
        else
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

void UCombatComponent::OnTelescopeEquipped_Implementation(AItem_Telescope* Telescope)
{
    if (!GetOwner()->HasAuthority()) return;

    EquippedTelescope = Telescope;
    if (EquippedTelescope)
    {
        EquippedTelescope->PickUp(OwnerCitizen);
    }
}

void UCombatComponent::OnTelescopeUnequipped_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

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

void UCombatComponent::OnGunEquipped_Implementation(AItem_Gun* Gun)
{
    if (!GetOwner()->HasAuthority() || !Gun || !OwnerCitizen || !GetWorld()) return;

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
}

void UCombatComponent::OnGunUnequipped_Implementation()
{
    if (!GetOwner()->HasAuthority() || !IsValid(EquippedGun)) return;

    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    EquippedGun->SetActorHiddenInGame(true);
    EquippedGun->SetActorEnableCollision(false);
    EquippedGun = nullptr;
}

void UCombatComponent::DropCurrentWeapon_Implementation()
{
    if (!GetOwner()->HasAuthority() || !EquippedGun) return;

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

void UCombatComponent::HandleShoot_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    UPlayerModeComponent* PlayerMode = OwnerCitizen ? OwnerCitizen->GetPlayerModeComponent() : nullptr;
    if (!OwnerCitizen || !PlayerMode || PlayerMode->GetCurrentMode() != EPlayerMode::Combat)
        return;

    if (EquippedGun)
    {
        EquippedGun->Fire();
    }
}

void UCombatComponent::HandleAim_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

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