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

void AItem_Plank::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

void AItem_Plank::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;

    bIsBuiltPlank = true;

    if (MeshComponent)
    {
        // 서버에서 설정
        MeshComponent->SetMobility(EComponentMobility::Movable);
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // 모든 클라이언트에 전파
        MulticastSetMobility(EComponentMobility::Movable);
    }

    // 네트워크 상태 강제 업데이트
    ForceNetUpdate();
}