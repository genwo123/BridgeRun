// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * ���� ���� ������
 */
UENUM(BlueprintType)
enum class ETotemType : uint8
{
    Normal      UMETA(DisplayName = "�Ϲ� ����", ToolTip = "10�� ��ġ�� �⺻ ����"),
    Gold        UMETA(DisplayName = "��� ����", ToolTip = "20�� ��ġ�� ��� ����"),
    Diamond     UMETA(DisplayName = "���̾Ƹ�� ����", ToolTip = "30�� ��ġ�� ���̾Ƹ�� ����")
};

/**
 * ������ ���� ������
 */
UENUM(BlueprintType)
enum class ETotemZoneType : uint8
{
    Team        UMETA(DisplayName = "�� ������", ToolTip = "�� ���� ��ó, �⺻ ����"),
    Neutral     UMETA(DisplayName = "�߸� ������", ToolTip = "�� �߾�, ���� ����")
};

/**
 * �� ������ �����ϴ� �⺻ ����ü
 */
USTRUCT(BlueprintType)
struct FBasicTeamInfo
{
    GENERATED_BODY()

    /** �� ���� �ĺ��� */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 TeamId = 0;

    /** ���� ���� �� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 Score = 0;

    /** ���� �� ���� (��� ����) */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 TotalScore = 0;

    /** ���� ���� ���� ��� */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 CurrentRank = 0;

    /** ���� ȹ���� 1�� Ƚ�� */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 FirstPlaceCount = 0;

    FBasicTeamInfo() = default;
};

/**
 * �긮���� ������ ��ü ���¸� �����ϴ� Ŭ����
 */
UCLASS()
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** �⺻ ������ */
    ABridgeRunGameState();

    /** �� ���� ������Ʈ (��� Ŭ���̾�Ʈ�� ����) */
    UFUNCTION(NetMulticast, Reliable, Category = "Team")
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);

    /** ���� ���� �Ϸ� �� ���� ���� �غ� */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void CompleteRound();

    /** ���� ���� */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void StartRound();

    /** ���Ÿ�� Ȱ��ȭ (3���� ������ 1��) */
    UFUNCTION(NetMulticast, Reliable, Category = "Game")
    void ActivateGoldenTime();

    /** ���� ���� ȹ�� ó�� */
    UFUNCTION(BlueprintCallable, Category = "Totem")
    void AddTotemScore(int32 TeamId, ETotemType TotemType, ETotemZoneType ZoneType, float TimeHeld);

    /** ���� ���� �� �� ��� ��� */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void CalculateTeamRanks();

    /** ���� ����� ���� ���� �ο� */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void AssignRankPoints();

    /** ���� ���� �� ���� ���� ���� */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void DetermineWinner();

    /** ��� �� ���� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<FBasicTeamInfo> Teams;

    /** ���� ��� �ð� (��) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float MatchTime;

    /** ���� ���� (1-3) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 CurrentRound = 0;

    /** ���� �غ� �ܰ� ���� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bIsInPreparationPhase = true;

    /** ���Ÿ�� Ȱ��ȭ ���� (3���� ������ 1��) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bGoldenTimeActive = false;

    /** ���� ��ǥ ���� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 RoundTargetScore = 40;

    /** ���� ���� ���� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bGameOver = false;

    /** �¸��� �� ID */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 WinnerTeamId = -1;

protected:
    /** ��Ʈ��ũ ���� �Ӽ� ���� */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** ���� �⺻ ���� �� */
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    TMap<ETotemType, int32> TotemValues;

    /** ������ ���� �� */
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    TMap<ETotemZoneType, float> ZoneMultipliers;

    /** ���� ����� ���� */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TMap<int32, int32> RankPoints;

    /** �� ���庰 Ÿ�̸� ���� (��) */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float RoundTime = 240.0f; // 4��

    /** ���� �غ� �ð� (��) */
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float PreparationTime = 30.0f; // 30��

    /** RPC ���� �Լ��� */
    void UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore);
    void CompleteRound_Implementation();
    void StartRound_Implementation();
    void ActivateGoldenTime_Implementation();
};