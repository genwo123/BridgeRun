
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

    // 플레이어의 앞쪽 방향으로 위치 설정
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,  // 타겟에 스냅
        EAttachmentRule::SnapToTarget,  // 회전도 타겟 기준
        EAttachmentRule::KeepWorld,     // 스케일은 유지
        true);                          // 위치 계산 시 웰딩 사용

    AttachToActor(Player, AttachRules);

    // 플레이어 앞쪽으로 일정 거리 떨어진 위치에 배치
    FVector ForwardOffset = Player->GetActorForwardVector() * 100.0f;  // 앞으로 100 유닛
    FVector UpOffset = FVector(0.0f, 0.0f, 50.0f);  // 위로 50 유닛
    SetActorRelativeLocation(ForwardOffset + UpOffset);

    // 플레이어의 회전값을 따라가도록 설정
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
                // 기존 카메라 설정 저장
                DefaultFOV = Camera->FieldOfView;

                // SpringArm 길이를 0으로 설정하여 1인칭 시점으로 전환
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }

                // FOV 변경으로 확대
                Camera->SetFieldOfView(ZoomedFOV);
            }
            else
            {
                // 원래 FOV로 복구
                Camera->SetFieldOfView(DefaultFOV);

                // SpringArm 길이를 원래대로 복구하여 3인칭 시점으로 전환
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
            // 카메라 회전 설정 유지
            SpringArm->bUsePawnControlRotation = true;

            // 카메라 지연 효과는 기존대로 유지
            SpringArm->bEnableCameraLag = false;
            SpringArm->bEnableCameraRotationLag = false;

            // 캐릭터 회전 설정도 유지
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
    // 부모 클래스의 OnOverlapBegin은 호출하지 않음 (자동 획득 방지)
    UE_LOG(LogTemp, Warning, TEXT("Telescope OnOverlapBegin with: %s"), *OtherActor->GetName());
}