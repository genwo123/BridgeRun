// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BridgeRunPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStatsChanged, class ABridgeRunPlayerState*, PlayerState, int32, StatType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStatsReset, class ABridgeRunPlayerState*, PlayerState);

/**
 * �긮���� �÷��̾� ���� - �� ����, �г���, Ready ����, ��� ����
 */
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ABridgeRunPlayerState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================
    // �� �ý���
    // =========================

    UPROPERTY(ReplicatedUsing = OnRep_TeamID, BlueprintReadOnly, Category = "Team")
    int32 TeamID = -1;

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);

    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const { return TeamID; }

    UFUNCTION()
    void OnRep_TeamID();

    // =========================
    // �κ� �ý���
    // =========================
    virtual void BeginDestroy() override;

    /** �÷��̾� �г���  */
    UPROPERTY(ReplicatedUsing = OnRep_PlayerNickname, BlueprintReadOnly, Category = "Lobby")
    FString PlayerNickname;

    /** Ready ���� */
    UPROPERTY(ReplicatedUsing = OnRep_ReadyStatus, BlueprintReadOnly, Category = "Lobby")
    bool bReadyStatus = false;

    /** ���� ���� */
    UPROPERTY(ReplicatedUsing = OnRep_HostStatus, BlueprintReadOnly, Category = "Lobby")
    bool bHostStatus = false;

    // �г��� ����
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetPlayerNickname(const FString& NewNickname);

    UFUNCTION(Server, Reliable, Category = "Lobby")
    void ServerSetPlayerNickname(const FString& NewNickname);

    UFUNCTION(BlueprintPure, Category = "Lobby")
    FString GetPlayerNickname() const { return PlayerNickname; }

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsNicknameValid() const;

    // Ready ���� ����
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetReadyStatus(bool bNewReady);

    UFUNCTION(Server, Reliable, Category = "Lobby")
    void ServerSetReadyStatus(bool bNewReady);

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool GetReadyStatus() const { return bReadyStatus; }

    // ���� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetHostStatus(bool bNewHost);

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool GetHostStatus() const { return bHostStatus; }

    // ���� �ݹ��
    UFUNCTION()
    void OnRep_PlayerNickname();

    UFUNCTION()
    void OnRep_ReadyStatus();

    UFUNCTION()
    void OnRep_HostStatus();

    // =========================
    // ���� ��� �ý���
    // =========================

    // ���� ���
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalPlanksBuilt = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTentsBuilt = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTrophyScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalHitCount = 0;

    // ���� ���
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundPlanksBuilt = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTentsBuilt = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTrophyScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundHitCount = 0;

    // ��� ������Ʈ �Լ���
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddPlankBuilt();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTentBuilt();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTrophyScore(int32 TrophyPoints);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddHitCount();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerResetRoundStats();

    // Get �Լ���
    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalPlanksBuilt() const { return TotalPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTentsBuilt() const { return TotalTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTrophyScore() const { return TotalTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalHitCount() const { return TotalHitCount; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundPlanksBuilt() const { return RoundPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTentsBuilt() const { return RoundTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTrophyScore() const { return RoundTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundHitCount() const { return RoundHitCount; }

    // =========================
    // �̺�Ʈ �ý���
    // =========================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerStatsChanged OnPlayerStatsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRoundStatsReset OnRoundStatsReset;

protected:
    UFUNCTION()
    void OnRep_TotalStats();

    UFUNCTION()
    void OnRep_RoundStats();

private:
    enum class EStatType : int32
    {
        Planks = 0,
        Tents = 1,
        Trophy = 2,
        Hits = 3
    };
};