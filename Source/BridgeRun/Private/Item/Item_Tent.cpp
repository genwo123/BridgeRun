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
void AItem_Tent::OnBulletHit_Implementation()
{
    if (!HasAuthority()) return;
    // 데미지 감소 로직 추가 예정
}