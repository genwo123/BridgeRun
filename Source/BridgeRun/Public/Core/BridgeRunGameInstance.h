// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * �÷��̾� ���� ���� ����ü (�� ���� + �̸� ����)
 */
USTRUCT(BlueprintType)
struct BRIDGERUN_API FPlayerTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerID;           // �÷��̾� ���� ID (�ڵ� ����)

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerName;         // �÷��̾� �г���

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 TeamID = -1;          // �� ID

    FPlayerTeamInfo() {}

    // ������ (ID �ڵ� ����)
    FPlayerTeamInfo(const FString& InPlayerName, int32 InTeamID)
        : PlayerName(InPlayerName), TeamID(InTeamID)
    {
        // PlayerID �ڵ� ���� (Ÿ�ӽ����� + ����)
        PlayerID = FString::Printf(TEXT("Player_%lld_%d"),
            FDateTime::Now().GetTicks(),
            FMath::RandRange(1000, 9999));
    }

    // ���� ȣȯ�� ������
    FPlayerTeamInfo(const FString& InPlayerID, const FString& InPlayerName, int32 InTeamID)
        : PlayerID(InPlayerID), PlayerName(InPlayerName), TeamID(InTeamID)
    {
    }
};

/**
 * ���� �ν��Ͻ� Ŭ���� - �κ� + �� ���� ����
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();
    virtual void Init() override;

    // =========================
    // �κ� �ý��� ȣȯ�� (���� LobbyGameInstance ��ü)
    // =========================

    /** ���� �κ� �ý��� ȣȯ�� - �÷��̾� �̸� (FText) */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    FText PlayerNameText;

    /** ���� �κ� �ý��� ȣȯ�� - ��Ų �ε��� */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    int32 SkinIndex = 0;

    /** �̱��� ���� ȣȯ (���� �κ� �ڵ忡�� ���) */
    UFUNCTION(BlueprintCallable, Category = "Lobby Compatibility")
    static UBridgeRunGameInstance* GetInstance();

    // =========================
    // �÷��̾� ���� �Լ� (�ܼ�ȭ)
    // =========================

    /** ���� �÷��̾� �̸� ���� */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetCurrentPlayerName(const FString& NewPlayerName);

    /** ���� �÷��̾� �̸� �������� (FString) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetCurrentPlayerName() const;

    /** ���� �÷��̾� �̸� �������� (FText - �κ� ȣȯ��) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FText GetCurrentPlayerNameAsText() const;

    // =========================
    // �� ���� ���� �Լ�
    // =========================

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UpdateTeamScore(int32 TeamID, int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void AddTeamScore(int32 TeamID, int32 ScoreToAdd);

    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetTeamScore(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetWinningTeam() const;

    // =========================
    // �÷��̾� ���� ���� �Լ� (�ܼ�ȭ)
    // =========================

    /** �÷��̾� ���� ���� (�̸����� �ڵ� ID ����) */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    FString AddPlayerInfo(const FString& PlayerName, int32 TeamID);

    /** �÷��̾� ���� ���� (ID ���� ����) */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void SavePlayerTeamInfo(const FString& PlayerID, int32 TeamID, const FString& PlayerName = "");

    /** �÷��̾� ID�� �� ID �������� */
    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetPlayerTeamID(const FString& PlayerID) const;

    /** �÷��̾� ID�� �̸� �������� */
    UFUNCTION(BlueprintPure, Category = "Teams")
    FString GetPlayerNameByID(const FString& PlayerID) const;

    /** ��� �÷��̾� ���� �������� */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<FPlayerTeamInfo> GetAllPlayersInfo() const;

    /** Ư�� ���� �÷��̾�� �������� */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<FPlayerTeamInfo> GetPlayersByTeam(int32 TeamID) const;

    /** �÷��̾� ���� ��ü ���� */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void ClearPlayersTeamInfo();

    /** ����׿�: ��� �÷��̾� ���� ��� */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void PrintPlayersTeamInfo() const;




    // =========================
    // �� ���� ���� �Լ�
    // =========================

    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> StoredTeamCounts;




    UFUNCTION(BlueprintPure, Category = "Teams")
    bool AreTeamsBalanced() const;

    UFUNCTION(BlueprintPure, Category = "Teams")
    bool HasMinimumTeams(int32 MinTeamCount = 2) const;

    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<int32> GetTeamPlayerCounts() const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void SetTeamPlayerCounts(const TArray<int32>& NewTeamCounts);

    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetActiveTeamsCount() const;

    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bSkipLoginScreen = false;


    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bHasPlayerName = false;

    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bReturningFromGame = false;

protected:
    // �� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // �÷��̾� ���� �迭
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<FPlayerTeamInfo> PlayersTeamInfo;

    // ���� �÷��̾� �̸� (FString)
    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CurrentPlayerName;

    // �� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumberOfTeams = 4;

    // �� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 MinimumTeamsRequired = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 MaxPlayerDifferenceAllowed = 1;

    // ��ȿ�� �� ID Ȯ��
    bool IsValidTeamID(int32 TeamID) const;

private:
    // �̱��� ���� ����
    static TWeakObjectPtr<UBridgeRunGameInstance> Instance;
};