// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * 게임 진행 단계 (로비 포함)
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
 * 팀 승점 정보
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
 * BridgeRun 게임 상태 - 로비부터 게임 종료까지 모든 상태 관리
 */
UCLASS(BlueprintType, Blueprintable)
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ABridgeRunGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================
    // 공통 게임 상태
    // =========================

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    EGamePhase CurrentPhase = EGamePhase::Lobby;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    float PhaseTimeRemaining = 30.0f;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRoundNumber = 1;

    // Phase 관리 함수들
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetPhaseTimeRemaining(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentRoundNumber(int32 NewRound);

    // Get 함수들
    UFUNCTION(BlueprintPure, Category = "Game Info")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetPhaseTimeRemaining() const { return PhaseTimeRemaining; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetCurrentRoundNumber() const { return CurrentRoundNumber; }

    // Phase 체크 함수들
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
    // 로비 시스템 (NEW!)
    // =========================

    /** 방 이름 */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
    FString RoomName = TEXT("BridgeRun Room");

    /** 최대 플레이어 수 */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayersCount = 12;

    /** 현재 플레이어 수 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
    int32 CurrentPlayersCount = 0;

    // 로비 관리 함수들
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

    // Ready 시스템 (PlayerState 기반이지만 전체 상황은 GameState에서 관리)
    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool AreAllNonHostPlayersReady() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetReadyPlayersCount() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 GetTotalNonHostPlayersCount() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool CanStartGame() const;

    // 모든 클라이언트 UI 업데이트
    UFUNCTION(NetMulticast, Reliable, Category = "Lobby")
    void MulticastUpdateLobbyUI();

    // =========================
    // 게임플레이 시스템 (기존)
    // =========================

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    TArray<FTeamVictoryData> TeamVictoryPoints;

    UPROPERTY(ReplicatedUsing = OnRep_GameStarted, BlueprintReadWrite, Category = "Game State")
    bool bGameStarted = false;

    // 팀 관리
    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void InitializeTeams(const TArray<int32>& ActiveTeamIDs);

    UFUNCTION(BlueprintCallable, Category = "Game Management")
    void StartGameWithTeams(const TArray<int32>& ActiveTeamIDs);

    // 팀 정보 함수들
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

    // 점수 관리
    UFUNCTION(BlueprintCallable, Category = "Team Score")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamCurrentScore(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamVictoryPoints(int32 TeamID) const;

    // 순위 시스템
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

    // 게임 로직
    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void CalculateRoundVictoryPoints();

    UFUNCTION(BlueprintPure, Category = "Game Logic")
    bool ShouldEndGame() const;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastGameOverUI();

    // =========================
    // UI 표시용 함수들
    // =========================

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetFormattedTime() const;

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetRoundText() const;

    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetPhaseText() const;

    // =========================
    // 이벤트 시스템
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