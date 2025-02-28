// Public/Core/BridgeRunGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    WaitingToStart,
    InProgress,
    RoundEnd,
    GameOver
};

USTRUCT(BlueprintType)
struct BRIDGERUN_API FTeamInfo
{
    GENERATED_BODY()

    UPROPERTY()
    int32 TeamID;

    UPROPERTY()
    int32 Score;

    UPROPERTY()
    int32 PlayerCount;

    FTeamInfo()
        : TeamID(0)
        , Score(0)
        , PlayerCount(0)
    {}
};

UCLASS(minimalapi)
class ABridgeRunGameMode : public AGameModeBase
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

    // 점수 관리
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddTeamScore(int32 TeamID, int32 Score);

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetTeamScore(int32 TeamID) const;

protected:
    // 게임 설정
    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    int32 MaxTeams = 4;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    int32 MinPlayersToStart = 6;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    float RoundDuration = 300.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    float PostRoundDelay = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    float JobSystemActivationTime = 240.0f;

    // 스폰 위치
    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    TArray<FVector> PlayerStartLocations;

    // 게임 상태 변수
    UPROPERTY(Replicated)
    EGameState CurrentGameState;

    UPROPERTY(Replicated)
    int32 CurrentRound;

    UPROPERTY(Replicated)
    float RoundTimeRemaining;

    UPROPERTY(Replicated)
    bool bJobSystemActive;

    UPROPERTY(Replicated)
    TArray<FTeamInfo> TeamInfo;

private:
    // 타이머 핸들
    FTimerHandle GameTimerHandle;
    FTimerHandle RoundTimerHandle;
    FTimerHandle JobSystemTimerHandle;

    // 내부 함수
    void HandleRoundTimer();
    void HandleJobSystemActivation();
    void AssignPlayerToTeam(APlayerController* NewPlayer);
    int32 GetOptimalTeamForTeam() const;
    bool CanStartGame() const;
    void UpdateGameState();
};