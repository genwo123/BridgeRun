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

    // 무기 장착 여부에 따라 캐릭터 회전 설정
    UpdateCharacterRotation(EquippedGun != nullptr);
}

void UCombatComponent::OnCombatModeExited()
{
    // 장비 해제
    if (EquippedTelescope)
    {
        OnTelescopeUnequipped();
    }

    if (EquippedGun)
    {
        OnGunUnequipped();
    }

    // 캐릭터 이동 설정 복원
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

    // 전투 모드 및 소유자 확인
    if (!IsValidCombatState())
        return;

    // 발사 처리
    if (EquippedGun)
    {
        EquippedGun->Fire();
    }
}

void UCombatComponent::HandleAim_Implementation()
{
    if (!GetOwner()->HasAuthority())
        return;

    // 전투 모드 및 소유자 확인
    if (!IsValidCombatState())
        return;

    // 장비에 따른 조준 처리
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

    // 조준 상태 확인 및 해제
    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    // 인벤토리 업데이트
    if (OwnerCitizen && OwnerCitizen->GetInvenComponent())
    {
        OwnerCitizen->GetInvenComponent()->UpdateItemCount(EInventorySlot::Gun, -1);
    }

    // 참조 저장 및 상태 업데이트
    AItem_Gun* GunToThrow = EquippedGun;
    bHasGun = false;
    EquippedGun = nullptr;

    // 카메라 및 플레이어 모드 리셋
    if (OwnerCitizen)
    {
        ResetCameraSettings();
        OwnerCitizen->GetPlayerModeComponent()->SetPlayerMode(EPlayerMode::Normal);
        OwnerCitizen->GetInvenComponent()->SetCurrentSelectedSlot(EInventorySlot::None);
    }

    // 저장된 참조로 총기 던지기
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
        // 확대 중이면 해제
        if (EquippedTelescope->bIsZoomed)
        {
            EquippedTelescope->ToggleZoom();
        }

        // 카메라 설정 초기화 및 객체 정리
        ResetCameraSettings();
        EquippedTelescope->Destroy();
        EquippedTelescope = nullptr;
    }
}

void UCombatComponent::OnGunEquipped_Implementation(AItem_Gun* Gun)
{
    if (!GetOwner()->HasAuthority() || !Gun || !OwnerCitizen || !GetWorld())
        return;

    // 기존 총기 숨기기
    if (IsValid(EquippedGun) && EquippedGun != Gun)
    {
        HideCurrentGun();
    }

    // 새 총기 장착
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

    // 조준 상태 해제
    if (EquippedGun->IsAiming())
    {
        EquippedGun->ToggleAim();
    }

    // 총기 숨기기 및 충돌 비활성화
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
        // 총기 장착 시 캐릭터가 카메라 요(yaw) 회전을 따름
        OwnerCitizen->bUseControllerRotationYaw = true;
        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = false;
        }
    }
    else
    {
        // 기본 상태는 카메라와 독립적인 회전
        OwnerCitizen->bUseControllerRotationPitch = false;
        OwnerCitizen->bUseControllerRotationYaw = false;
        OwnerCitizen->bUseControllerRotationRoll = false;
    }
}

void UCombatComponent::ResetCameraSettings()
{
    if (!OwnerCitizen)
        return;

    // 캐릭터 회전 설정 초기화
    OwnerCitizen->bUseControllerRotationPitch = false;
    OwnerCitizen->bUseControllerRotationYaw = false;
    OwnerCitizen->bUseControllerRotationRoll = false;

    // 이동 컴포넌트 설정 초기화
    if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MovementComp->bUseControllerDesiredRotation = false;
    }

    // 스프링암 설정 초기화
    if (USpringArmComponent* SpringArm = OwnerCitizen->FindComponentByClass<USpringArmComponent>())
    {
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->TargetArmLength = 300.0f;
    }

    // 카메라 설정 초기화
    if (UCameraComponent* Camera = OwnerCitizen->GetFollowCamera())
    {
        Camera->bUsePawnControlRotation = false;
    }
}