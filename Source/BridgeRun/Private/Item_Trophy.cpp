// Item_Trophy.cpp
#include "Item_Trophy.h"
#include "Citizen.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "TrophyZone.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AItem_Trophy::AItem_Trophy()
{
    bIsHeld = false;
    ItemType = EInventorySlot::Trophy;

    // 메시 컴포넌트 콜리전 설정
    
    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    // 박스 콜리전 컴포넌트 설정
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        CollisionComponent->SetGenerateOverlapEvents(true);
        CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    }
}

void AItem_Trophy::PickUp(ACitizen* Player)
{
    if (!Player) return;

    bIsHeld = true;

    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // TrophyZone과의 오버랩 유지
    }

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
    }

    // 플레이어에게 부착
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToActor(Player, AttachRules);
    SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}

void AItem_Trophy::Drop()
{
    bIsHeld = false;

    // 드롭할 때 클래스 정보 출력
    UE_LOG(LogTemp, Warning, TEXT("Trophy Dropped - Class: %s"), *GetClass()->GetName());

    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        UE_LOG(LogTemp, Warning, TEXT("Trophy CollisionComponent Enabled"));
    }

    if (MeshComponent)
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        MeshComponent->SetSimulatePhysics(true);
        UE_LOG(LogTemp, Warning, TEXT("Trophy Physics Enabled"));
    }
}

void AItem_Trophy::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor))
        return;

    // TrophyZone과의 상호작용만 체크
    if (OtherActor->IsA(ATrophyZone::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trophy overlapping with TrophyZone"));
    }
}


void AItem_Trophy::BeginPlay()  // BeginPlay 구현
{
    Super::BeginPlay();  // 부모 클래스의 BeginPlay 호출

    // 필요한 초기화 코드가 있다면 여기에 추가
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetGenerateOverlapEvents(true);
    }
}