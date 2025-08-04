// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

TWeakObjectPtr<UBridgeRunGameInstance> UBridgeRunGameInstance::Instance;

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // �⺻�� ����
    LastUsedNickname = TEXT("");
    PreferredSkinIndex = 0;
    GraphicsQuality = 2;
    MasterVolume = 1.0f;
    SFXVolume = 1.0f;
    BGMVolume = 1.0f;

    CurrentPlayerName = TEXT("");
    PlayerNameText = FText::FromString(TEXT(""));
    SkinIndex = 0;

    bSkipLoginScreen = false;
    bHasPlayerName = false;
    bReturningFromGame = false;
}

void UBridgeRunGameInstance::Init()
{
    Super::Init();

    Instance = this;

    // ���� ���� �ڵ� �ε�
    LoadLocalSettings();

    // �⺻ ȣȯ�� ����
    PlayerNameText = FText::FromString(CurrentPlayerName);
}

UBridgeRunGameInstance* UBridgeRunGameInstance::GetInstance()
{
    return Instance.Get();
}

// =========================
// Ŭ���̾�Ʈ ���� ����
// =========================

void UBridgeRunGameInstance::SaveLocalSettings()
{
    // ���� ������ SaveGame �ý��� ���
    // ����� �޸𸮿��� ����
}

void UBridgeRunGameInstance::LoadLocalSettings()
{
    // ���� ������ SaveGame �ý��ۿ��� �ε�
    // ����� �⺻�� ���
}

void UBridgeRunGameInstance::ResetToDefaults()
{
    LastUsedNickname = TEXT("");
    PreferredSkinIndex = 0;
    GraphicsQuality = 2;
    MasterVolume = 1.0f;
    SFXVolume = 1.0f;
    BGMVolume = 1.0f;

    SaveLocalSettings();
}

// =========================
// �� ��ȯ �ӽ� ������
// =========================

void UBridgeRunGameInstance::SaveTeamInfoForTransition(const TArray<FPlayerTeamInfo>& TeamInfo)
{
    TempPlayerTeamInfo = TeamInfo;
}

TArray<FPlayerTeamInfo> UBridgeRunGameInstance::GetTeamInfoFromTransition() const
{
    return TempPlayerTeamInfo;
}

int32 UBridgeRunGameInstance::GetPlayerTeamIDForTransition(const FString& PlayerID) const
{
    for (const FPlayerTeamInfo& Info : TempPlayerTeamInfo)
    {
        if (Info.PlayerID == PlayerID)
        {
            return Info.TeamID;
        }
    }
    return -1;
}

void UBridgeRunGameInstance::ClearTransitionData()
{
    TempPlayerTeamInfo.Empty();
}

// =========================
// ���� �Լ���
// =========================

void UBridgeRunGameInstance::SetCurrentPlayerName(const FString& NewPlayerName)
{
    CurrentPlayerName = NewPlayerName;
    PlayerNameText = FText::FromString(NewPlayerName);
    LastUsedNickname = NewPlayerName; // ������ ����� �⺻������ ����
    bHasPlayerName = !NewPlayerName.IsEmpty();
}

FString UBridgeRunGameInstance::GetCurrentPlayerName() const
{
    return CurrentPlayerName;
}

FText UBridgeRunGameInstance::GetCurrentPlayerNameAsText() const
{
    return PlayerNameText;
}