// Item_Plank.cpp
#include "Item_Plank.h"

AItem_Plank::AItem_Plank()
{
    ItemType = EInventorySlot::Plank;
    bIsBuiltPlank = false;
    MaxPlankLength = 300.0f;

    // �⺻ �޽ÿ� ��Ƽ������ �������Ʈ���� ����
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;
}

void AItem_Plank::OnPlaced()
{
    bIsBuiltPlank = true;

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