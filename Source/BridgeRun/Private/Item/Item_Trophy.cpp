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

    // ��Ʈ��ũ ������Ʈ �� ����
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;

    // ���� �� �浹 ���� �ʱ�ȭ
    SetupTrophyPhysicsAndCollision();

    // ������ ���� ����
    SetReplicateMovement(true);
}

void AItem_Trophy::SetupTrophyPhysicsAndCollision()
{
    // �޽� ������Ʈ ����
    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        MeshComponent->SetVisibility(true);

        // ���� �ùķ��̼� ���� ����
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
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

    // ������ �ݸ��� ���� ������Ʈ
    SetTrophyPhysics(false);
    UpdateTrophyCollision();
    UpdateTrophyVisibility(true);

    // ��Ƽĳ��Ʈ�� ���� ó��
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
        // �浹 �������� �ٽ� ������� ����
        MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // ���� �ٽ� Ȱ��ȭ
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

    // Ŭ���̾�Ʈ������ ������ �浹 ���� ����
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // ���� ����
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
}

void AItem_Trophy::MulticastOnPickedUp_Implementation(ACharacter* Player)
{
    if (!IsValid(Player) || !MeshComponent) return;

    // ���� ������ ����
    FVector OriginalScale = GetActorScale3D();

    // ���� ��Ȱ��ȭ
    MeshComponent->SetSimulatePhysics(false);

    // �浹 ���� - ������ �����ϰ� �ٸ� �������� �浹�ϵ���
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // �߿�: Ʈ���Ǹ� �� ĳ���Ϳʹ� �浹���� �ʵ��� ����
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // ���� ��Ģ�� KeepRelative�� ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        false
    );

    // ��Ʈ ������Ʈ�� ����
    AttachToComponent(Player->GetRootComponent(), AttachRules);

    // Ʈ���� ��ġ ����
    SetActorRelativeLocation(FVector(200.0f, 0.0f, 50.0f));  // �������θ� �ణ ��ġ
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));  // ȸ�� ����

    // ���� ������ ����
    SetActorScale3D(OriginalScale);

    // ��Ʈ��ũ ������� ���� ����
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
        // �ֿ��� ����
        SetTrophyPhysics(false);
        UpdateTrophyCollision();
        UpdateTrophyVisibility(true);

        // ���� ����
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepRelative,
            false
        );

        AttachToActor(OwningPlayer, AttachRules);

        // ��ġ ����
        SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
        SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
    else
    {
        // �ٴڿ� �ִ� ����
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
        // ���� ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);

        // �浹 ���� - ���ΰ��� �浹�� ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }

    // ���� ��Ģ ����
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        false
    );

    // ��Ʈ ������Ʈ�� ����
    AttachToComponent(Player->GetRootComponent(), AttachRules);

    // ����� ��ġ ���� - �������θ� �ణ ��ġ
    SetActorRelativeLocation(FVector(200.0f, 0.0f, 50.0f));
    SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // ��Ʈ��ũ ������� ���� ����
    ForceNetUpdate();
}

void AItem_Trophy::ServerTryRespawn_Implementation(const FVector& RespawnLocation)
{
    if (!HasAuthority() || !MeshComponent) return;

    // �ʱ�ȭ
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // ��ġ ����
    SetActorLocation(RespawnLocation + FVector(0, 0, 500.0f));

    // ����/�浹 �缳��
    FTimerHandle StateTimer;
    GetWorld()->GetTimerManager().SetTimer(StateTimer, [this]()
        {
            if (!IsValid(this)) return;

            // ������� ���� ����
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

    // ���� ���� �ʱ�ȭ
    bIsPickedUp = false;
    OwningPlayer = nullptr;
    bIsTrophyActive = true;

    // ��ġ ���� (���� �߰�)
    SetActorLocation(NewLocation + FVector(0, 0, 200.0f));

    // ���� ������Ʈ
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