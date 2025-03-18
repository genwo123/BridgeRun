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
    bAlwaysRelevant = true;  // �׻� ��Ʈ��ũ ���� Ȱ��ȭ

    // ������Ʈ �ʱ�ȭ
    InitializeComponents();

    // �⺻�� �ʱ�ȭ
    Amount = 1;
    ItemType = EInventorySlot::None;
    bIsPickedUp = false;
    bIsBuiltItem = false;
    OwningPlayer = nullptr;

    // ��Ʈ��ũ ������Ʈ �� ����
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;
}

void AItem::InitializeComponents()
{
    // Mesh Component ����
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetIsReplicated(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Collision Component ����
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(MeshComponent);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionComponent->SetIsReplicated(true);
    CollisionComponent->SetGenerateOverlapEvents(true);
}

void AItem::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        // ���� �� ��Ʈ��ũ ���� �߰�
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
    }

    // �ʱ� ���� ����
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
    // ���� �� �浹 ���� �ʱ�ȭ
    SetItemPhysicsState(!bIsPickedUp);
}

void AItem::PickUp_Implementation(ACharacter* Character)
{
    if (!HasAuthority()) return;
    if (!IsValid(Character)) return;
    if (bIsPickedUp || OwningPlayer) return;

    // ���� ������Ʈ
    bIsPickedUp = true;
    OwningPlayer = Character;

    // ��� Ŭ���̾�Ʈ�� �˸�
    MulticastOnPickedUp(Character);

    // ��Ʈ��ũ ���� ������Ʈ ����
    ForceNetUpdate();
}

void AItem::Drop_Implementation()
{
    if (!HasAuthority()) return;
    if (!bIsPickedUp || !OwningPlayer) return;

    // ���� ������Ʈ
    bIsPickedUp = false;
    OwningPlayer = nullptr;

    // ��� Ŭ���̾�Ʈ�� �˸�
    MulticastOnDropped();

    // ��Ʈ��ũ ���� ������Ʈ ����
    ForceNetUpdate();
}

void AItem::MulticastOnPickedUp_Implementation(ACharacter* NewOwner)
{
    if (!IsValid(NewOwner)) return;

    // �����ڿ��� ����
    AttachToPlayer(NewOwner);
}

void AItem::MulticastOnDropped_Implementation()
{
    // ���� ����
    DetachFromPlayer();
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
    }
    else
    {
        DetachFromPlayer();
    }
}

void AItem::UpdatePhysicsState(bool bEnablePhysics)
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(bEnablePhysics);

        // ���� ��Ȱ��ȭ �� �浹 ��� ����
        ECollisionEnabled::Type CollisionType = bEnablePhysics ?
            ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
        MeshComponent->SetCollisionEnabled(CollisionType);
    }
}

void AItem::UpdateCollisionState(bool bEnableCollision)
{
    if (CollisionComponent)
    {
        ECollisionEnabled::Type CollisionType = bEnableCollision ?
            ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
        CollisionComponent->SetCollisionEnabled(CollisionType);
    }
}

void AItem::SetItemPhysicsState(bool bEnablePhysics)
{
    // ���� ���� ������Ʈ
    UpdatePhysicsState(bEnablePhysics);

    // �浹 ���� ������Ʈ
    UpdateCollisionState(bEnablePhysics);
}

void AItem::AttachToPlayer(ACharacter* Player)
{
    if (!IsValid(Player)) return;

    // ���� �� �浹 ��Ȱ��ȭ
    SetItemPhysicsState(false);

    // Actor ��ü�� ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    AttachToActor(Player, AttachRules);

    // �����ۺ� Ŀ���� Transform ����
    FTransform ItemTransform = GetPickupTransform(Player);
    SetActorRelativeLocation(ItemTransform.GetLocation());
    SetActorRelativeRotation(ItemTransform.GetRotation());

    ForceNetUpdate();
}

void AItem::DetachFromPlayer()
{
    // ���� �� �浹 Ȱ��ȭ
    SetItemPhysicsState(true);

    // ���� �и�
    FDetachmentTransformRules DetachRules(
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        true
    );

    DetachFromActor(DetachRules);

    // ��Ʈ��ũ ���� ���� ������Ʈ
    ForceNetUpdate();
}

FTransform AItem::GetPickupTransform_Implementation(ACharacter* Player) const
{
    // �⺻ ��ġ/ȸ�� ��ȯ
    return FTransform(DefaultPickupRotation, DefaultPickupOffset);
}