// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

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

    /** �� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 Score = 0;

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

    /** ��� �� ���� */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<FBasicTeamInfo> Teams;

    /** ���� ��� �ð� (��) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float MatchTime;

protected:
    /** ��Ʈ��ũ ���� �Ӽ� ���� */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore);
};