// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item_Trophy.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "TrophyZone.generated.h"

// 트로피존 타입 열거형 추가
UENUM(BlueprintType)
enum class ETrophyZoneType : uint8
{
    TeamBase UMETA(DisplayName = "Team Base"),    // 팀 베이스 (트로피존의 팀 ID로 점수 부여)
    Neutral  UMETA(DisplayName = "Neutral Zone")  // 중립 지역 (트로피의 팀 ID로 점수 부여)
};

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

    // 트로피존 타입 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    ETrophyZoneType ZoneType = ETrophyZoneType::TeamBase;

    // 점수 배율 시스템 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Gameplay|Multiplier")
    float ScoreMultiplier = 1.0f;  // 기본 배율은 1.0

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

    // 트로피 배치 이벤트 추가
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
    void BP_TrophyPlaced(AItem_Trophy* Trophy);

    // 배율 변경 이벤트 추가
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay|Multiplier")
    void BP_MultiplierChanged(float NewMultiplier);

    // 배율 설정 함수
    UFUNCTION(BlueprintCallable, Category = "Gameplay|Multiplier")
    void SetScoreMultiplier(float NewMultiplier);

protected:
    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TimerText;
    // ScoreText 제거됨 - UI로 대체

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

    // 배율 복제 이벤트 추가
    UFUNCTION()
    void OnRep_ScoreMultiplier();

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

    // 서버 함수
    UFUNCTION(Server, Reliable)
    void ServerUpdateScore(int32 NewScore);
    UFUNCTION(Server, Reliable)
    void ServerHandleTrophyPlacement(AItem_Trophy* Trophy);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    void MulticastOnScoreUpdated(int32 NewScore);

    // 배율 관련 서버 함수 추가
    UFUNCTION(Server, Reliable)
    void ServerSetScoreMultiplier(float NewMultiplier);
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnMultiplierChanged(float NewMultiplier);

private:
    // 초기화 함수
    void InitializeComponents();
    void SetupTriggerBox();
    void SetupTextComponents();
    void UpdateTimerText();
};