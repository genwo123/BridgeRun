// Private/Item/Item_Plank.cpp
#include "Item/Item_Plank.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

AItem_Plank::AItem_Plank()
{
    bReplicates = true;
    ItemType = EInventorySlot::Plank;
    bIsBuiltPlank = false;
    MaxPlankLength = 300.0f;
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;

    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
    }
}

// Item_Plank.cpp�� BeginPlay �Լ�
void AItem_Plank::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        // �ٸ� ���ڿʹ� �浹�ϵ��� ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

        // õ������ �浹�� �����ϵ��� ����
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // õ�� ���� ä��

        // ���� ����
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // ��Ʈ��ũ ������Ʈ ����
        ForceNetUpdate();
    }
}

void AItem_Plank::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;

    bIsBuiltPlank = true;

    if (MeshComponent)
    {
        // ���� �ùķ��̼� ������ ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // ���� ���·� ����
        MeshComponent->SetMobility(EComponentMobility::Stationary);

        // �浹 ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

        // ��ġ ������Ʈ ����
        MeshComponent->UpdateComponentToWorld();

        // ��� Ŭ���̾�Ʈ�� ����ȭ
        MulticastSetPlankPhysicsState(EComponentMobility::Stationary);
    }

    ForceNetUpdate();
}


void AItem_Plank::MulticastSetPlankPhysicsState_Implementation(EComponentMobility::Type NewMobility)
{
    // ���� üũ ���� - ��� Ŭ���̾�Ʈ���� ����
    if (MeshComponent)
    {
        // ���� �ùķ��̼� ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // ���� ���·� ����
        MeshComponent->SetMobility(NewMobility);

        // �浹 ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

        // ��ġ ������Ʈ ����
        MeshComponent->UpdateComponentToWorld();
    }
}

void AItem_Plank::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Plank, bIsBuiltPlank);
    DOREPLIFETIME(AItem_Plank, MaxPlankLength);
}

void AItem_Plank::OnRep_IsBuilt()
{
    if (bIsBuiltPlank)
    {
        if (MeshComponent)
        {
            MeshComponent->SetMobility(EComponentMobility::Movable);
            MeshComponent->SetSimulatePhysics(true);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }
    }
}
