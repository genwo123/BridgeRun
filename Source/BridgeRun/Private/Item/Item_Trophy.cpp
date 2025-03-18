// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Item/Item_Trophy.h"
#include "Characters/Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AItem_Trophy::AItem_Trophy()
{
    ItemType = EInventorySlot::Trophy;
    bReplicates = true;
    bIsTrophyActive = true;

    // 기본 오프셋 및 회전 설정
    PickupOffset = FVector(100.0f, 0.0f, 50.0f);
    PickupRotation = FRotator(0.0f, 0.0f, 0.0f);

    // 네트워크 업데이트 빈도 증가
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;

    // 물리 및 충돌 설정 초기화
    SetupTrophyPhysicsAndCollision();

    // 움직임 복제 설정
    SetReplicateMovement(true);
}

void AItem_Trophy::SetupTrophyPhysicsAndCollision()
{
    // 메시 컴포넌트 설정
    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        MeshComponent->SetVisibility(true);

        // 물리 시뮬레이션 권한 설정
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
    }

    // 콜리전 컴포넌트 설정
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        CollisionComponent->SetGenerateOverlapEvents(true);
        CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    }
}

void AItem_Trophy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItem_Trophy, bIsTrophyActive);
}

void AItem_Trophy::BeginPlay()
{
    Super::BeginPlay();
    UpdateTrophyState();
}

void AItem_Trophy::PickUp_Implementation(ACharacter* Character)
{
    if (!HasAuthority() || !Character) return;

    Super::PickUp_Implementation(Character);

    // 물리와 콜리전 설정 업데이트
    SetTrophyPhysics(false);
    UpdateTrophyCollision();
    UpdateTrophyVisibility(true);

    // 멀티캐스트로 부착 처리
    MulticastOnPickedUp(Character);

    // 네트워크 동기화를 위한 상태 업데이트
    ForceNetUpdate();
}

void AItem_Trophy::Drop_Implementation()
{
    Super::Drop_Implementation();

    if (!HasAuthority()) return;

    if (MeshComponent)
    {
        // 충돌 프로필을 다시 원래대로 설정
        MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // 물리 다시 활성화
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    MulticastOnTrophyDropped();
    ForceNetUpdate();
}

void AItem_Trophy::MulticastOnTrophyDropped_Implementation()
{
    if (!MeshComponent) return;

    // 클라이언트에서도 동일한 충돌 설정 적용
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // 물리 설정
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
}

void AItem_Trophy::MulticastOnPickedUp_Implementation(ACharacter* Player)
{
    if (!IsValid(Player) || !MeshComponent) return;

    // 현재 스케일 저장
    FVector OriginalScale = GetActorScale3D();

    // 물리 비활성화
    MeshComponent->SetSimulatePhysics(false);

    // 충돌 설정 - 주인은 제외하고 다른 사람들과는 충돌하도록
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // 중요: 트로피를 든 캐릭터와는 충돌하지 않도록 설정
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // 부착 규칙을 KeepRelative로 변경
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        false
    );

    // 루트 컴포넌트에 부착
    AttachToComponent(Player->GetRootComponent(), AttachRules);

    // 트로피 위치 조정
    SetActorRelativeLocation(FVector(200.0f, 0.0f, 50.0f));  // 앞쪽으로만 약간 배치
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));  // 회전 제거

    // 원래 스케일 복원
    SetActorScale3D(OriginalScale);

    // 네트워크 변경사항 적용 강제
    ForceNetUpdate();
}

void AItem_Trophy::OnRep_TrophyState()
{
    UpdateTrophyVisibility(bIsTrophyActive);
}

void AItem_Trophy::UpdateTrophyState()
{
    if (bIsPickedUp && IsValid(OwningPlayer))
    {
        // 주우진 상태
        SetTrophyPhysics(false);
        UpdateTrophyCollision();
        UpdateTrophyVisibility(true);

        // 부착 설정
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepRelative,
            false
        );

        AttachToActor(OwningPlayer, AttachRules);

        // 위치 설정
        SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
        SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
    else
    {
        // 바닥에 있는 상태
        DetachTrophyFromPlayer();
        SetTrophyPhysics(true);
        UpdateTrophyCollision();
        UpdateTrophyVisibility(true);
    }
}

void AItem_Trophy::UpdateTrophyVisibility(bool bIsVisible)
{
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(bIsVisible, true);
    }
}

void AItem_Trophy::SetTrophyPhysics(bool bEnablePhysics)
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(bEnablePhysics);
    }
}

void AItem_Trophy::UpdateTrophyCollision()
{
    if (CollisionComponent)
    {
        ECollisionEnabled::Type CollisionType = bIsPickedUp ?
            ECollisionEnabled::QueryOnly : ECollisionEnabled::QueryAndPhysics;

        CollisionComponent->SetCollisionEnabled(CollisionType);
        CollisionComponent->SetGenerateOverlapEvents(true);
    }
}

void AItem_Trophy::AttachTrophyToPlayer(ACharacter* Player)
{
    if (!IsValid(Player)) return;

    if (MeshComponent)
    {
        // 물리 비활성화
        MeshComponent->SetSimulatePhysics(false);

        // 충돌 설정 - 주인과의 충돌만 무시
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }

    // 부착 규칙 변경
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        false
    );

    // 루트 컴포넌트에 부착
    AttachToComponent(Player->GetRootComponent(), AttachRules);

    // 상대적 위치 설정 - 앞쪽으로만 약간 배치
    SetActorRelativeLocation(FVector(200.0f, 0.0f, 50.0f));
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // 네트워크 변경사항 적용 강제
    ForceNetUpdate();
}

void AItem_Trophy::ServerTryRespawn_Implementation(const FVector& RespawnLocation)
{
    if (!HasAuthority() || !MeshComponent) return;

    // 초기화
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 위치 설정
    SetActorLocation(RespawnLocation + FVector(0, 0, 500.0f));

    // 물리/충돌 재설정
    FTimerHandle StateTimer;
    GetWorld()->GetTimerManager().SetTimer(StateTimer, [this]()
        {
            if (!IsValid(this)) return;

            // 순서대로 상태 설정
            MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetSimulatePhysics(true);

            UpdateTrophyState();
            ForceNetUpdate();
        }, 0.1f, false);
}

void AItem_Trophy::MulticastHandleRespawn_Implementation(const FVector& NewLocation)
{
    if (!MeshComponent) return;

    // 기존 상태 초기화
    bIsPickedUp = false;
    OwningPlayer = nullptr;
    bIsTrophyActive = true;

    // 위치 설정 (높이 추가)
    SetActorLocation(NewLocation + FVector(0, 0, 200.0f));

    // 상태 업데이트
    UpdateTrophyState();
}

void AItem_Trophy::DetachTrophyFromPlayer()
{
    if (!MeshComponent) return;

    FDetachmentTransformRules DetachRules(
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        true
    );

    MeshComponent->DetachFromComponent(DetachRules);
}

FTransform AItem_Trophy::GetPickupTransform_Implementation(ACharacter* Player) const
{
    return FTransform(FRotator::ZeroRotator, FVector(100.0f, 0.0f, 50.0f));
}