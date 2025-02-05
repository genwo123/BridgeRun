// Private/Items/Item_Tent.cpp
#include "Item/Item_Tent.h"
#include "Net/UnrealNetwork.h"

AItem_Tent::AItem_Tent()
{
    bReplicates = true;
    ItemType = EInventorySlot::Tent;
    bIsBuiltTent = false;
    DamageReduction = 0.5f;
    bBlocksVision = true;
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;

    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
    }
}

void AItem_Tent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Tent, bIsBuiltTent);
    DOREPLIFETIME(AItem_Tent, DamageReduction);
    DOREPLIFETIME(AItem_Tent, bBlocksVision);
}

void AItem_Tent::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;

    bIsBuiltTent = true;

    if (MeshComponent)
    {
        MeshComponent->SetMobility(EComponentMobility::Stationary);
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        MulticastSetPhysicsState();
    }

    ForceNetUpdate();
}

void AItem_Tent::MulticastSetPhysicsState_Implementation()
{
    if (!IsNetMode(NM_DedicatedServer))
    {
        if (MeshComponent)
        {
            MeshComponent->SetMobility(EComponentMobility::Stationary);
            MeshComponent->SetSimulatePhysics(false);
            MeshComponent->SetEnableGravity(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }
    }
}

void AItem_Tent::OnBulletHit_Implementation()
{
    if (!HasAuthority()) return;
    // 데미지 처리 로직은 나중에 추가
}