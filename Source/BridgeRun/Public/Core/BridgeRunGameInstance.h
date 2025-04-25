// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * �÷��̾� �� ���� ���� ����ü
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
 * ���� �ν��Ͻ� Ŭ���� - �� ���� �� �÷��̾� �� ���� ����
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

    // �÷��̾� �� ���� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void SavePlayerTeamInfo(const FString& InPlayerID, int32 InTeamID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetPlayerTeamID(const FString& InPlayerID) const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void ClearPlayersTeamInfo();

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void PrintPlayersTeamInfo() const;

    // �� ���� �� ���� ���� ��Ģ �Լ�
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    bool AreTeamsBalanced() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    bool HasMinimumTeams(int32 MinTeamCount = 2) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    TArray<int32> GetTeamPlayerCounts() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
    int32 GetActiveTeamsCount() const;

    // �� ���� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "4"))
    int32 MinimumTeamsRequired = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "0", ClampMax = "10"))
    int32 MaxPlayerDifferenceAllowed = 1;

protected:
    // �� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // �÷��̾� �� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<FPlayerTeamInfo> PlayersTeamInfo;

    // �� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumberOfTeams = 4;

    // ��ȿ�� �� ID Ȯ�� (���ο�)
    bool IsValidTeamID(int32 TeamID) const;
};