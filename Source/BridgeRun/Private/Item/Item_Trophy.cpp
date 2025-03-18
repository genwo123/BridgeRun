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

    // 움직임 복제 설정
    SetReplicateMovement(true);
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

    // 물리와 콜리전 설정
    SetTrophyPhysics(false);
    UpdateTrophyCollision();
    UpdateTrophyVisibility(true);

    // 여기서 AttachTrophyToPlayer를 호출하지 않고 
    // 멀티캐스트로만 부착 처리
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
        // 물리 설정 전에 콜리전 프로필 설정
        MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // 그 다음 물리 활성화
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 모든 클라이언트에 충돌 상태 동기화
    MulticastOnTrophyDropped();

    // 네트워크 상태 업데이트 강제
    ForceNetUpdate();
}

// Item_Trophy.cpp
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

    // 물리 비활성화
    MeshComponent->SetSimulatePhysics(false);

    // 충돌 유지 설정
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 캐릭터 전체 액터에 부착 (메시 컴포넌트가 아닌 액터에 부착)
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepRelative,
        false
    );

    // 액터에 부착
    AttachToActor(Player, AttachRules);

    // 캐릭터 앞쪽에 트로피 위치 설정
    SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // 모든 네트워크에서 변경사항 적용 강제
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
        SetTrophyPhysics(false);
        UpdateTrophyCollision();
        UpdateTrophyVisibility(true);

        // 부착 방식 유지 (액터에 부착)
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepRelative,
            false
        );

        AttachToActor(OwningPlayer, AttachRules);

        // 클라이언트와 동일한 위치 사용
        SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
        SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
    else
    {
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

    // 액터 부착 방식 사용
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepRelative,
        false
    );

    // 액터에 부착
    AttachToActor(Player, AttachRules);

    // 클라이언트와 동일한 위치
    SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
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
            MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));  // 물리 프로필 명시적 설정
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetSimulatePhysics(true);

            UpdateTrophyState();
            ForceNetUpdate();

            // 디버그 표시 추가
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                FString::Printf(TEXT("Trophy Physics State: %d"), MeshComponent->IsSimulatingPhysics()));

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

    // 업데이트
    UpdateTrophyState();  // 이 함수는 충돌과 물리를 모두 처리함
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

