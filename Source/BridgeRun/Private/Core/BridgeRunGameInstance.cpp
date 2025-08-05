#include "Core/BridgeRunGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

// === OnlineSubsystem ����� ===
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"

// === Steam SDK ��� ===
#if STEAM_SDK_ENABLED
#include "steam_api.h"
#endif

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



void UBridgeRunGameInstance::CheckSteamSDKStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Steam SDK Status Check ==="));

    // ��� Steam API ȣ�� �ӽ� �ּ� ó��
    /*
#if STEAM_SDK_ENABLED
    // Steam �ڵ� ��ü �ּ�...
#endif
    */

    // OnlineSubsystem�� Ȯ��
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        FString SubsystemName = OnlineSub->GetSubsystemName().ToString();
        UE_LOG(LogTemp, Warning, TEXT(" OnlineSubsystem: %s"), *SubsystemName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT(" OnlineSubsystem: NULL"));
    }
}