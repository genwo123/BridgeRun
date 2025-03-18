// Private/Items/Item_Tent.cpp
#include "Item/Item_Tent.h"
#include "Net/UnrealNetwork.h"

AItem_Tent::AItem_Tent()
{
    bReplicates = true;
    ItemType = EInventorySlot::Tent;

    // �⺻ �Ӽ� �ʱ�ȭ
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

    // ��Ƽ���� �ʱ�ȭ
    InitializeMaterials();

    // �浹 ����
    SetupCollisionSettings();
}

void AItem_Tent::InitializeMaterials()
{
    // DynamicMaterial �ʱ�ȭ
    if (MeshComponent && MeshComponent->GetMaterial(0))
    {
        DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    }
}

void AItem_Tent::SetupCollisionSettings()
{
    if (!MeshComponent) return;

    // �⺻ �浹 ���� ����
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // ��� ä�ο� ���� �⺻������ Block ����
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // ���ڿʹ� ��ĥ �� �ֵ��� ����
    MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap); // ���� ���� ä��

    // ���� ����
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetEnableGravity(true);
    MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

    // ��Ʈ��ũ ������Ʈ ����
    ForceNetUpdate();
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

    // ���� ������Ʈ
    bIsBuiltTent = true;
    CurrentHealth = MaxHealth;

    if (MeshComponent)
    {
        // ��Ʈ�� ��ġ�� ���·� ����
        ApplyBuiltTentState();

        // ��� Ŭ���̾�Ʈ�� ����ȭ
        MulticastSetPhysicsState();
    }

    ForceNetUpdate();
}

void AItem_Tent::ApplyBuiltTentState()
{
    if (!MeshComponent) return;

    // ���� �ùķ��̼� ������ ��Ȱ��ȭ
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetEnableGravity(false);

    // ���� ���·� ����
    MeshComponent->SetMobility(EComponentMobility::Stationary);

    // �浹 ����
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);

    // ��ġ ������Ʈ ����
    MeshComponent->UpdateComponentToWorld();
}

void AItem_Tent::MulticastOnTentPlaced_Implementation()
{
    if (!HasAuthority() && MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetMobility(EComponentMobility::Stationary);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void AItem_Tent::MulticastSetPhysicsState_Implementation()
{
    // ���� üũ ���� - ��� Ŭ���̾�Ʈ���� ����
    if (MeshComponent)
    {
        // ���� �ùķ��̼� ��Ȱ��ȭ
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // ���� ���·� ����
        MeshComponent->SetMobility(EComponentMobility::Stationary);

        // �浹 ����
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // ��ġ ������Ʈ ����
        MeshComponent->UpdateComponentToWorld();
    }
}

void AItem_Tent::OnBulletHit_Implementation()
{
    if (!HasAuthority()) return;

    // ü�� ����
    CurrentHealth--;

    // �ǰ� ȿ��
    MulticastPlayHitEffect();

    // ü�� Ȯ��
    if (CurrentHealth <= 0)
    {
        // �ٷ� Destroy���� �ʰ� 1�� �Ŀ� �ı�
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AItem_Tent::DestroyTent, 1.0f, false);
    }

    ForceNetUpdate();
}

void AItem_Tent::DestroyTent()
{
    if (HasAuthority())
    {
        Destroy();
    }
}

void AItem_Tent::MulticastPlayHitEffect_Implementation()
{
    // �߱� ȿ�� ���
    PlayGlowEffect(GlowIntensity);
}

void AItem_Tent::PlayGlowEffect(float Intensity)
{
    if (!DynamicMaterial) return;

    // �߱� ȿ�� �ѱ�
    DynamicMaterial->SetScalarParameterValue("EmissiveIntensity", Intensity);

    // ���� �ð� �Ŀ� �߱� ȿ�� ����
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue("EmissiveIntensity", 0.0f);
            }
        }, GlowDuration, false);
}

void AItem_Tent::OnRep_CurrentHealth()
{
    // Ŭ���̾�Ʈ���� ü�� ��ȭ �� �ǰ� ȿ�� ���
    MulticastPlayHitEffect();
}