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
    CurrentHealth = MaxHealth;

    if (MeshComponent)
    {
        MeshComponent->SetIsReplicated(true);
    }
}

void AItem_Tent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;

    // 다이나믹 머티리얼 생성
    if (MeshComponent && MeshComponent->GetMaterial(0))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(0), this);
        MeshComponent->SetMaterial(0, DynamicMaterial);
    }
}

void AItem_Tent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItem_Tent, bIsBuiltTent);
    DOREPLIFETIME(AItem_Tent, DamageReduction);
    DOREPLIFETIME(AItem_Tent, bBlocksVision);
    DOREPLIFETIME(AItem_Tent, MaxHealth);
    DOREPLIFETIME(AItem_Tent, CurrentHealth);
}

void AItem_Tent::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;

    bIsBuiltTent = true;
    CurrentHealth = MaxHealth;

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

    CurrentHealth--;

    // 피격 효과
    MulticastPlayHitEffect();

    if (CurrentHealth <= 0)
    {
        // 바로 Destroy하지 않고 1초 후에 파괴
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AItem_Tent::DestroyTent, 1.0f, false);
    }

    ForceNetUpdate();
}

// 새로운 함수 추가
void AItem_Tent::DestroyTent()
{
    if (HasAuthority())
    {
        Destroy();
    }
}


void AItem_Tent::MulticastPlayHitEffect_Implementation()
{
    if (DynamicMaterial)
    {
        // 발광 효과 켜기 (GlowIntensity는 기본값 5.0f)
        DynamicMaterial->SetScalarParameterValue("EmissiveIntensity", GlowIntensity);

        // 0.2초 후에 발광 효과 끄기
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetScalarParameterValue("EmissiveIntensity", 0.0f);
                }
            }, GlowDuration, false);
    }
}

void AItem_Tent::OnRep_CurrentHealth()
{
    // 클라이언트에서 체력 변화 시 피격 효과 재생
    MulticastPlayHitEffect();
}