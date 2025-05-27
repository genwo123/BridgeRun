// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

// ���� ����
class UTeamManagerComponent;

/**
 * ���庰 ���� ����ü
 */
USTRUCT(BlueprintType)
struct FRoundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
    float PlayTime = 180.0f;        // ���� �÷��� �ð� (��)

    // ������
    FRoundSettings()
    {
        PlayTime = 180.0f;
    }

    FRoundSettings(float InPlayTime)
        : PlayTime(InPlayTime)
    {
    }
};

/** �긮�� �� ������ ���� ���Ӹ�� Ŭ���� */
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

    // === ���� �ý��� �Լ��� ===

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartStrategyPhase();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartRoundPlaying();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void EndRound();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void EndGame();

    // === ���� ���� Get �Լ��� ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetRoundPlayTime(int32 RoundNumber) const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetStrategyTime(int32 RoundNumber) const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetMaxRounds() const { return RoundSettingsArray.Num(); }

    // �� ���� ������Ʈ ������ (���� ����)
    UFUNCTION(BlueprintPure, Category = "Team")
    UTeamManagerComponent* GetTeamManager() const { return TeamManagerComponent; }

protected:
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void CalculateRoundRankings();

    // === �� ���� (���� ����) ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTeamManagerComponent* TeamManagerComponent = nullptr;

    // === ���庰 ���� �迭 ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Round Settings", meta = (TitleProperty = "PlayTime"))
    TArray<FRoundSettings> RoundSettingsArray;

    // === ���� �ð� ���� ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategy Time")
    float FirstStrategyTime = 30.0f;    // ù ���� ���� �ð�

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategy Time")
    float OtherStrategyTime = 10.0f;    // 2,3���� ���� �ð�

    // === ��Ÿ �ð� ���� ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Settings")
    float RoundEndWaitTime = 3.0f;      // ���� ���� �� ��� �ð�

    // === ���� ���� ���� (�Ϻ� ����) ===
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
    // Ÿ�̸� �ڵ�
    FTimerHandle PhaseTimerHandle;

    // ���� �Լ���
    void UpdatePhaseTimer();
    void OnPhaseTimeEnd();
    bool CanStartGame() const;

    // Ÿ�̸� ���� �Լ� (���� ����)
    void SetGameTimer(FTimerHandle& TimerHandle, void (ABridgeRunGameMode::* Function)(), float Delay, bool bLooping = false);
    void ClearGameTimer(FTimerHandle& TimerHandle);
};