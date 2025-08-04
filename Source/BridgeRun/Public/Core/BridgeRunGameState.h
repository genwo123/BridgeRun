// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * ���� ���� �ܰ� (�κ� ����)
 */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Lobby           UMETA(DisplayName = "Lobby"),
    StrategyTime    UMETA(DisplayName = "Strategy Time"),
    RoundPlaying    UMETA(DisplayName = "Round Playing"),
    RoundEnd        UMETA(DisplayName = "Round End"),
    GameEnd         UMETA(DisplayName = "Game End")
};

/**
 * �� ���� ����
 */
USTRUCT(BlueprintType)
struct FTeamVictoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 CurrentRoundScore = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    TArray<int32> RoundResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TotalVictoryPoints = 0;

    FTeamVictoryData() = default;
};

/**
 * BridgeRun ���� ���� - �κ���� ���� ������� ��� ���� ����
 */
UCLASS(BlueprintType, Blueprintable)
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ABridgeRunGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================
    // ���� ���� ����
    // =========================

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    EGamePhase CurrentPhase = EGamePhase::Lobby;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    float PhaseTimeRemaining = 30.0f;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRoundNumber = 1;

    // Phase ���� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetPhaseTimeRemaining(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentRoundNumber(int32 NewRound);

    // Get �Լ���
    UFUNCTION(BlueprintPure, Category = "Game Info")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetPhaseTimeRemaining() const { return PhaseTimeRemaining; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetCurrentRoundNumber() const { return CurrentRoundNumber; }

    // Phase üũ �Լ���
    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsLobbyPhase() const { return CurrentPhase == EGamePhase::Lobby; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsStrategyTime() const { return CurrentPhase == EGamePhase::StrategyTime; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundPlaying() const { return CurrentPhase == EGamePhase::RoundPlaying; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundEnd() const { return CurrentPhase == EGamePhase::RoundEnd; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsGameEnd() const { return CurrentPhase == EGamePhase::GameEnd; }

    // =========================
    // �κ� �ý��� (NEW!)
    // =========================

    /** �� �̸� */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
    FString RoomName = TEXT("BridgeRun Room");

    /** �ִ� �÷��̾� �� */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayersCount = 12;

    /** ���� �÷��̾� �� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
    int32 CurrentPlayersCount = 0;

    // �κ� ���� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetRoomName(const FString& NewRoomName);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetMaxPlayersCount(int32 NewMaxPlayers);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void UpdateCurrentPlayersCount();

    UFUNCTION(BlueprintPure, Category = "Lobby")
    FString GetRoomName() const { return RoomName; }

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetMaxPlayersCount() const { return MaxPlayersCount; }

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetCurrentPlayersCount() const { return CurrentPlayersCount; }

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsRoomFull() const { return CurrentPlayersCount >= MaxPlayersCount; }

    // Ready �ý��� (PlayerState ��������� ��ü ��Ȳ�� GameState���� ����)
    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool AreAllNonHostPlayersReady() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetReadyPlayersCount() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetTotalNonHostPlayersCount() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool CanStartGame() const;

    // ��� Ŭ���̾�Ʈ UI ������Ʈ
    UFUNCTION(NetMulticast, Reliable, Category = "Lobby")
    void MulticastUpdateLobbyUI();

    // =========================
    // �����÷��� �ý��� (����)
    // =========================

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    TArray<FTeamVictoryData> TeamVictoryPoints;

    UPROPERTY(ReplicatedUsing = OnRep_GameStarted, BlueprintReadWrite, Category = "Game State")
    bool bGameStarted = false;

    // �� ����
    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void InitializeTeams(const TArray<int32>& ActiveTeamIDs);

    UFUNCTION(BlueprintCallable, Category = "Game Management")
    void StartGameWithTeams(const TArray<int32>& ActiveTeamIDs);

    // �� ���� �Լ���
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

    // ���� ����
    UFUNCTION(BlueprintCallable, Category = "Team Score")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamCurrentScore(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamVictoryPoints(int32 TeamID) const;

    // ���� �ý���
    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetTeamRankings() const;

    UFUNCTION(BlueprintPure, Category = "Rankings")
    int32 GetTeamRank(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Rankings")
    bool IsGameTied() const;

    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetWinningTeams() const;

    UFUNCTION(BlueprintPure, Category = "Rankings")
    FString GetRankDisplayText(int32 TeamID) const;

    // ���� ����
    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void CalculateRoundVictoryPoints();

    UFUNCTION(BlueprintPure, Category = "Game Logic")
    bool ShouldEndGame() const;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastGameOverUI();

    // =========================
    // UI ǥ�ÿ� �Լ���
    // =========================

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetFormattedTime() const;

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetRoundText() const;

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetPhaseText() const;

    // =========================
    // �̺�Ʈ �ý���
    // =========================

    UFUNCTION()
    void OnRep_GameStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI Events")
    void BP_CreateTeamScoreWidgets();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowGameOverUIEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnPhaseChanged(EGamePhase NewPhase, EGamePhase OldPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnRoundChanged(int32 NewRound, int32 OldRound);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnTimeUpdated(float NewTime);
};