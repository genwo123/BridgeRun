// Copyright BridgeRun Game, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item_Trophy.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "TrophyZone.generated.h"

UCLASS()
class BRIDGERUN_API ATrophyZone : public AActor
{
    GENERATED_BODY()

public:
    ATrophyZone();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 TeamID = 0;  // 기본값은 0 (첫 번째 팀)


    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay")
    int32 GetCurrentScore() const { return CurrentScore; }

    UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentScore, Category = "Gameplay")
    int32 CurrentScore;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    
    // Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TimerText;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* ScoreText;

    // Gameplay Properties
    UPROPERTY(ReplicatedUsing = OnRep_PlacedTrophy)
    AItem_Trophy* PlacedTrophy;

    UPROPERTY(EditAnywhere, Category = "Gameplay")
    float ScoreTime;


    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)  // Replicated만 되어있던 걸 수정
        float RemainingTime;

    
    UFUNCTION()
    void OnRep_RemainingTime();

 

    // Timers
    FTimerHandle ScoreTimerHandle;
    FTimerHandle UpdateTimerHandle;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void UpdateScoreText();

    UFUNCTION()
    void OnRep_PlacedTrophy();

    UFUNCTION()
    void OnRep_CurrentScore();

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

    UFUNCTION()
    void UpdateTimer();

    UFUNCTION()
    void OnScoreTimerComplete();

    // Server Functions
    UFUNCTION(Server, Reliable)
    void ServerUpdateScore(int32 NewScore);

    UFUNCTION(Server, Reliable)
    void ServerHandleTrophyPlacement(AItem_Trophy* Trophy);

    UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    void MulticastOnScoreUpdated(int32 NewScore);
};