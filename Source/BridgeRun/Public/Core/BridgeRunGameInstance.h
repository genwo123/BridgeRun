// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * 게임 인스턴스 클래스 - 팀 점수 관리
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    UBridgeRunGameInstance();

    virtual void Init() override;

    // 팀 점수 관리 함수
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void AddTeamScore(int32 TeamID, int32 ScoreToAdd);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetTeamScore(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetWinningTeam() const;

    // 팀 점수 배열
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // 팀 수 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 NumberOfTeams = 4;

    // 로그 함수
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogTeamScores();
};