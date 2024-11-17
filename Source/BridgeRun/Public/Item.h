// Item.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InvenComponent.h" // EInventorySlot Á¤ÀÇ¿ë
#include "Item.generated.h"

UCLASS()
class BRIDGERUN_API AItem : public AActor
{
    GENERATED_BODY()

public:
    AItem();

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, Category = "Item")
    EInventorySlot ItemType;

    UPROPERTY(EditDefaultsOnly, Category = "Item")
    int32 Amount;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* CollisionComponent;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);
};