// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

// 전방 선언
class UTeamManagerComponent;

/** 게임 상태를 나타내는 열거형 */
UENUM(BlueprintType)
enum class EGameState : uint8
{
    WaitingToStart,  // 게임 시작 대기 중
    InProgress,      // 게임 진행 중
    RoundEnd,        // 라운드 종료
    GameOver         // 게임 종료
};

/** 브리지 런 게임의 메인 게임모드 클래스 */
UCLASS(Blueprintable)
class BRIDGERUN_API ABridgeRunGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABridgeRunGameMode();
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 게임 진행 함수
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartGame();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndGame();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartNewRound();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndCurrentRound();

    // 팀 관리 컴포넌트 접근자
    UFUNCTION(BlueprintPure, Category = "Team")
    UTeamManagerComponent* GetTeamManager() const { return TeamManagerComponent; }

protected:
    // 팀 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTeamManagerComponent* TeamManagerComponent = nullptr;

    // 게임 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    int32 MinPlayersToStart = 6;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float RoundDuration = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float PostRoundDelay = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float JobSystemActivationTime = 240.0f;

    // 스폰 위치
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
    TArray<FVector> PlayerStartLocations;

    // 게임 상태 변수
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    EGameState CurrentGameState = EGameState::WaitingToStart;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 CurrentRound = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float RoundTimeRemaining = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bJobSystemActive = false;

private:
    // 타이머 핸들
    FTimerHandle GameTimerHandle;
    FTimerHandle RoundTimerHandle;
    FTimerHandle JobSystemTimerHandle;

    // 내부 유틸리티 함수
    void HandleRoundTimer();
    void HandleJobSystemActivation();
    bool CanStartGame() const;
    void UpdateGameState();

    // 타이머 헬퍼 함수
    void SetGameTimer(FTimerHandle& TimerHandle, void (ABridgeRunGameMode::* Function)(), float Delay, bool bLooping = false);
    void ClearGameTimer(FTimerHandle& TimerHandle);
};