// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * 게임 진행 단계 열거형
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
 * 팀 승점 정보 구조체
 */
USTRUCT(BlueprintType)
struct FTeamVictoryData
{
    GENERATED_BODY()

    /** 팀 ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID = 0;

    /** 현재 라운드 점수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 CurrentRoundScore = 0;

    /** 각 라운드 결과 (순위) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    TArray<int32> RoundResults;

    /** 총 승점 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TotalVictoryPoints = 0;

    FTeamVictoryData() = default;
};

/**
 * 브리지런 게임의 전체 상태를 관리하는 클래스
 */
UCLASS(BlueprintType, Blueprintable)
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ABridgeRunGameState();

    /** 네트워크 복제 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // === 게임 상태 변수들 (블루프린트에서 편집 가능) ===

    /** 현재 게임 진행 단계 */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    EGamePhase CurrentPhase = EGamePhase::Lobby;

    /** 현재 단계의 남은 시간 (초) */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    float PhaseTimeRemaining = 30.0f;

    /** 현재 라운드 번호 (1, 2, 3) */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRoundNumber = 1;

    /** 팀 승점 정보 배열 */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Game State")
    TArray<FTeamVictoryData> TeamVictoryPoints;

    // === Setter 함수들 (서버에서만 호출) ===

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetPhaseTimeRemaining(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetCurrentRoundNumber(int32 NewRound);

    // === Getter 함수들 (BP에서 호출 가능) ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    float GetPhaseTimeRemaining() const { return PhaseTimeRemaining; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    int32 GetCurrentRoundNumber() const { return CurrentRoundNumber; }

    // === UI 표시용 함수들 ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetFormattedTime() const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetRoundText() const;

    UFUNCTION(BlueprintPure, Category = "Game Info")
    FString GetPhaseText() const;

    // === 게임 상태 체크 함수들 ===

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsStrategyTime() const { return CurrentPhase == EGamePhase::StrategyTime; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundPlaying() const { return CurrentPhase == EGamePhase::RoundPlaying; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsRoundEnd() const { return CurrentPhase == EGamePhase::RoundEnd; }

    UFUNCTION(BlueprintPure, Category = "Game Info")
    bool IsGameEnd() const { return CurrentPhase == EGamePhase::GameEnd; }

    // === 팀 점수 관련 함수들 ===

    UFUNCTION(BlueprintCallable, Category = "Team Score")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamCurrentScore(int32 TeamID) const;

    /** 특정 팀의 총 승점 반환 */
    UFUNCTION(BlueprintPure, Category = "Team Score")
    int32 GetTeamVictoryPoints(int32 TeamID) const;

    // === 순위 관련 함수들 ===

    /** 승점 순으로 정렬된 팀 순위 배열 반환 */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetTeamRankings() const;

    /** 특정 팀의 순위 반환 (1등부터 시작) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    int32 GetTeamRank(int32 TeamID) const;

    /** 게임이 동점인지 확인 */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    bool IsGameTied() const;

    /** 우승팀들의 ID 배열 반환 (동점 우승 고려) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    TArray<int32> GetWinningTeams() const;

    /** 순위 표시 텍스트 반환 ("1등", "2등 (공동)" 등) */
    UFUNCTION(BlueprintPure, Category = "Rankings")
    FString GetRankDisplayText(int32 TeamID) const;

    // === 게임 로직 함수들 ===

    /** 라운드 종료 시 승점 계산 및 부여 */
    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void CalculateRoundVictoryPoints();

    /** 게임 종료 조건 확인 (3라운드 완료) */
    UFUNCTION(BlueprintPure, Category = "Game Logic")
    bool ShouldEndGame() const;

    // === 팀 관리 함수들 ===

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

    // === 블루프린트에서 오버라이드 가능한 함수들 ===

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnPhaseChanged(EGamePhase NewPhase, EGamePhase OldPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnRoundChanged(int32 NewRound, int32 OldRound);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnTimeUpdated(float NewTime);
};