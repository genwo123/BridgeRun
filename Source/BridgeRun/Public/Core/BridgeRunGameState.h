// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * ���� ���� �ܰ� ������
 */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Lobby           UMETA(DisplayName = "Lobby"),
    StrategyTime    UMETA(DisplayName = "Strategy time"),
    RoundPlaying    UMETA(DisplayName = "Round progress"),
    RoundEnd        UMETA(DisplayName = "Round ends"),
    GameEnd         UMETA(DisplayName = "Game over")
};

/**
 * �� ���� ���� ����ü
 */
USTRUCT(BlueprintType)
struct FTeamVictoryData
{
    GENERATED_BODY()

    /** �� ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID = 0;

    /** ���� ���� ���� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 CurrentRoundScore = 0;

    /** �� ���� ��� (����) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    TArray<int32> RoundResults;

    /** �� ���� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TotalVictoryPoints = 0;

    FTeamVictoryData() = default;
};

/**
 * �긮���� ������ ��ü ���¸� �����ϴ� Ŭ����
 */
UCLASS(BlueprintType, Blueprintable)
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** �⺻ ������ */
    ABridgeRunGameState();

    /** ��Ʈ��ũ ���� �Ӽ� ���� */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // === ���� ���� ������ (�������Ʈ���� ���� ����) ===

    /** ���� ���� ���� �ܰ� */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    EGamePhase CurrentPhase = EGamePhase::Lobby;

    /** ���� �ܰ��� ���� �ð� (��) */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    float PhaseTimeRemaining = 30.0f;

    /** ���� ���� ��ȣ (1, 2, 3) */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRoundNumber = 1;

    /** �� ���� ���� �迭 */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    TArray<FTeamVictoryData> TeamVictoryPoints;

    // === Setter �Լ��� (���������� ȣ��) ===

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetPhaseTimeRemaining(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentRoundNumber(int32 NewRound);

    // === Getter �Լ��� (BP���� ȣ�� ����) ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetPhaseTimeRemaining() const { return PhaseTimeRemaining; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetCurrentRoundNumber() const { return CurrentRoundNumber; }

    // === UI ǥ�ÿ� �Լ��� ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetFormattedTime() const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetRoundText() const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetPhaseText() const;

    // === ���� ���� üũ �Լ��� ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsStrategyTime() const { return CurrentPhase == EGamePhase::StrategyTime; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundPlaying() const { return CurrentPhase == EGamePhase::RoundPlaying; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundEnd() const { return CurrentPhase == EGamePhase::RoundEnd; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsGameEnd() const { return CurrentPhase == EGamePhase::GameEnd; }

    // === �� ���� ���� �Լ��� ===

    UFUNCTION(BlueprintCallable, Category = "Team Score")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamCurrentScore(int32 TeamID) const;

    /** Ư�� ���� �� ���� ��ȯ */
    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamVictoryPoints(int32 TeamID) const;

    // === ���� ���� �Լ��� ===

    /** ���� ������ ���ĵ� �� ���� �迭 ��ȯ */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetTeamRankings() const;

    /** Ư�� ���� ���� ��ȯ (1����� ����) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    int32 GetTeamRank(int32 TeamID) const;

    /** ������ �������� Ȯ�� */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    bool IsGameTied() const;

    /** ��������� ID �迭 ��ȯ (���� ��� ���) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetWinningTeams() const;

    /** ���� ǥ�� �ؽ�Ʈ ��ȯ ("1��", "2�� (����)" ��) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    FString GetRankDisplayText(int32 TeamID) const;

    // === ���� ���� �Լ��� ===

    /** ���� ���� �� ���� ��� �� �ο� */
    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void CalculateRoundVictoryPoints();

    /** ���� ���� ���� Ȯ�� (3���� �Ϸ�) */
    UFUNCTION(BlueprintPure, Category = "Game Logic")
    bool ShouldEndGame() const;

    // === �� ���� �Լ��� ===

    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void InitializeTeams(const TArray<int32>& ActiveTeamIDs);

    UFUNCTION(BlueprintPure, Category = "Team Info")
    FString GetTeamName(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team Info")
    FSlateColor GetTeamColor(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team Info")
    bool IsTeamActive(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team Info")
    int32 GetActiveTeamCount() const;

    UFUNCTION(BlueprintPure, Category = "Team Info")
    TArray<int32> GetActiveTeamIDs() const;

    // === �������Ʈ���� �������̵� ������ �Լ��� ===

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnPhaseChanged(EGamePhase NewPhase, EGamePhase OldPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnRoundChanged(int32 NewRound, int32 OldRound);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnTimeUpdated(float NewTime);
};