// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * 플레이어 팀 정보 저장 구조체
 */
USTRUCT(BlueprintType)
struct BRIDGERUN_API FPlayerTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    FString PlayerID;

    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    int32 TeamID = -1;

    FPlayerTeamInfo() {}

    FPlayerTeamInfo(const FString& InPlayerID, int32 InTeamID)
        : PlayerID(InPlayerID), TeamID(InTeamID)
    {
    }
};

/**
 * 게임 인스턴스 클래스 - 팀 점수 및 플레이어 팀 정보 관리
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();

    // 초기화
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

    // 플레이어 팀 정보 관리 함수
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetPlayerTeamID(const FString& InPlayerID) const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void ClearPlayersTeamInfo();

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void PrintPlayersTeamInfo() const;

    // 팀 균형 및 게임 시작 규칙 함수
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    bool AreTeamsBalanced() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    bool HasMinimumTeams(int32 MinTeamCount = 2) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    TArray<int32> GetTeamPlayerCounts() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetActiveTeamsCount() const;

    // 팀 균형 관련 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "4"))
    int32 MinimumTeamsRequired = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "0", ClampMax = "10"))
    int32 MaxPlayerDifferenceAllowed = 1;

protected:
    // 팀 점수 배열
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // 플레이어 팀 정보 배열
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<FPlayerTeamInfo> PlayersTeamInfo;

    // 팀 수 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumberOfTeams = 4;

    // 유효한 팀 ID 확인 (내부용)
    bool IsValidTeamID(int32 TeamID) const;
};