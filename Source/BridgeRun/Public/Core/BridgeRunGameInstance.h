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

    // �� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // �� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 NumberOfTeams = 4;

    // �α� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogTeamScores();
};