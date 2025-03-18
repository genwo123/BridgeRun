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
        // 판자의 초기 충돌 및 물리 설정
        SetupCollisionSettings();

        // 물리 설정
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

        // 네트워크 업데이트 강제
        ForceNetUpdate();
    }
}

void AItem_Plank::SetupCollisionSettings()
{
    if (!MeshComponent) return;

    // 기본 충돌 설정
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // 천막과의 충돌만 무시하도록 설정
    MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // 천막 전용 채널
}

void AItem_Plank::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;

    // 상태 업데이트
    bIsBuiltPlank = true;

    if (MeshComponent)
    {
        // 판자를 설치된 상태로 변경
        ApplyBuiltPlankState();

        // 모든 클라이언트에 동기화
        MulticastSetPlankPhysicsState(EComponentMobility::Stationary);
    }

    ForceNetUpdate();
}

void AItem_Plank::ApplyBuiltPlankState()
{
    if (!MeshComponent) return;

    // 물리 시뮬레이션 완전히 비활성화
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetEnableGravity(false);

    // 고정 상태로 설정
    MeshComponent->SetMobility(EComponentMobility::Stationary);

    // 충돌 설정
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

    // 위치 업데이트 강제
    MeshComponent->UpdateComponentToWorld();
}

void AItem_Plank::MulticastSetPlankPhysicsState_Implementation(EComponentMobility::Type NewMobility)
{
    // 서버 체크 제거 - 모든 클라이언트에서 실행
    if (MeshComponent)
    {
        // 물리 시뮬레이션 비활성화
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);

        // 고정 상태로 설정
        MeshComponent->SetMobility(NewMobility);

        // 충돌 설정
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

        // 위치 업데이트 강제
        MeshComponent->UpdateComponentToWorld();
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