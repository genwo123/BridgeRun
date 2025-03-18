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

    // 초기 탄약 설정
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
                // 조준 시작 - 현재 설정 저장 후 새 설정 적용
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
                // 조준 종료 - 기본 설정 복원
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

    // 상태 업데이트
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

    // 조준 중이면 해제
    if (bIsAiming)
    {
        ToggleAim();
    }

    // 상태 업데이트
    bIsHeld = false;
    OnRep_HeldState();
}

void AItem_Gun::Fire_Implementation()
{
    if (!HasAuthority() || CurrentAmmo <= 0) return;

    if (ACitizen* Player = Cast<ACitizen>(GetOwner()))
    {
        // 발사 위치 및 방향 계산
        FVector Start = GetActorLocation();
        FVector Forward = Player->GetActorForwardVector();
        FVector End = Start + (Forward * 5000.0f);

        // 충돌 쿼리 설정
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(Player);

        // 라인 트레이스 수행
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

        // 히트 결과 처리
        if (bHit)
        {
            End = HitResult.ImpactPoint;
            ProcessFireHit(HitResult);
        }

        // 디버그 효과 표시
        ShowFireDebugEffects(Start, End, HitResult);

        // 탄약 감소
        UpdateAmmoCount();
    }
}

void AItem_Gun::ShowFireDebugEffects(const FVector& Start, const FVector& End, const FHitResult& HitResult)
{
    if (!bShowDebugLine) return;

    // 총알 궤적 표시
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

    // 히트 위치 표시 (히트가 있는 경우)
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
    // 텐트 히트 체크
    if (AItem_Tent* HitTent = Cast<AItem_Tent>(HitResult.GetActor()))
    {
        // 텐트에 데미지 전달
        HitTent->OnBulletHit();

        // 텐트 히트 디버그 메시지
        UE_LOG(LogTemp, Display, TEXT("Hit tent at location: %s"), *HitResult.ImpactPoint.ToString());
    }
}

void AItem_Gun::UpdateAmmoCount(bool bLogChange)
{
    CurrentAmmo--;

    // 탄약 변경 로그 (요청된 경우)
    if (bLogChange)
    {
        // 화면 메시지 표시
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                2.0f,
                FColor::Yellow,
                FString::Printf(TEXT("Ammo: %d / %d"), CurrentAmmo, MaxAmmo)
            );
        }

        // 로그 출력
        UE_LOG(LogTemp, Display, TEXT("Ammo remaining: %d / %d"), CurrentAmmo, MaxAmmo);
    }
}

void AItem_Gun::ToggleAim_Implementation()
{
    if (!HasAuthority()) return;

    // 조준 상태 토글
    bIsAiming = !bIsAiming;
    OnRep_AimState();
}

void AItem_Gun::ThrowForward_Implementation()
{
    if (!GetOwner() || !HasAuthority()) return;

    // 소유자로부터 분리
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 물리 상태 초기화
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
        MeshComponent->SetLinearDamping(LinearDamping);
        MeshComponent->SetAngularDamping(AngularDamping);

        // 네트워크 동기화 설정
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // 컴포넌트 업데이트
        MeshComponent->UpdateComponentToWorld();

        // 임펄스 추가 (충돌 설정 후)
        if (GetOwner())
        {
            FVector ThrowDirection = GetOwner()->GetActorForwardVector();
            MeshComponent->AddImpulse(ThrowDirection * ThrowForce, NAME_None, true);
        }
    }

    // 상태 업데이트
    bIsHeld = false;
    SetOwner(nullptr);

    // 네트워크 업데이트 강제
    ForceNetUpdate();
}