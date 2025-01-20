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

    // �⺻ ������ �� ȸ�� ����
    PickupOffset = FVector(100.0f, 0.0f, 50.0f);
    PickupRotation = FRotator(0.0f, 0.0f, 0.0f);

    // �޽� ������Ʈ ����
    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetVisibility(true);
    }

    // �ݸ��� ������Ʈ ����
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

    // ������ �ݸ��� ����
    SetTrophyPhysics(false);
    UpdateTrophyCollision();
    UpdateTrophyVisibility(true);

    // ��� Ŭ���̾�Ʈ�� ������ ���� ��ġ ����
    MulticastOnPickedUp(Character);

    // ��Ʈ��ũ ����ȭ�� ���� ���� ������Ʈ
    ForceNetUpdate();
}



void AItem_Trophy::Drop_Implementation()
{
    Super::Drop_Implementation();

    if (!HasAuthority()) return;

    if (MeshComponent)
    {
        // ���� �浹 ������ ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        // �� ���� ���� Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(true);
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // ��Ʈ��ũ ���� ������Ʈ ����
    ForceNetUpdate();
}

void AItem_Trophy::MulticastOnPickedUp_Implementation(ACharacter* Player)
{
    if (!IsValid(Player)) return;

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // ��ġ ���� ����
    SetActorRelativeLocation(FVector(150.0f, 0.0f, 100.0f));

    // �� ���� ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepRelative,  // SnapToTarget ��� KeepRelative ���
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepWorld,
        false
    );

    AttachToActor(Player, AttachRules);
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
        AttachTrophyToPlayer(OwningPlayer);
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
    if (!IsValid(Player) || !MeshComponent) return;

    // ���� ��Ģ ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    // �޽� ����
    MeshComponent->AttachToComponent(
        Player->GetMesh(),
        AttachRules
    );

    // ����� ��ġ�� ȸ�� ����
    MeshComponent->SetRelativeLocation(PickupOffset);
    MeshComponent->SetRelativeRotation(PickupRotation);
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