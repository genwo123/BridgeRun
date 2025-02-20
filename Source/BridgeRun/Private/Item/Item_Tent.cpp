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

    // ���̳��� ��Ƽ���� ����
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

    // �ǰ� ȿ��
    MulticastPlayHitEffect();

    if (CurrentHealth <= 0)
    {
        // �ٷ� Destroy���� �ʰ� 1�� �Ŀ� �ı�
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AItem_Tent::DestroyTent, 1.0f, false);
    }

    ForceNetUpdate();
}

// ���ο� �Լ� �߰�
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
        // �߱� ȿ�� �ѱ� (GlowIntensity�� �⺻�� 5.0f)
        DynamicMaterial->SetScalarParameterValue("EmissiveIntensity", GlowIntensity);

        // 0.2�� �Ŀ� �߱� ȿ�� ����
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
    // Ŭ���̾�Ʈ���� ü�� ��ȭ �� �ǰ� ȿ�� ���
    MulticastPlayHitEffect();
}