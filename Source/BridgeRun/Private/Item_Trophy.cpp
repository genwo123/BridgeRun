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

    // �޽� ������Ʈ �ݸ��� ����
    
    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    // �ڽ� �ݸ��� ������Ʈ ����
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
        CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // TrophyZone���� ������ ����
    }

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
    }

    // �÷��̾�� ����
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToActor(Player, AttachRules);
    SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}

void AItem_Trophy::Drop()
{
    bIsHeld = false;

    // ����� �� Ŭ���� ���� ���
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

    // TrophyZone���� ��ȣ�ۿ븸 üũ
    if (OtherActor->IsA(ATrophyZone::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trophy overlapping with TrophyZone"));
    }
}


void AItem_Trophy::BeginPlay()  // BeginPlay ����
{
    Super::BeginPlay();  // �θ� Ŭ������ BeginPlay ȣ��

    // �ʿ��� �ʱ�ȭ �ڵ尡 �ִٸ� ���⿡ �߰�
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetGenerateOverlapEvents(true);
    }
}