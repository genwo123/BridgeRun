// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BridgeRunPlayerState.generated.h"

// BP���� ����� �̺�Ʈ ��������Ʈ��
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStatsChanged, class ABridgeRunPlayerState*, PlayerState, int32, StatType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStatsReset, class ABridgeRunPlayerState*, PlayerState);

/**
 * �긮���� ������ �÷��̾� ���� Ŭ����
 * �÷��̾� �� ���� �� ���ھ�� ��踦 �����մϴ�
 */
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    /** �⺻ ������ */
    ABridgeRunPlayerState();

    /** ��Ʈ��ũ ���� ���� */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================
    // ���� �� ���� (����)
    // =========================

    /** �� ID ���� */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);

    /** �� ID �������� */
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const;

    /** �÷��̾��� �� ID */
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    /** �� ID ���� �� ȣ��Ǵ� �Լ� */
    UFUNCTION()
    void OnRep_TeamID();

    // =========================
    // ���� �߰�: ���� ���� ��� (���� ��ü)
    // =========================

    /** ��ü ���� ���� ��ġ�� ���� �� */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalPlanksBuilt = 0;

    /** ��ü ���� ���� ��ġ�� ��Ʈ �� */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTentsBuilt = 0;

    /** ��ü ���� ���� �������� ���� ���� */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalTrophyScore = 0;

    /** ��ü ���� ���� ��/������ ���� Ƚ�� */
    UPROPERTY(ReplicatedUsing = OnRep_TotalStats, BlueprintReadOnly, Category = "Total Stats")
    int32 TotalHitCount = 0;

    // =========================
    // ���� �߰�: ���� ���� ��� (���庰 �ʱ�ȭ)
    // =========================

    /** ���� ���忡�� ��ġ�� ���� �� */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundPlanksBuilt = 0;

    /** ���� ���忡�� ��ġ�� ��Ʈ �� */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTentsBuilt = 0;

    /** ���� ���忡�� �������� ���� ���� */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundTrophyScore = 0;

    /** ���� ���忡�� ��/������ ���� Ƚ�� */
    UPROPERTY(ReplicatedUsing = OnRep_RoundStats, BlueprintReadOnly, Category = "Round Stats")
    int32 RoundHitCount = 0;

    // =========================
    // ��� ������Ʈ �Լ��� (Server RPC)
    // =========================

    /** ���� ��ġ �� ȣ�� - ���� �� ���� ��� �� �� ���� */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddPlankBuilt();

    /** ��Ʈ ��ġ �� ȣ�� - ���� �� ���� ��� �� �� ���� */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTentBuilt();

    /** ���� ���� ȹ�� �� ȣ�� - ���� �� ���� ��� �� �� ���� */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddTrophyScore(int32 TrophyPoints);

    /** ��/���� Ÿ�� �� ȣ�� - ���� �� ���� ��� �� �� ���� */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerAddHitCount();

    /** ���� ���� �� ȣ�� - ���� ��踸 �ʱ�ȭ */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
    void ServerResetRoundStats();

    // =========================
    // BP���� ���� ����� �� �ִ� Get �Լ���
    // =========================

    // ���� ��� Get �Լ���
    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalPlanksBuilt() const { return TotalPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTentsBuilt() const { return TotalTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalTrophyScore() const { return TotalTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Total Stats")
    int32 GetTotalHitCount() const { return TotalHitCount; }

    // ���� ��� Get �Լ���
    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundPlanksBuilt() const { return RoundPlanksBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTentsBuilt() const { return RoundTentsBuilt; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundTrophyScore() const { return RoundTrophyScore; }

    UFUNCTION(BlueprintPure, Category = "Round Stats")
    int32 GetRoundHitCount() const { return RoundHitCount; }

    // =========================
    // BP���� ���ε��� �� �ִ� �̺�Ʈ�� (�ǽð� UI ������Ʈ��)
    // =========================

    /** ��谡 ����� �� ��ε�ĳ��Ʈ�Ǵ� �̺�Ʈ */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerStatsChanged OnPlayerStatsChanged;

    /** ���� ��谡 �ʱ�ȭ�� �� ��ε�ĳ��Ʈ�Ǵ� �̺�Ʈ */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRoundStatsReset OnRoundStatsReset;

    /** �÷��̾� ǥ�� �̸� (�κ񿡼� �Է��� �г���) */
    UPROPERTY(ReplicatedUsing = OnRep_DisplayName, BlueprintReadOnly, Category = "Player Info")
    FString DisplayName;

    /** �÷��̾� ǥ�� �̸� ���� */
    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void SetDisplayName(const FString& NewDisplayName);

    /** �÷��̾� ǥ�� �̸� �������� */
    UFUNCTION(BlueprintPure, Category = "Player Info")
    FString GetDisplayName() const { return DisplayName; }

    /** ǥ�� �̸��� ��ȿ���� Ȯ�� */
    UFUNCTION(BlueprintPure, Category = "Player Info")
    bool IsDisplayNameValid() const;


protected:
    // =========================
    // ���� �Լ��� (�ǽð� ������Ʈ �̺�Ʈ �߻�)
    // =========================

    /** ���� ��� ���� �� ȣ�� */
    UFUNCTION()
    void OnRep_TotalStats();

    /** ���� ��� ���� �� ȣ�� */
    UFUNCTION()
    void OnRep_RoundStats();

    UFUNCTION()
    void OnRep_DisplayName();

private:
    // ��� Ÿ�� enum (�̺�Ʈ ���п�)
    enum class EStatType : int32
    {
        Planks = 0,
        Tents = 1,
        Trophy = 2,
        Hits = 3
    };
};