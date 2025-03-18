// Private/Item/Item_Gun.cpp
#include "Item/Item_Gun.h"
#include "Characters/Citizen.h"
#include "Item/Item_Tent.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

AItem_Gun::AItem_Gun()
{
    bReplicates = true;
    bIsHeld = false;
    ItemType = EInventorySlot::Gun;
    CurrentAmmo = MaxAmmo;

    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
    }
}

void AItem_Gun::BeginPlay()
{
    Super::BeginPlay();

    // 초기 상태 설정
    CurrentAmmo = MaxAmmo;

    if (MeshComponent)
    {
        if (bIsHeld)
        {
            // 들고 있을 때 설정
            MeshComponent->SetSimulatePhysics(false);
            MeshComponent->SetEnableGravity(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else
        {
            // 바닥에 있을 때 설정
            MeshComponent->SetSimulatePhysics(true);
            MeshComponent->SetEnableGravity(true);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
            MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        }

        // 네트워크 복제 설정
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
    }
}

void AItem_Gun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Gun, bIsHeld);
    DOREPLIFETIME(AItem_Gun, bIsAiming);
    DOREPLIFETIME(AItem_Gun, CurrentAmmo);

}

void AItem_Gun::OnRep_HeldState()
{
    if (bIsHeld && GetOwner())
    {
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
        AttachToActor(GetOwner(), AttachRules);

        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            SetActorRelativeLocation(FVector(100.0f, 30.0f, 0.0f));
            SetActorRotation(Character->GetActorRotation());
        }
    }
    else
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        if (MeshComponent)
        {
            MeshComponent->SetSimulatePhysics(true);
        }
    }
}

void AItem_Gun::OnRep_AimState()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        if (UCameraComponent* Camera = Player->GetFollowCamera())
        {
            if (bIsAiming)
            {
                DefaultFOV = Camera->FieldOfView;
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }
                Camera->SetFieldOfView(AimFOV);
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

void AItem_Gun::PickUp_Implementation(ACharacter* Character)
{
    if (!Character) return;

    bIsHeld = true;
    SetOwner(Character);

    if (MeshComponent)
    {
        // 물리 및 충돌 비활성화
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 캐릭터에 부착
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToActor(Character, AttachRules);

    // 상대적 위치 설정
    SetActorRelativeLocation(FVector(30.0f, 10.0f, 0.0f));
    SetActorRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    // 네트워크 업데이트
    ForceNetUpdate();
}

void AItem_Gun::Drop_Implementation()
{
    if (!HasAuthority()) return;

    if (bIsAiming)
    {
        ToggleAim();
    }

    bIsHeld = false;
    OnRep_HeldState();
}

void AItem_Gun::Fire_Implementation()
{
    if (!HasAuthority() || CurrentAmmo <= 0) return;

    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        FVector Start = GetActorLocation();
        FVector Forward = Player->GetActorForwardVector();
        FVector End = Start + (Forward * 5000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(Player);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
        {
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);

            // 텐트 히트 체크 추가
            if (AItem_Tent* HitTent = Cast<AItem_Tent>(HitResult.GetActor()))
            {
                // 텐트에 데미지 전달
                HitTent->OnBulletHit();
            }
        }

        CurrentAmmo--;
    }
}
void AItem_Gun::ToggleAim_Implementation()
{
    if (!HasAuthority()) return;
    bIsAiming = !bIsAiming;
    OnRep_AimState();
}

void AItem_Gun::ThrowForward_Implementation()
{
    if (!GetOwner() || !HasAuthority()) return;

    // 1. 소유자로부터 분리
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 2. 물리 상태 초기화
    if (MeshComponent)
    {
        // 충돌 설정 활성화
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

        // 물리 시뮬레이션 활성화
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);

        // 물리 특성 설정
        MeshComponent->SetLinearDamping(0.5f);    // 공기 저항
        MeshComponent->SetAngularDamping(0.5f);   // 회전 저항

        // 네트워크 동기화 설정
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // 컴포넌트 업데이트
        MeshComponent->UpdateComponentToWorld();

        // 5. 임펄스 추가 (충돌 설정 후)
        if (GetOwner())
        {
            FVector ThrowDirection = GetOwner()->GetActorForwardVector();
            MeshComponent->AddImpulse(ThrowDirection * 500.0f, NAME_None, true);
        }
    }

    // 상태 업데이트
    bIsHeld = false;
    SetOwner(nullptr);

    // 네트워크 업데이트 강제
    ForceNetUpdate();
}