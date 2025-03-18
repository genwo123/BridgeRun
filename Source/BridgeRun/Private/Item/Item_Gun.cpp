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

    // �ʱ� ���� ����
    CurrentAmmo = MaxAmmo;

    if (MeshComponent)
    {
        if (bIsHeld)
        {
            // ��� ���� �� ����
            MeshComponent->SetSimulatePhysics(false);
            MeshComponent->SetEnableGravity(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else
        {
            // �ٴڿ� ���� �� ����
            MeshComponent->SetSimulatePhysics(true);
            MeshComponent->SetEnableGravity(true);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
            MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        }

        // ��Ʈ��ũ ���� ����
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
        // ���� �� �浹 ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // ĳ���Ϳ� ����
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToActor(Character, AttachRules);

    // ����� ��ġ ����
    SetActorRelativeLocation(FVector(30.0f, 10.0f, 0.0f));
    SetActorRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    // ��Ʈ��ũ ������Ʈ
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

            // ��Ʈ ��Ʈ üũ �߰�
            if (AItem_Tent* HitTent = Cast<AItem_Tent>(HitResult.GetActor()))
            {
                // ��Ʈ�� ������ ����
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

    // 1. �����ڷκ��� �и�
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 2. ���� ���� �ʱ�ȭ
    if (MeshComponent)
    {
        // �浹 ���� Ȱ��ȭ
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

        // ���� �ùķ��̼� Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);

        // ���� Ư�� ����
        MeshComponent->SetLinearDamping(0.5f);    // ���� ����
        MeshComponent->SetAngularDamping(0.5f);   // ȸ�� ����

        // ��Ʈ��ũ ����ȭ ����
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // ������Ʈ ������Ʈ
        MeshComponent->UpdateComponentToWorld();

        // 5. ���޽� �߰� (�浹 ���� ��)
        if (GetOwner())
        {
            FVector ThrowDirection = GetOwner()->GetActorForwardVector();
            MeshComponent->AddImpulse(ThrowDirection * 500.0f, NAME_None, true);
        }
    }

    // ���� ������Ʈ
    bIsHeld = false;
    SetOwner(nullptr);

    // ��Ʈ��ũ ������Ʈ ����
    ForceNetUpdate();
}