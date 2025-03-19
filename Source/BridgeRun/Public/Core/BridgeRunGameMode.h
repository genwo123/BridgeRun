// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

// ���� ����
class UTeamManagerComponent;

/** ���� ���¸� ��Ÿ���� ������ */
UENUM(BlueprintType)
enum class EGameState : uint8
{
    WaitingToStart,  // ���� ���� ��� ��
    InProgress,      // ���� ���� ��
    RoundEnd,        // ���� ����
    GameOver         // ���� ����
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

    // ���� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartGame();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndGame();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartNewRound();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndCurrentRound();

    // �� ���� ������Ʈ ������
    UFUNCTION(BlueprintPure, Category = "Team")
    UTeamManagerComponent* GetTeamManager() const { return TeamManagerComponent; }

protected:
    // �� ���� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTeamManagerComponent* TeamManagerComponent = nullptr;

    // ���� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    int32 MinPlayersToStart = 6;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float RoundDuration = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float PostRoundDelay = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float JobSystemActivationTime = 240.0f;

    // ���� ��ġ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
    TArray<FVector> PlayerStartLocations;

    // ���� ���� ����
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    EGameState CurrentGameState = EGameState::WaitingToStart;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 CurrentRound = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float RoundTimeRemaining = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bJobSystemActive = false;

private:
    // Ÿ�̸� �ڵ�
    FTimerHandle GameTimerHandle;
    FTimerHandle RoundTimerHandle;
    FTimerHandle JobSystemTimerHandle;

    // ���� ��ƿ��Ƽ �Լ�
    void HandleRoundTimer();
    void HandleJobSystemActivation();
    bool CanStartGame() const;
    void UpdateGameState();

    // Ÿ�̸� ���� �Լ�
    void SetGameTimer(FTimerHandle& TimerHandle, void (ABridgeRunGameMode::* Function)(), float Delay, bool bLooping = false);
    void ClearGameTimer(FTimerHandle& TimerHandle);
};