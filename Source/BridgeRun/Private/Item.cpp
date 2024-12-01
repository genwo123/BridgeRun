// Item.cpp
#include "Item.h"
#include "Components/BoxComponent.h"
#include "Citizen.h"
#include <ItemSpawnZone.h>

AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = true;
    // 메시 컴포넌트 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    // 콜리전 컴포넌트 설정
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(MeshComponent);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
    CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
    // 기본값 설정
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

// Item.cpp의 OnOverlapBegin 수정
void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // 건설된 아이템은 오버랩 무시
    if (bIsBuiltItem)
    {
        return;
    }

    if (ACitizen* Citizen = Cast<ACitizen>(OtherActor))
    {
        // 아이템 획득
        Citizen->AddItem(ItemType, Amount);
        Destroy();
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // 필요한 경우 오버랩 종료 처리
}