// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * �� ���� ����ü (�� ��ȯ�� �ӽ� �����)
 */
USTRUCT(BlueprintType)
struct BRIDGERUN_API FPlayerTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerID;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerName;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 TeamID = -1;

    FPlayerTeamInfo() {}

    FPlayerTeamInfo(const FString& InPlayerName, int32 InTeamID)
        : PlayerName(InPlayerName), TeamID(InTeamID)
    {
        PlayerID = FString::Printf(TEXT("Player_%lld_%d"),
            FDateTime::Now().GetTicks(),
            FMath::RandRange(1000, 9999));
    }

    FPlayerTeamInfo(const FString& InPlayerID, const FString& InPlayerName, int32 InTeamID)
        : PlayerID(InPlayerID), PlayerName(InPlayerName), TeamID(InTeamID) {
    }
};

/**
 * BridgeRun ���� �ν��Ͻ� - Ŭ���̾�Ʈ ���� ���� + �� ��ȯ �����͸� ����
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();
    virtual void Init() override;

    // =========================
    // Ŭ���̾�Ʈ ���� ���� (���� ����)
    // =========================

    /** ������ ����� �г��� (���� ���ӽ� �⺻��) */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    FString LastUsedNickname = TEXT("");

    /** ��ȣ�ϴ� ��Ų �ε��� */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    int32 PreferredSkinIndex = 0;

    /** �׷��� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    int32 GraphicsQuality = 2; // 0=Low, 1=Medium, 2=High, 3=Epic

    /** ���� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float BGMVolume = 1.0f;

    // ���� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void SaveLocalSettings();

    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void LoadLocalSettings();

    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void ResetToDefaults();

    // =========================
    // �κ� �ý��� ȣȯ�� (SimpleLobbySystem��)
    // =========================

    UFUNCTION(BlueprintCallable, Category = "Steam Debug")
    void CheckSteamSDKStatus();

    /** �κ� �ý��� ȣȯ�� �÷��̾� �̸� */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    FText PlayerNameText;

    /** �κ� �ý��� ȣȯ�� ��Ų �ε��� */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    int32 SkinIndex = 0;

    /** �̱��� ���� ȣȯ */
    UFUNCTION(BlueprintCallable, Category = "Lobby Compatibility")
    static UBridgeRunGameInstance* GetInstance();

    // =========================
    // UI �׺���̼� ����
    // =========================

    /** �α��� ȭ�� �ǳʶٱ� */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bSkipLoginScreen = false;

    /** �г��� ���� �Ϸ� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bHasPlayerName = false;

    /** ���ӿ��� ���� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bReturningFromGame = false;

    // =========================
    // �� ��ȯ �ӽ� ������ (�ּ��Ѹ�)
    // =========================

    /** �� ��ȯ�� �� ���� �ӽ� ���� */
    UPROPERTY(BlueprintReadWrite, Category = "Map Transition")
    TArray<FPlayerTeamInfo> TempPlayerTeamInfo;

    /** �� ��ȯ�� �� ���� ���� */
    UFUNCTION(BlueprintCallable, Category = "Map Transition")
    void SaveTeamInfoForTransition(const TArray<FPlayerTeamInfo>& TeamInfo);

    /** �� ��ȯ�� �� ���� �������� */
    UFUNCTION(BlueprintPure, Category = "Map Transition")
    TArray<FPlayerTeamInfo> GetTeamInfoFromTransition() const;

    /** �� ��ȯ�� Ư�� �÷��̾� �� ID �������� */
    UFUNCTION(BlueprintPure, Category = "Map Transition")
    int32 GetPlayerTeamIDForTransition(const FString& PlayerID) const;

    /** �� ��ȯ ������ �ʱ�ȭ */
    UFUNCTION(BlueprintCallable, Category = "Map Transition")
    void ClearTransitionData();

    // =========================
    // ���� �Լ���
    // =========================

    /** ���� �÷��̾� �г��� ���� */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetCurrentPlayerName(const FString& NewPlayerName);

    /** ���� �÷��̾� �г��� �������� (FString) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetCurrentPlayerName() const;

    /** ���� �÷��̾� �г��� �������� (FText - �κ� ȣȯ��) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FText GetCurrentPlayerNameAsText() const;

protected:
    // ���� �÷��̾� �̸� (���� ������)
    UPROPERTY(BlueprintReadWrite, Category = "Internal")
    FString CurrentPlayerName;

private:
    // �̱��� ���� ����
    static TWeakObjectPtr<UBridgeRunGameInstance> Instance;
};