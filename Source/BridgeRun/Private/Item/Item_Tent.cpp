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

// Item_Tent.cpp의 BeginPlay 함수
void AItem_Tent::BeginPlay()
{
    Super::BeginPlay();


    // DynamicMaterial 초기화 추가
    if (MeshComponent && MeshComponent->GetMaterial(0))
    {
        DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    }

    if (MeshComponent)
    {
        // 기본 충돌 설정 구성
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // 모든 채널에 대해 기본적으로 Block 응답
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);


        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap); // 판자 전용 채널


        // 물리 설정
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // 네트워크 업데이트 강제
        ForceNetUpdate();
    }
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
        // 물리 시뮬레이션 완전히 비활성화
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // 고정 상태로 설정
        MeshComponent->SetMobility(EComponentMobility::Stationary);

        // 충돌 설정
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);

        // 위치 업데이트 강제
        MeshComponent->UpdateComponentToWorld();

        // 모든 클라이언트에 동기화
        MulticastSetPhysicsState();
    }

    ForceNetUpdate();
}

void AItem_Tent::MulticastSetPhysicsState_Implementation()
{
    // 서버 체크 제거 - 모든 클라이언트에서 실행
    if (MeshComponent)
    {
        // 물리 시뮬레이션 비활성화
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // 고정 상태로 설정
        MeshComponent->SetMobility(EComponentMobility::Stationary);

        // 충돌 설정
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // 위치 업데이트 강제
        MeshComponent->UpdateComponentToWorld();
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