// Item_Gun.cpp
#include "Item_Gun.h"
#include "Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"  // 추가된 헤더
#include <DrawDebugHelpers.h>

AItem_Gun::AItem_Gun()
{
    bIsHeld = false;
    ItemType = EInventorySlot::Gun;
    CurrentAmmo = MaxAmmo;

    // 생성자에서 바로 태그 초기화
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
    if (bIsAiming)  // 줌 상태라면
    {
        ToggleAim();  // 줌 해제
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

        // 디버그 라인 추가
        DrawDebugLine(
            GetWorld(),
            Start,
            End,
            FColor::Red,
            false,
            2.0f  // 2초동안 보여짐
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
                // 여기에 히트 효과 추가 (넉백, 데미지 등)
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
    // 자동 획득 방지를 위해 부모 클래스의 OnOverlapBegin은 호출하지 않음
}

void AItem_Gun::ToggleAim()
{
    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        if (UCameraComponent* Camera = Player->GetFollowCamera())
        {
            if (!bIsAiming)
            {
                // 기존 설정 저장
                DefaultFOV = Camera->FieldOfView;
                // SpringArm 길이를 0으로 하여 1인칭 전환
                if (USpringArmComponent* SpringArm = Player->FindComponentByClass<USpringArmComponent>())
                {
                    DefaultArmLength = SpringArm->TargetArmLength;
                    SpringArm->TargetArmLength = 0.0f;
                }
                // FOV 변경 (망원경보다 덜 확대)
                Camera->SetFieldOfView(AimFOV);
            }
            else
            {
                // 원래 설정으로 복구
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
        // 플레이어 전방으로 던지기
        FVector ThrowDirection = Player->GetActorForwardVector();
        FVector ThrowVelocity = ThrowDirection * 1000.0f + FVector(0, 0, 300.0f);

        Drop();  // 기존 Drop 호출

        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
        {
            PrimComp->AddImpulse(ThrowVelocity);
        }
    }
}

void AItem_Gun::SetGunTag(const FString& NewTag)
{
    GunTag = GetName();  // 액터의 실제 이름을 태그로 사용
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