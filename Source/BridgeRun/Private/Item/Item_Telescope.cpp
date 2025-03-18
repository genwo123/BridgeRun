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
        // 소유자에게 부착
        UpdateCameraTransform(Cast<ACitizen>(GetOwner()));
    }
}

void AItem_Telescope::UpdateCameraTransform(ACitizen* Player)
{
    if (!Player) return;

    // 소유자에게 부착
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        true
    );

    AttachToActor(Player, AttachRules);

    // 상대적 위치 및 회전 설정
    SetActorRelativeLocation(FVector(100.0f, 30.0f, 50.0f));  // 캐릭터 기준 앞쪽, 약간 오른쪽, 위쪽
    SetActorRotation(Player->GetActorRotation());
}

void AItem_Telescope::OnRep_ZoomState()
{
    ACitizen* Player = Cast<ACitizen>(GetOwner());
    if (!Player) return;

    // 줌 설정 적용
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
        // 줌 시작 - 현재 카메라 설정 저장
        DefaultFOV = Camera->FieldOfView;

        if (SpringArm)
        {
            // 스프링암 설정 저장 및 변경
            DefaultArmLength = SpringArm->TargetArmLength;
            SpringArm->TargetArmLength = 0.0f;
        }

        // 줌 FOV 적용
        Camera->SetFieldOfView(ZoomedFOV);
    }
    else
    {
        // 줌 종료 - 기본 설정 복원
        Camera->SetFieldOfView(DefaultFOV);

        if (SpringArm)
        {
            // 스프링암 설정 복원
            SpringArm->TargetArmLength = DefaultArmLength;
        }
    }
}

void AItem_Telescope::PickUp_Implementation(ACharacter* Character)
{
    Super::PickUp_Implementation(Character);

    if (!Character || !HasAuthority()) return;

    // 상태 업데이트
    bIsHeld = true;

    // 클라이언트 동기화를 위한 이벤트 호출
    OnRep_HeldState();
}

void AItem_Telescope::ToggleZoom_Implementation()
{
    if (!HasAuthority()) return;

    // 줌 상태 토글
    bIsZoomed = !bIsZoomed;

    // 클라이언트 동기화를 위한 이벤트 호출
    OnRep_ZoomState();
}

void AItem_Telescope::Drop_Implementation()
{
    Super::Drop_Implementation();

    if (!HasAuthority()) return;

    // 줌 상태였으면 해제
    if (bIsZoomed)
    {
        bIsZoomed = false;
        OnRep_ZoomState();

        // 카메라 설정 초기화
        ACitizen* Player = Cast<ACitizen>(GetOwner());
        if (Player)
        {
            ResetCameraSettings(Player);
        }
    }

    // 상태 업데이트 및 분리
    bIsHeld = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AItem_Telescope::ResetCameraSettings(ACitizen* Player)
{
    if (!Player) return;

    // 카메라 설정 초기화
    UCameraComponent* Camera = Player->GetFollowCamera();
    if (!Camera) return;

    // FOV 초기화
    Camera->SetFieldOfView(DefaultFOV);

    // 스프링암 설정 초기화
    USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>();
    if (SpringArm)
    {
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bEnableCameraLag = false;
        SpringArm->bEnableCameraRotationLag = false;

        // 캐릭터 회전 설정
        ACharacter* Character = Cast<ACharacter>(Player);
        if (Character)
        {
            Character->bUseControllerRotationYaw = true;
        }
    }
}