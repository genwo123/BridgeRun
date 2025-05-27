// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

// 전방 선언
class UTeamManagerComponent;

/**
 * 라운드별 설정 구조체
 */
USTRUCT(BlueprintType)
struct FRoundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
    float PlayTime = 180.0f;        // 라운드 플레이 시간 (초)

    // 생성자
    FRoundSettings()
    {
        PlayTime = 180.0f;
    }

    FRoundSettings(float InPlayTime)
        : PlayTime(InPlayTime)
    {
    }
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
    virtual void RestartPlayer(AController* NewPlayer) override;

    // === 라운드 시스템 함수들 ===

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartStrategyPhase();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartRoundPlaying();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void EndRound();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void EndGame();

    // === 라운드 정보 Get 함수들 ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetRoundPlayTime(int32 RoundNumber) const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetStrategyTime(int32 RoundNumber) const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetMaxRounds() const { return RoundSettingsArray.Num(); }

    // 팀 관리 컴포넌트 접근자 (기존 유지)
    UFUNCTION(BlueprintPure, Category = "Team")
    UTeamManagerComponent* GetTeamManager() const { return TeamManagerComponent; }

protected:
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void CalculateRoundRankings();

    // === 팀 관리 (기존 유지) ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTeamManagerComponent* TeamManagerComponent = nullptr;

    // === 라운드별 설정 배열 ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Round Settings", meta = (TitleProperty = "PlayTime"))
    TArray<FRoundSettings> RoundSettingsArray;

    // === 전략 시간 설정 ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategy Time")
    float FirstStrategyTime = 30.0f;    // 첫 라운드 전략 시간

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategy Time")
    float OtherStrategyTime = 10.0f;    // 2,3라운드 전략 시간

    // === 기타 시간 설정 ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Settings")
    float RoundEndWaitTime = 3.0f;      // 라운드 종료 후 대기 시간

    // === 기존 게임 설정 (일부 유지) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    int32 MinPlayersToStart = 6;

    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void InitializeActiveTeams();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowRoundEndResults();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideRoundEndResults();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnRoundEndUI(int32 RoundNumber);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void GameOverUI();


private:
    // 타이머 핸들
    FTimerHandle PhaseTimerHandle;

    // 내부 함수들
    void UpdatePhaseTimer();
    void OnPhaseTimeEnd();
    bool CanStartGame() const;

    // 타이머 헬퍼 함수 (기존 유지)
    void SetGameTimer(FTimerHandle& TimerHandle, void (ABridgeRunGameMode::* Function)(), float Delay, bool bLooping = false);
    void ClearGameTimer(FTimerHandle& TimerHandle);
};