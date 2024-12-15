// Item_Tent.cpp
#include "Item_Tent.h"

AItem_Tent::AItem_Tent()
{
    ItemType = EInventorySlot::Tent;
    bIsBuiltTent = false;
    DamageReduction = 0.5f;  // 50% 데미지 감소
    bBlocksVision = true;

    // 기본 메시와 머티리얼은 블루프린트에서 설정
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;
}

void AItem_Tent::OnPlaced()
{
    bIsBuiltTent = true;

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

void AItem_Tent::OnBulletHit()
{
    // 총알 맞았을 때의 처리
    // 데미지 감소 로직은 나중에 구현
}