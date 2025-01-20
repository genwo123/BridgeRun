// Private/Item/Item_Gun.cpp
#include "Item/Item_Gun.h"
#include "Characters/Citizen.h"
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

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void AItem_Gun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Gun, CurrentAmmo);
    DOREPLIFETIME(AItem_Gun, bIsHeld);
    DOREPLIFETIME(AItem_Gun, bIsAiming);
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
    Super::PickUp_Implementation(Character);

    if (!Character || !HasAuthority()) return;

    bIsHeld = true;
    OnRep_HeldState();
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

            if (AActor* HitActor = HitResult.GetActor())
            {
                // 히트 처리 로직 추가 가능
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
    if (!HasAuthority()) return;

    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        FVector ThrowDirection = Player->GetActorForwardVector();
        FVector ThrowVelocity = ThrowDirection * 1000.0f + FVector(0, 0, 300.0f);

        Drop();

        if (MeshComponent)
        {
            MeshComponent->AddImpulse(ThrowVelocity);
        }
    }
}
