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
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
        AttachToActor(GetOwner(), AttachRules);

        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            FVector ForwardOffset = Character->GetActorForwardVector() * 100.0f;
            FVector UpOffset = FVector(0.0f, 0.0f, 50.0f);
            SetActorRelativeLocation(ForwardOffset + UpOffset);
            SetActorRotation(Character->GetActorRotation());
        }
    }
}

void AItem_Telescope::OnRep_ZoomState()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        if (UCameraComponent* Camera = Player->GetFollowCamera())
        {
            if (bIsZoomed)
            {
                DefaultFOV = Camera->FieldOfView;
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }
                Camera->SetFieldOfView(ZoomedFOV);
            }
            else
            {
                Camera->SetFieldOfView(DefaultFOV);
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    SpringArm->TargetArmLength = DefaultArmLength;
                }
            }
        }
    }
}

void AItem_Telescope::PickUp_Implementation(ACharacter* Character)
{
    Super::PickUp_Implementation(Character);

    if (!Character || !HasAuthority()) return;

    bIsHeld = true;
    OnRep_HeldState();
}

void AItem_Telescope::ToggleZoom_Implementation()
{
    if (!HasAuthority()) return;
    bIsZoomed = !bIsZoomed;
    OnRep_ZoomState();
}

void AItem_Telescope::Drop_Implementation()
{
    Super::Drop_Implementation();

    if (!HasAuthority()) return;

    if (bIsZoomed)
    {
        bIsZoomed = false;
        OnRep_ZoomState();
        if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
        {
            ResetCameraSettings(Player);
        }
    }

    bIsHeld = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AItem_Telescope::ResetCameraSettings(ACitizen* Player)
{
    if (UCameraComponent* Camera = Player->GetFollowCamera())
    {
        Camera->SetFieldOfView(DefaultFOV);
        if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
        {
            SpringArm->bUsePawnControlRotation = true;
            SpringArm->bEnableCameraLag = false;
            SpringArm->bEnableCameraRotationLag = false;
            if (ACharacter* Character = Cast<ACharacter>(Player))
            {
                Character->bUseControllerRotationYaw = true;
            }
        }
    }
}
