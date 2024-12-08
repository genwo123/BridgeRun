
// Item_Telescope.cpp
#include "Item_Telescope.h"
#include "Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

AItem_Telescope::AItem_Telescope()
{
    bIsHeld = false;
    ItemType = EInventorySlot::Telescope;
}

void AItem_Telescope::PickUp(ACitizen* Player)
{
    if (!Player) return;

    bIsHeld = true;

    // �÷��̾��� ���� �������� ��ġ ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,  // Ÿ�ٿ� ����
        EAttachmentRule::SnapToTarget,  // ȸ���� Ÿ�� ����
        EAttachmentRule::KeepWorld,     // �������� ����
        true);                          // ��ġ ��� �� ���� ���

    AttachToActor(Player, AttachRules);

    // �÷��̾� �������� ���� �Ÿ� ������ ��ġ�� ��ġ
    FVector ForwardOffset = Player->GetActorForwardVector() * 100.0f;  // ������ 100 ����
    FVector UpOffset = FVector(0.0f, 0.0f, 50.0f);  // ���� 50 ����
    SetActorRelativeLocation(ForwardOffset + UpOffset);

    // �÷��̾��� ȸ������ ���󰡵��� ����
    SetActorRotation(Player->GetActorRotation());
}

void AItem_Telescope::ToggleZoom()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        if (UCameraComponent* Camera = Player->GetFollowCamera())
        {
            if (!bIsZoomed)
            {
                // ���� ī�޶� ���� ����
                DefaultFOV = Camera->FieldOfView;

                // SpringArm ���̸� 0���� �����Ͽ� 1��Ī �������� ��ȯ
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }

                // FOV �������� Ȯ��
                Camera->SetFieldOfView(ZoomedFOV);
            }
            else
            {
                // ���� FOV�� ����
                Camera->SetFieldOfView(DefaultFOV);

                // SpringArm ���̸� ������� �����Ͽ� 3��Ī �������� ��ȯ
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    SpringArm->TargetArmLength = DefaultArmLength;
                }
            }
            bIsZoomed = !bIsZoomed;
        }
    }
}

void AItem_Telescope::Drop()
{
    if (bIsZoomed)
    {
        if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
        {
            ResetCameraSettings(Player);
        }
        bIsZoomed = false;
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
            // ī�޶� ȸ�� ���� ����
            SpringArm->bUsePawnControlRotation = true;

            // ī�޶� ���� ȿ���� ������� ����
            SpringArm->bEnableCameraLag = false;
            SpringArm->bEnableCameraRotationLag = false;

            // ĳ���� ȸ�� ������ ����
            if (ACharacter* Character = Cast<ACharacter>(Player))
            {
                Character->bUseControllerRotationYaw = true;
            }
        }
    }
}

void AItem_Telescope::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // �θ� Ŭ������ OnOverlapBegin�� ȣ������ ���� (�ڵ� ȹ�� ����)
    UE_LOG(LogTemp, Warning, TEXT("Telescope OnOverlapBegin with: %s"), *OtherActor->GetName());
}