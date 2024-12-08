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
        if (EquippedGun)  // ���� ��� ���� ��
        {
            OwnerCitizen->bUseControllerRotationYaw = true;  // ���콺 �������� ȸ��
            if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
            {
                MovementComp->bOrientRotationToMovement = false;  // �̵� �������� ȸ������ ����
            }
        }
        else  // ������ ����� ��
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
        // �� ���� üũ �� ����
        if (EquippedTelescope->bIsZoomed)
        {
            EquippedTelescope->ToggleZoom();  // �� ���� ����
        }

        ResetCameraSettings();
        EquippedTelescope->Destroy();
        EquippedTelescope = nullptr;
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

void UCombatComponent::DropCurrentWeapon()
{
    if (EquippedGun)
    {
        FString GunTag = EquippedGun->GetGunTag();
        if (!GunTag.IsEmpty())
        {
            GunAmmoStorage[GunTag] = EquippedGun->GetCurrentAmmo();
        }

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
}


void UCombatComponent::OnGunEquipped(AItem_Gun* Gun)
{
    // ������ üũ
    if (!Gun || !OwnerCitizen || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnGunEquipped: Invalid pointers detected"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("===== OnGunEquipped Start ====="));

    // ���� ���� ������ ���� ����
    if (IsValid(EquippedGun) && EquippedGun != Gun)
    {
        if (!EquippedGun->GetGunTag().IsEmpty())
        {
            const int32 AmmoToStore = EquippedGun->GetCurrentAmmo();
            GunAmmoStorage.Add(EquippedGun->GetGunTag(), AmmoToStore);

            UE_LOG(LogTemp, Warning, TEXT("Storing previous gun state - Tag: [%s], Ammo: %d"),
                *EquippedGun->GetGunTag(), AmmoToStore);
        }

        // ���� �� �����ϰ� ó��
        EquippedGun->SetActorHiddenInGame(true);
        EquippedGun->SetActorEnableCollision(false);
    }

    // �� �� ����
    EquippedGun = Gun;

    // ����� ź�� ���� ���� �õ�
    const FString CurrentGunTag = Gun->GetGunTag();
    if (!CurrentGunTag.IsEmpty() && GunAmmoStorage.Contains(CurrentGunTag))
    {
        const int32* FoundAmmo = GunAmmoStorage.Find(CurrentGunTag);
        if (FoundAmmo)
        {
            Gun->SetAmmo(*FoundAmmo);
            UE_LOG(LogTemp, Warning, TEXT("Restored ammo for gun [%s]: %d"),
                *CurrentGunTag, *FoundAmmo);
        }
    }
    else
    {
        GunAmmoStorage.Add(CurrentGunTag, Gun->GetCurrentAmmo());
        UE_LOG(LogTemp, Warning, TEXT("Storing new gun state - Tag: [%s], Ammo: %d"),
            *CurrentGunTag, Gun->GetCurrentAmmo());
    }

    // �� ����
    Gun->PickUp(OwnerCitizen);

    UE_LOG(LogTemp, Warning, TEXT("===== OnGunEquipped End ====="));
}

void UCombatComponent::OnGunUnequipped()
{
    if (!IsValid(EquippedGun))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnGunUnequipped: No valid gun to unequip"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("===== OnGunUnequipped Start ====="));

    // ���� ���� ���� �õ�
    const FString CurrentGunTag = EquippedGun->GetGunTag();
    if (!CurrentGunTag.IsEmpty())
    {
        const int32 CurrentAmmo = EquippedGun->GetCurrentAmmo();
        GunAmmoStorage.Add(CurrentGunTag, CurrentAmmo);

        UE_LOG(LogTemp, Warning, TEXT("Storing gun state - Tag: [%s], Ammo: %d"),
            *CurrentGunTag, CurrentAmmo);
    }

    // ���� ���� ����
    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    // �� �����
    EquippedGun->SetActorHiddenInGame(true);
    EquippedGun->SetActorEnableCollision(false);
    EquippedGun = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("===== OnGunUnequipped End ====="));
}