// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Item/Item.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;  // 항상 네트워크 복제 활성화

    // Mesh Component 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetIsReplicated(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);  // Mobility를 Movable로 설정
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Collision Component 설정
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(MeshComponent);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionComponent->SetIsReplicated(true);
    CollisionComponent->SetGenerateOverlapEvents(true);

    // 기본값 초기화
    Amount = 1;
    ItemType = EInventorySlot::None;
    bIsPickedUp = false;
    bIsBuiltItem = false;
    OwningPlayer = nullptr;

    // 네트워크 업데이트 빈도 설정
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;
}

void AItem::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        // 물리 시뮬레이션 설정
        MeshComponent->SetMobility(EComponentMobility::Movable);
        MeshComponent->SetSimulatePhysics(true);

        // 네트워크 물리 설정
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
        MeshComponent->SetIsReplicated(true);
    }

    SetupInitialState();
}

void AItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AItem, ItemType, COND_InitialOnly);
    DOREPLIFETIME(AItem, Amount);
    DOREPLIFETIME(AItem, bIsBuiltItem);
    DOREPLIFETIME_CONDITION(AItem, bIsPickedUp, COND_None);
    DOREPLIFETIME_CONDITION(AItem, OwningPlayer, COND_None);
}

void AItem::MulticastSetMobility_Implementation(EComponentMobility::Type NewMobility)
{
    if (MeshComponent)
    {
        MeshComponent->SetMobility(NewMobility);
    }
}


void AItem::SetupInitialState()
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(!bIsPickedUp);
    }

    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(
            bIsPickedUp ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics
        );
    }
}

void AItem::PickUp_Implementation(ACharacter* Character)
{
    if (!HasAuthority()) return;
    if (!IsValid(Character)) return;
    if (bIsPickedUp || OwningPlayer) return;

    bIsPickedUp = true;
    OwningPlayer = Character;

    // 물리/충돌 상태 업데이트
    UpdatePhysicsState(false);
    UpdateCollisionState(false);

    // 모든 클라이언트에 알림
    MulticastOnPickedUp(Character);

    // 네트워크 상태 업데이트 강제
    ForceNetUpdate();
}

void AItem::Drop_Implementation()
{
    if (!HasAuthority()) return;
    if (!bIsPickedUp || !OwningPlayer) return;

    bIsPickedUp = false;
    OwningPlayer = nullptr;

    // 물리/충돌 상태 업데이트
    UpdatePhysicsState(true);
    UpdateCollisionState(true);

    // 모든 클라이언트에 알림
    MulticastOnDropped();

    // 네트워크 상태 업데이트 강제
    ForceNetUpdate();
}

void AItem::MulticastOnPickedUp_Implementation(ACharacter* NewOwner)
{
    if (!IsValid(NewOwner)) return;

    // 소유자에게 부착
    AttachToPlayer(NewOwner);

    // 물리/충돌 상태 업데이트
    UpdatePhysicsState(false);
    UpdateCollisionState(false);
}

void AItem::MulticastOnDropped_Implementation()
{
    // 부착 해제
    DetachFromPlayer();

    // 물리/충돌 상태 업데이트
    UpdatePhysicsState(true);
    UpdateCollisionState(true);
}

void AItem::OnRep_IsPickedUp()
{
    UpdateItemState();
}

void AItem::OnRep_OwningPlayer()
{
    UpdateItemState();
}

void AItem::UpdateItemState()
{
    if (bIsPickedUp && IsValid(OwningPlayer))
    {
        AttachToPlayer(OwningPlayer);
        UpdatePhysicsState(false);
        UpdateCollisionState(false);
    }
    else
    {
        DetachFromPlayer();
        UpdatePhysicsState(true);
        UpdateCollisionState(true);
    }
}

void AItem::UpdatePhysicsState(bool bEnablePhysics)
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(bEnablePhysics);
    }
}

void AItem::UpdateCollisionState(bool bEnableCollision)
{
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(
            bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
        );
    }
}

void AItem::AttachToPlayer(ACharacter* Player)
{
    if (!IsValid(Player)) return;

    // 물리/충돌 비활성화
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Actor 전체를 부착
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    AttachToActor(Player, AttachRules);

    // 아이템별 커스텀 Transform 적용
    FTransform ItemTransform = GetPickupTransform(Player);
    SetActorRelativeLocation(ItemTransform.GetLocation());
    SetActorRelativeRotation(ItemTransform.GetRotation());

    ForceNetUpdate();
}

void AItem::DetachFromPlayer()
{
    // 물리/충돌 활성화
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    FDetachmentTransformRules DetachRules(
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        true
    );

    // Actor 전체를 분리
    DetachFromActor(DetachRules);

    // 네트워크 상태 강제 업데이트
    ForceNetUpdate();
}

FTransform AItem::GetPickupTransform_Implementation(ACharacter* Player) const
{
    // 기본 위치/회전 반환
    return FTransform(DefaultPickupRotation, DefaultPickupOffset);
}