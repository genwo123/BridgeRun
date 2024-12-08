// Item_Gun.cpp
#include "Item_Gun.h"
#include "Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"  // �߰��� ���
#include <DrawDebugHelpers.h>

AItem_Gun::AItem_Gun()
{
    bIsHeld = false;
    ItemType = EInventorySlot::Gun;
    CurrentAmmo = MaxAmmo;

    // �����ڿ��� �ٷ� �±� �ʱ�ȭ
    GunTag = FString::Printf(TEXT("Gun_%d"), GetUniqueID());
    UE_LOG(LogTemp, Warning, TEXT("Gun Created with ammo: %d, Tag: %s"), CurrentAmmo, *GunTag);
}

void AItem_Gun::PickUp(ACitizen* Player)
{
    if (!Player) return;

    UE_LOG(LogTemp, Warning, TEXT("PickUp - Gun [%s] Before attach, Tag: [%s], Ammo: %d"),
        *GetName(), *GunTag, CurrentAmmo);

    bIsHeld = true;

    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        true);

    AttachToActor(Player, AttachRules);
    SetActorRelativeLocation(FVector(100.0f, 30.0f, 0.0f));
    SetActorRotation(Player->GetActorRotation());

    UE_LOG(LogTemp, Warning, TEXT("PickUp - Gun [%s] After attach, Tag: [%s], Ammo: %d"),
        *GetName(), *GunTag, CurrentAmmo);
}


void AItem_Gun::Drop()
{
    if (bIsAiming)  // �� ���¶��
    {
        ToggleAim();  // �� ����
    }

    bIsHeld = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        PrimComp->SetSimulatePhysics(true);
    }
}
void AItem_Gun::Fire()
{
    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No ammo left!"));
        return;
    }
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        FVector Start = GetActorLocation();
        FVector Forward = Player->GetActorForwardVector();
        FVector End = Start + (Forward * 5000.0f);

        // ����� ���� �߰�
        DrawDebugLine(
            GetWorld(),
            Start,
            End,
            FColor::Red,
            false,
            2.0f  // 2�ʵ��� ������
        );

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(Player);
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s at location: %s"),
                    *HitActor->GetName(),
                    *HitResult.Location.ToString());
                // ���⿡ ��Ʈ ȿ�� �߰� (�˹�, ������ ��)
            }
        }
        CurrentAmmo--;
        UE_LOG(LogTemp, Warning, TEXT("Ammo left: %d"), CurrentAmmo);
    }
}

void AItem_Gun::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // �ڵ� ȹ�� ������ ���� �θ� Ŭ������ OnOverlapBegin�� ȣ������ ����
}

void AItem_Gun::ToggleAim()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        if (UCameraComponent* Camera = Player->GetFollowCamera())
        {
            if (!bIsAiming)
            {
                // ���� ���� ����
                DefaultFOV = Camera->FieldOfView;
                // SpringArm ���̸� 0���� �Ͽ� 1��Ī ��ȯ
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }
                // FOV ���� (�����溸�� �� Ȯ��)
                Camera->SetFieldOfView(AimFOV);
            }
            else
            {
                // ���� �������� ����
                Camera->SetFieldOfView(DefaultFOV);
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    SpringArm->TargetArmLength = DefaultArmLength;
                }
            }
            bIsAiming = !bIsAiming;
        }
    }
}


void AItem_Gun::ThrowForward()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        // �÷��̾� �������� ������
        FVector ThrowDirection = Player->GetActorForwardVector();
        FVector ThrowVelocity = ThrowDirection * 1000.0f + FVector(0, 0, 300.0f);

        Drop();  // ���� Drop ȣ��

        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
        {
            PrimComp->AddImpulse(ThrowVelocity);
        }
    }
}

void AItem_Gun::SetGunTag(const FString& NewTag)
{
    GunTag = GetName();  // ������ ���� �̸��� �±׷� ���
    UE_LOG(LogTemp, Warning, TEXT("Gun [%s] - SetGunTag: Using actual name as tag, CurrentAmmo: %d"),
        *GunTag, CurrentAmmo);
}

FString AItem_Gun::GetGunTag() const
{
    if (GunTag.IsEmpty())
    {
        const_cast<AItem_Gun*>(this)->GunTag = GetName();
        UE_LOG(LogTemp, Warning, TEXT("Gun [%s] - GetGunTag: Using actual name as tag"), *GunTag);
    }
    return GunTag;
}