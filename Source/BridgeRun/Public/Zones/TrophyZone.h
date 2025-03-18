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
    // 생성자 및 기본 함수
    ATrophyZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    // 팀 관련 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 TeamID = 0;  // 기본값은 0 (첫 번째 팀)

    // 점수 관련 함수 및 속성
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay")
    int32 GetCurrentScore() const { return CurrentScore; }

    UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentScore, Category = "Gameplay")
    int32 CurrentScore;

    // 타이머 관련 함수
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void OnScoreTimerComplete();

    // 블루프린트 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
    void BP_ScoreUpdated(int32 InTeamID, int32 InNewScore);

protected:
    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TimerText;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* ScoreText;

    // 게임플레이 속성
    UPROPERTY(ReplicatedUsing = OnRep_PlacedTrophy)
    AItem_Trophy* PlacedTrophy;

    UPROPERTY(EditAnywhere, Category = "Gameplay")
    float ScoreTime;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    float RemainingTime;

    // 타이머 핸들
    FTimerHandle ScoreTimerHandle;
    FTimerHandle UpdateTimerHandle;

    // 복제 이벤트 함수
    UFUNCTION()
    void OnRep_RemainingTime();

    UFUNCTION()
    void OnRep_PlacedTrophy();

    UFUNCTION()
    void OnRep_CurrentScore();

    // 오버랩 이벤트
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

    // 업데이트 함수
    UFUNCTION()
    void UpdateTimer();

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void UpdateScoreText();

    // 서버 함수
    UFUNCTION(Server, Reliable)
    void ServerUpdateScore(int32 NewScore);

    UFUNCTION(Server, Reliable)
    void ServerHandleTrophyPlacement(AItem_Trophy* Trophy);

    UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    void MulticastOnScoreUpdated(int32 NewScore);

private:
    // 초기화 함수
    void InitializeComponents();
    void SetupTriggerBox();
    void SetupTextComponents();
    void UpdateTimerText();
};