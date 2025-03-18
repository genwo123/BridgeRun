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
#include "Kismet/GameplayStatics.h"

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

    // �ʱ� ź�� ����
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
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepWorld,
            true
        );

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
                // ���� ���� - ���� ���� ���� �� �� ���� ����
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
                // ���� ���� - �⺻ ���� ����
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

    // ���� ������Ʈ
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

    // ���� ���̸� ����
    if (bIsAiming)
    {
        ToggleAim();
    }

    // ���� ������Ʈ
    bIsHeld = false;
    OnRep_HeldState();
}

void AItem_Gun::Fire_Implementation()
{
    if (!HasAuthority() || CurrentAmmo <= 0) return;

    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        // �߻� ��ġ �� ���� ���
        FVector Start = GetActorLocation();
        FVector Forward = Player->GetActorForwardVector();
        FVector End = Start + (Forward * 5000.0f);

        // �浹 ���� ����
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(Player);

        // ���� Ʈ���̽� ����
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

        // ��Ʈ ��� ó��
        if (bHit)
        {
            End = HitResult.ImpactPoint;
            ProcessFireHit(HitResult);
        }

        // ����� ȿ�� ǥ��
        ShowFireDebugEffects(Start, End, HitResult);

        // ź�� ����
        UpdateAmmoCount();
    }
}

void AItem_Gun::ShowFireDebugEffects(const FVector& Start, const FVector& End, const FHitResult& HitResult)
{
    if (!bShowDebugLine) return;

    // �Ѿ� ���� ǥ��
    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        DebugLineColor,
        false,
        DebugLineDuration,
        0,
        1.0f
    );

    // ��Ʈ ��ġ ǥ�� (��Ʈ�� �ִ� ���)
    if (HitResult.bBlockingHit)
    {
        DrawDebugSphere(
            GetWorld(),
            HitResult.ImpactPoint,
            10.0f,
            8,
            FColor::Orange,
            false,
            DebugLineDuration
        );
    }
}

void AItem_Gun::ProcessFireHit(const FHitResult& HitResult)
{
    // ��Ʈ ��Ʈ üũ
    if (AItem_Tent* HitTent = Cast<AItem_Tent>(HitResult.GetActor()))
    {
        // ��Ʈ�� ������ ����
        HitTent->OnBulletHit();

        // ��Ʈ ��Ʈ ����� �޽���
        UE_LOG(LogTemp, Display, TEXT("Hit tent at location: %s"), *HitResult.ImpactPoint.ToString());
    }
}

void AItem_Gun::UpdateAmmoCount(bool bLogChange)
{
    CurrentAmmo--;

    // ź�� ���� �α� (��û�� ���)
    if (bLogChange)
    {
        // ȭ�� �޽��� ǥ��
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                2.0f,
                FColor::Yellow,
                FString::Printf(TEXT("Ammo: %d / %d"), CurrentAmmo, MaxAmmo)
            );
        }

        // �α� ���
        UE_LOG(LogTemp, Display, TEXT("Ammo remaining: %d / %d"), CurrentAmmo, MaxAmmo);
    }
}

void AItem_Gun::ToggleAim_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ���� ���
    bIsAiming = !bIsAiming;
    OnRep_AimState();
}

void AItem_Gun::ThrowForward_Implementation()
{
    if (!GetOwner() || !HasAuthority()) return;

    // �����ڷκ��� �и�
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // ���� ���� �ʱ�ȭ
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
        MeshComponent->SetLinearDamping(LinearDamping);
        MeshComponent->SetAngularDamping(AngularDamping);

        // ��Ʈ��ũ ����ȭ ����
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // ������Ʈ ������Ʈ
        MeshComponent->UpdateComponentToWorld();

        // ���޽� �߰� (�浹 ���� ��)
        if (GetOwner())
        {
            FVector ThrowDirection = GetOwner()->GetActorForwardVector();
            MeshComponent->AddImpulse(ThrowDirection * ThrowForce, NAME_None, true);
        }
    }

    // ���� ������Ʈ
    bIsHeld = false;
    SetOwner(nullptr);

    // ��Ʈ��ũ ������Ʈ ����
    ForceNetUpdate();
}