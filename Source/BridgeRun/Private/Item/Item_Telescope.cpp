// Private/Item/Item_Telescope.cpp
#include "Item/Item_Telescope.h"
#include "Characters/Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

AItem_Telescope::AItem_Telescope()
{
    bReplicates = true;
    bIsHeld = false;
    ItemType = EInventorySlot::Telescope;
}

void AItem_Telescope::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Telescope, bIsHeld);
    DOREPLIFETIME(AItem_Telescope, bIsZoomed);
}

void AItem_Telescope::OnRep_HeldState()
{
    if (bIsHeld && GetOwner())
    {
        // �����ڿ��� ����
        UpdateCameraTransform(Cast<ACitizen>(GetOwner()));
    }
}

void AItem_Telescope::UpdateCameraTransform(ACitizen* Player)
{
    if (!Player) return;

    // �����ڿ��� ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        true
    );

    AttachToActor(Player, AttachRules);

    // ����� ��ġ �� ȸ�� ����
    SetActorRelativeLocation(FVector(100.0f, 30.0f, 50.0f));  // ĳ���� ���� ����, �ణ ������, ����
    SetActorRotation(Player->GetActorRotation());
}

void AItem_Telescope::OnRep_ZoomState()
{
    ACitizen* Player = Cast<ACitizen>(GetOwner());
    if (!Player) return;

    // �� ���� ����
    ApplyZoomSettings(Player, bIsZoomed);
}

void AItem_Telescope::ApplyZoomSettings(ACitizen* Player, bool bZoom)
{
    if (!Player) return;

    UCameraComponent* Camera = Player->GetFollowCamera();
    if (!Camera) return;

    USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>();

    if (bZoom)
    {
        // �� ���� - ���� ī�޶� ���� ����
        DefaultFOV = Camera->FieldOfView;

        if (SpringArm)
        {
            // �������� ���� ���� �� ����
            DefaultArmLength = SpringArm->TargetArmLength;
            SpringArm->TargetArmLength = 0.0f;
        }

        // �� FOV ����
        Camera->SetFieldOfView(ZoomedFOV);
    }
    else
    {
        // �� ���� - �⺻ ���� ����
        Camera->SetFieldOfView(DefaultFOV);

        if (SpringArm)
        {
            // �������� ���� ����
            SpringArm->TargetArmLength = DefaultArmLength;
        }
    }
}

void AItem_Telescope::PickUp_Implementation(ACharacter* Character)
{
    Super::PickUp_Implementation(Character);

    if (!Character || !HasAuthority()) return;

    // ���� ������Ʈ
    bIsHeld = true;

    // Ŭ���̾�Ʈ ����ȭ�� ���� �̺�Ʈ ȣ��
    OnRep_HeldState();
}

void AItem_Telescope::ToggleZoom_Implementation()
{
    if (!HasAuthority()) return;

    // �� ���� ���
    bIsZoomed = !bIsZoomed;

    // Ŭ���̾�Ʈ ����ȭ�� ���� �̺�Ʈ ȣ��
    OnRep_ZoomState();
}

void AItem_Telescope::Drop_Implementation()
{
    Super::Drop_Implementation();

    if (!HasAuthority()) return;

    // �� ���¿����� ����
    if (bIsZoomed)
    {
        bIsZoomed = false;
        OnRep_ZoomState();

        // ī�޶� ���� �ʱ�ȭ
        ACitizen* Player = Cast<ACitizen>(GetOwner());
        if (Player)
        {
            ResetCameraSettings(Player);
        }
    }

    // ���� ������Ʈ �� �и�
    bIsHeld = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AItem_Telescope::ResetCameraSettings(ACitizen* Player)
{
    if (!Player) return;

    // ī�޶� ���� �ʱ�ȭ
    UCameraComponent* Camera = Player->GetFollowCamera();
    if (!Camera) return;

    // FOV �ʱ�ȭ
    Camera->SetFieldOfView(DefaultFOV);

    // �������� ���� �ʱ�ȭ
    USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>();
    if (SpringArm)
    {
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bEnableCameraLag = false;
        SpringArm->bEnableCameraRotationLag = false;

        // ĳ���� ȸ�� ����
        ACharacter* Character = Cast<ACharacter>(Player);
        if (Character)
        {
            Character->bUseControllerRotationYaw = true;
        }
    }
}