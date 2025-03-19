// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * ���� �ν��Ͻ� Ŭ���� - �� ���� ����
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();

    // �ʱ�ȭ
    virtual void Init() override;

    // �� ���� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void AddTeamScore(int32 TeamID, int32 ScoreToAdd);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetTeamScore(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetWinningTeam() const;

protected:
    // �� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // �� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumberOfTeams = 4;

    // ��ȿ�� �� ID Ȯ�� (���ο�)
    bool IsValidTeamID(int32 TeamID) const;
};