// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BridgeRunPlayerState.generated.h"

// BP에서 사용할 이벤트 델리게이트들
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStatsChanged, class ABridgeRunPlayerState*, PlayerState, int32, StatType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStatsReset, class ABridgeRunPlayerState*, PlayerState);

/**
 * 브리지런 게임의 플레이어 상태 클래스
 * 플레이어 팀 정보 및 스코어보드 통계를 관리합니다
 */
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ABridgeRunPlayerState();

    /** 네트워크 복제 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================
    // 기존 팀 정보 (유지)
    // =========================

    /** 팀 ID 설정 */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);

    /** 팀 ID 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const;

    /** 플레이어의 팀 ID */
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    /** 팀 ID 변경 시 호출되는 함수 */
    UFUNCTION()
    void OnRep_TeamID();

    // =========================
    // 새로 추가: 개인 누적 통계 (게임 전체)
    // =========================

    /** 전체 게임 동안 설치한 판자 수 */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalPlanksBuilt = 0;

    /** 전체 게임 동안 설치한 텐트 수 */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTentsBuilt = 0;

    /** 전체 게임 동안 토템으로 얻은 점수 */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTrophyScore = 0;

    /** 전체 게임 동안 적/토템을 맞춘 횟수 */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalHitCount = 0;

    // =========================
    // 새로 추가: 현재 라운드 통계 (라운드별 초기화)
    // =========================

    /** 현재 라운드에서 설치한 판자 수 */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundPlanksBuilt = 0;

    /** 현재 라운드에서 설치한 텐트 수 */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTentsBuilt = 0;

    /** 현재 라운드에서 토템으로 얻은 점수 */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTrophyScore = 0;

    /** 현재 라운드에서 적/토템을 맞춘 횟수 */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundHitCount = 0;

    // =========================
    // 통계 업데이트 함수들 (Server RPC)
    // =========================

    /** 판자 설치 시 호출 - 누적 및 라운드 통계 둘 다 증가 */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddPlankBuilt();

    /** 텐트 설치 시 호출 - 누적 및 라운드 통계 둘 다 증가 */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTentBuilt();

    /** 토템 점수 획득 시 호출 - 누적 및 라운드 통계 둘 다 증가 */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTrophyScore(int32 TrophyPoints);

    /** 적/토템 타격 시 호출 - 누적 및 라운드 통계 둘 다 증가 */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddHitCount();

    /** 라운드 시작 시 호출 - 라운드 통계만 초기화 */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerResetRoundStats();

    // =========================
    // BP에서 쉽게 사용할 수 있는 Get 함수들
    // =========================

    // 누적 통계 Get 함수들
    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalPlanksBuilt() const { return TotalPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTentsBuilt() const { return TotalTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTrophyScore() const { return TotalTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalHitCount() const { return TotalHitCount; }

    // 라운드 통계 Get 함수들
    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundPlanksBuilt() const { return RoundPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTentsBuilt() const { return RoundTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTrophyScore() const { return RoundTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundHitCount() const { return RoundHitCount; }

    // =========================
    // BP에서 바인딩할 수 있는 이벤트들 (실시간 UI 업데이트용)
    // =========================

    /** 통계가 변경될 때 브로드캐스트되는 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerStatsChanged OnPlayerStatsChanged;

    /** 라운드 통계가 초기화될 때 브로드캐스트되는 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRoundStatsReset OnRoundStatsReset;

    /** 플레이어 표시 이름 (로비에서 입력한 닉네임) */
    UPROPERTY(ReplicatedUsing = OnRep_DisplayName, BlueprintReadOnly, Category = "Player Info")
    FString DisplayName;

    /** 플레이어 표시 이름 설정 */
    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void SetDisplayName(const FString& NewDisplayName);

    /** 플레이어 표시 이름 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Player Info")
    FString GetDisplayName() const { return DisplayName; }

    /** 표시 이름이 유효한지 확인 */
    UFUNCTION(BlueprintPure, Category = "Player Info")
    bool IsDisplayNameValid() const;


protected:
    // =========================
    // 복제 함수들 (실시간 업데이트 이벤트 발생)
    // =========================

    /** 누적 통계 복제 시 호출 */
    UFUNCTION()
    void OnRep_TotalStats();

    /** 라운드 통계 복제 시 호출 */
    UFUNCTION()
    void OnRep_RoundStats();

    UFUNCTION()
    void OnRep_DisplayName();

private:
    // 통계 타입 enum (이벤트 구분용)
    enum class EStatType : int32
    {
        Planks = 0,
        Tents = 1,
        Trophy = 2,
        Hits = 3
    };
};