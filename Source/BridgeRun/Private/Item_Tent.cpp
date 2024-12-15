// Item_Tent.cpp
#include "Item_Tent.h"

AItem_Tent::AItem_Tent()
{
    ItemType = EInventorySlot::Tent;
    bIsBuiltTent = false;
    DamageReduction = 0.5f;  // 50% ������ ����
    bBlocksVision = true;

    // �⺻ �޽ÿ� ��Ƽ������ �������Ʈ���� ����
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;
}

void AItem_Tent::OnPlaced()
{
    bIsBuiltTent = true;

    if (MeshComponent)
    {
        // ���� �ùķ��̼� ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // ���� ����
        MeshComponent->SetMobility(EComponentMobility::Static);
    }
}

void AItem_Tent::OnBulletHit()
{
    // �Ѿ� �¾��� ���� ó��
    // ������ ���� ������ ���߿� ����
}