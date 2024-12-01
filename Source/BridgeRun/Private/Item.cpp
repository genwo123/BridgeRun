// Item.cpp
#include "Item.h"
#include "Components/BoxComponent.h"
#include "Citizen.h"
#include <ItemSpawnZone.h>

AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = true;
    // �޽� ������Ʈ ����
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    // �ݸ��� ������Ʈ ����
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(MeshComponent);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
    CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
    // �⺻�� ����
    Amount = 1;
    ItemType = EInventorySlot::None;
}

void AItem::BeginPlay()
{
    Super::BeginPlay();
    
}

void AItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Item.cpp�� OnOverlapBegin ����
void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // �Ǽ��� �������� ������ ����
    if (bIsBuiltItem)
    {
        return;
    }

    if (ACitizen* Citizen = Cast<ACitizen>(OtherActor))
    {
        // ������ ȹ��
        Citizen->AddItem(ItemType, Amount);
        Destroy();
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // �ʿ��� ��� ������ ���� ó��
}