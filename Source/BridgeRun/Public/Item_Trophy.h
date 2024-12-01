// Item_Trophy.h
#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Item_Trophy.generated.h"

UCLASS()
class BRIDGERUN_API AItem_Trophy : public AItem
{
    GENERATED_BODY()

public:
    AItem_Trophy();

    UPROPERTY()
    bool bIsHeld;

    void PickUp(class ACitizen* Player);
    void Drop();

    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult) override;

protected:
    virtual void BeginPlay() override;
};