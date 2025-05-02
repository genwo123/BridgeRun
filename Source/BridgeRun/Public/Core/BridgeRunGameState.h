// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * 토템 유형 열거형
 */
UENUM(BlueprintType)
enum class ETotemType : uint8
{
    Normal      UMETA(DisplayName = "일반 토템", ToolTip = "10점 가치의 기본 토템"),
    Gold        UMETA(DisplayName = "골드 토템", ToolTip = "20점 가치의 골드 토템"),
    Diamond     UMETA(DisplayName = "다이아몬드 토템", ToolTip = "30점 가치의 다이아몬드 토템")
};

/**
 * 토템존 유형 열거형
 */
UENUM(BlueprintType)
enum class ETotemZoneType : uint8
{
    Team        UMETA(DisplayName = "팀 토템존", ToolTip = "팀 본진 근처, 기본 점수"),
    Neutral     UMETA(DisplayName = "중립 토템존", ToolTip = "맵 중앙, 높은 점수")
};

/**
 * 팀 정보를 저장하는 기본 구조체
 */
USTRUCT(BlueprintType)
struct FBasicTeamInfo
{
    GENERATED_BODY()

    /** 팀 고유 식별자 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 TeamId = 0;

    /** 현재 라운드 팀 점수 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 Score = 0;

    /** 누적 팀 점수 (모든 라운드) */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 TotalScore = 0;

    /** 팀의 현재 라운드 등수 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 CurrentRank = 0;

    /** 팀이 획득한 1등 횟수 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 FirstPlaceCount = 0;

    FBasicTeamInfo() = default;
};

/**
 * 브리지런 게임의 전체 상태를 관리하는 클래스
 */
UCLASS()
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ABridgeRunGameState();

    /** 팀 점수 업데이트 (모든 클라이언트에 전파) */
    UFUNCTION(NetMulticast, Reliable, Category = "Team")
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);

    /** 현재 라운드 완료 및 다음 라운드 준비 */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void CompleteRound();

    /** 라운드 시작 */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void StartRound();

    /** 골든타임 활성화 (3라운드 마지막 1분) */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void ActivateGoldenTime();

    /** 토템 점수 획득 처리 */
    UFUNCTION(BlueprintCallable, Category = "Totem")
    void AddTotemScore(int32 TeamId, ETotemType TotemType, ETotemZoneType ZoneType, float TimeHeld);

    /** 라운드 종료 후 팀 등수 계산 */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void CalculateTeamRanks();

    /** 라운드 등수에 따른 점수 부여 */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void AssignRankPoints();

    /** 게임 종료 및 최종 승자 결정 */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void DetermineWinner();

    /** 모든 팀 정보 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<FBasicTeamInfo> Teams;

    /** 남은 경기 시간 (초) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float MatchTime;

    /** 현재 라운드 (1-3) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 CurrentRound = 0;

    /** 라운드 준비 단계 여부 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bIsInPreparationPhase = true;

    /** 골든타임 활성화 여부 (3라운드 마지막 1분) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bGoldenTimeActive = false;

    /** 라운드 목표 점수 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 RoundTargetScore = 40;

    /** 게임 종료 여부 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bGameOver = false;

    /** 승리한 팀 ID */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 WinnerTeamId = -1;

protected:
    /** 네트워크 복제 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 토템 기본 점수 값 */
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    TMap<ETotemType, int32> TotemValues;

    /** 토템존 배율 값 */
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    TMap<ETotemZoneType, float> ZoneMultipliers;

    /** 라운드 등수별 점수 */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TMap<int32, int32> RankPoints;

    /** 각 라운드별 타이머 길이 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float RoundTime = 240.0f; // 4분

    /** 라운드 준비 시간 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float PreparationTime = 30.0f; // 30초

    /** RPC 구현 함수들 */
    void UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore);
    void CompleteRound_Implementation();
    void StartRound_Implementation();
    void ActivateGoldenTime_Implementation();
};