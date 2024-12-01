// TrophyZone.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item_Trophy.h"  // 이 줄을 추가!
#include "TrophyZone.generated.h"

class UBoxComponent;
class UTextRenderComponent;
class AItem_Trophy;

UCLASS()
class ATrophyZone : public AActor
{
    GENERATED_BODY()

public:
    ATrophyZone();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere)
    UTextRenderComponent* TimerText;

    UPROPERTY(VisibleAnywhere)
    UTextRenderComponent* ScoreText;

    UPROPERTY()
    AItem_Trophy* PlacedTrophy;

    UPROPERTY(EditAnywhere, Category = "Gameplay")
    float ScoreTime;

    UPROPERTY(EditAnywhere, Category = "Gameplay")
    int32 ScoreAmount = 100;

    float RemainingTime;
    int32 CurrentScore;

    FTimerHandle ScoreTimerHandle;
    FTimerHandle UpdateTimerHandle;

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void UpdateScoreText();


    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void UpdateTimer();
    void OnScoreTimerComplete();
};