// Item_Plank.cpp
#include "Item_Plank.h"

AItem_Plank::AItem_Plank()
{
    ItemType = EInventorySlot::Plank;
    bIsBuiltPlank = false;
    MaxPlankLength = 300.0f;

    // 기본 메시와 머티리얼은 블루프린트에서 설정
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;
}

void AItem_Plank::OnPlaced()
{
    bIsBuiltPlank = true;

    if (MeshComponent)
    {
        // 물리 시뮬레이션 비활성화
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // 정적 설정
        MeshComponent->SetMobility(EComponentMobility::Static);
    }
}