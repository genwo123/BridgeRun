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
    if (!OwnerCitizen)
        return;

    // ���� ���� ���ο� ���� ĳ���� ȸ�� ����
    UpdateCharacterRotation(EquippedGun != nullptr);
}

void UCombatComponent::OnCombatModeExited()
{
    // ��� ����
    if (EquippedTelescope)
    {
        OnTelescopeUnequipped();
    }

    if (EquippedGun)
    {
        OnGunUnequipped();
    }

    // ĳ���� �̵� ���� ����
    if (OwnerCitizen)
    {
        if (UCharacterMovementComponent* MovementComponent = OwnerCitizen->GetCharacterMovement())
        {
            MovementComponent->bOrientRotationToMovement = true;
            OwnerCitizen->bUseControllerRotationYaw = false;
        }
    }
}

void UCombatComponent::HandleShoot_Implementation()
{
    if (!GetOwner()->HasAuthority())
        return;

    // ���� ��� �� ������ Ȯ��
    if (!IsValidCombatState())
        return;

    // �߻� ó��
    if (EquippedGun)
    {
        EquippedGun->Fire();
    }
}

void UCombatComponent::HandleAim_Implementation()
{
    if (!GetOwner()->HasAuthority())
        return;

    // ���� ��� �� ������ Ȯ��
    if (!IsValidCombatState())
        return;

    // ��� ���� ���� ó��
    if (EquippedTelescope)
    {
        EquippedTelescope->ToggleZoom();
    }
    else if (EquippedGun)
    {
        EquippedGun->ToggleAim();
    }
}

void UCombatComponent::DropCurrentWeapon_Implementation()
{
    if (!GetOwner()->HasAuthority() || !EquippedGun)
        return;

    // ���� ���� Ȯ�� �� ����
    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    // �κ��丮 ������Ʈ
    if (OwnerCitizen && OwnerCitizen->GetInvenComponent())
    {
        OwnerCitizen->GetInvenComponent()->UpdateItemCount(EInventorySlot::Gun, -1);
    }

    // ���� ���� �� ���� ������Ʈ
    AItem_Gun* GunToThrow = EquippedGun;
    bHasGun = false;
    EquippedGun = nullptr;

    // ī�޶� �� �÷��̾� ��� ����
    if (OwnerCitizen)
    {
        ResetCameraSettings();
        OwnerCitizen->GetPlayerModeComponent()->SetPlayerMode(EPlayerMode::Normal);
        OwnerCitizen->GetInvenComponent()->SetCurrentSelectedSlot(EInventorySlot::None);
    }

    // ����� ������ �ѱ� ������
    if (GunToThrow)
    {
        GunToThrow->ThrowForward();
    }
}

void UCombatComponent::OnTelescopeEquipped_Implementation(AItem_Telescope* Telescope)
{
    if (!GetOwner()->HasAuthority())
        return;

    EquippedTelescope = Telescope;
    if (EquippedTelescope)
    {
        EquippedTelescope->PickUp(OwnerCitizen);
    }
}

void UCombatComponent::OnTelescopeUnequipped_Implementation()
{
    if (!GetOwner()->HasAuthority())
        return;

    if (EquippedTelescope)
    {
        // Ȯ�� ���̸� ����
        if (EquippedTelescope->bIsZoomed)
        {
            EquippedTelescope->ToggleZoom();
        }

        // ī�޶� ���� �ʱ�ȭ �� ��ü ����
        ResetCameraSettings();
        EquippedTelescope->Destroy();
        EquippedTelescope = nullptr;
    }
}

void UCombatComponent::OnGunEquipped_Implementation(AItem_Gun* Gun)
{
    if (!GetOwner()->HasAuthority() || !Gun || !OwnerCitizen || !GetWorld())
        return;

    // ���� �ѱ� �����
    if (IsValid(EquippedGun) && EquippedGun != Gun)
    {
        HideCurrentGun();
    }

    // �� �ѱ� ����
    EquippedGun = Gun;
    EquippedGun->PickUp(OwnerCitizen);
}

void UCombatComponent::OnGunUnequipped_Implementation()
{
    if (!GetOwner()->HasAuthority() || !IsValid(EquippedGun))
        return;

    HideCurrentGun();
    EquippedGun = nullptr;
}


void UCombatComponent::HideCurrentGun()
{
    if (!EquippedGun)
        return;

    // ���� ���� ����
    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    // �ѱ� ����� �� �浹 ��Ȱ��ȭ
    EquippedGun->SetActorHiddenInGame(true);
    EquippedGun->SetActorEnableCollision(false);
}

bool UCombatComponent::IsValidCombatState() const
{
    UPlayerModeComponent* PlayerMode = OwnerCitizen ? OwnerCitizen->GetPlayerModeComponent() : nullptr;
    return (OwnerCitizen && PlayerMode && PlayerMode->GetCurrentMode() == EPlayerMode::Combat);
}

void UCombatComponent::UpdateCharacterRotation(bool bAllowYawRotation)
{
    if (!OwnerCitizen)
        return;

    if (bAllowYawRotation)
    {
        // �ѱ� ���� �� ĳ���Ͱ� ī�޶� ��(yaw) ȸ���� ����
        OwnerCitizen->bUseControllerRotationYaw = true;
        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = false;
        }
    }
    else
    {
        // �⺻ ���´� ī�޶�� �������� ȸ��
        OwnerCitizen->bUseControllerRotationPitch = false;
        OwnerCitizen->bUseControllerRotationYaw = false;
        OwnerCitizen->bUseControllerRotationRoll = false;
    }
}

void UCombatComponent::ResetCameraSettings()
{
    if (!OwnerCitizen)
        return;

    // ĳ���� ȸ�� ���� �ʱ�ȭ
    OwnerCitizen->bUseControllerRotationPitch = false;
    OwnerCitizen->bUseControllerRotationYaw = false;
    OwnerCitizen->bUseControllerRotationRoll = false;

    // �̵� ������Ʈ ���� �ʱ�ȭ
    if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MovementComp->bUseControllerDesiredRotation = false;
    }

    // �������� ���� �ʱ�ȭ
    if (USpringArmComponent* SpringArm = OwnerCitizen->FindComponentByClass<USpringArmComponent>())
    {
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->TargetArmLength = 300.0f;
    }

    // ī�޶� ���� �ʱ�ȭ
    if (UCameraComponent* Camera = OwnerCitizen->GetFollowCamera())
    {
        Camera->bUsePawnControlRotation = false;
    }
}