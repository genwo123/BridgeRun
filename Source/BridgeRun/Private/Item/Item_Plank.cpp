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
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("OnPlaced called on non-authority!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnPlaced_Implementation called - setting bIsBuiltPlank to true"));

    // 상태 업데이트
    bIsBuiltPlank = true;

    if (MeshComponent)
    {
        // 판자를 설치된 상태로 변경
        ApplyBuiltPlankState();
        UE_LOG(LogTemp, Warning, TEXT("ApplyBuiltPlankState called in OnPlaced"));

        // 🆕 강제로 네트워크 업데이트
        ForceNetUpdate();

        // 🆕 모든 클라이언트에 상태 동기화
        MulticastSetPlankPhysicsState(EComponentMobility::Stationary);
    }
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
    UE_LOG(LogTemp, Error, TEXT("=== OnRep_IsBuilt CALLED: bIsBuiltPlank = %s ==="),
        bIsBuiltPlank ? TEXT("TRUE") : TEXT("FALSE"));

    if (bIsBuiltPlank)
    {
        UE_LOG(LogTemp, Error, TEXT("Applying built plank state from OnRep"));
        // 🆕 ApplyBuiltPlankState 대신 직접 설정
        if (MeshComponent)
        {
            // Mobility를 먼저 Movable로 설정한 후 물리 비활성화
            MeshComponent->SetMobility(EComponentMobility::Movable);
            MeshComponent->SetSimulatePhysics(false);
            MeshComponent->SetEnableGravity(false);

            // 그 다음에 Stationary로 변경
            MeshComponent->SetMobility(EComponentMobility::Stationary);

            // 충돌 설정
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
            MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

            UE_LOG(LogTemp, Log, TEXT("OnRep: Plank physics properly disabled and set to stationary"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Setting plank to physics mode from OnRep"));
        if (MeshComponent)
        {
            MeshComponent->SetMobility(EComponentMobility::Movable);
            MeshComponent->SetSimulatePhysics(true);
            MeshComponent->SetEnableGravity(true);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}