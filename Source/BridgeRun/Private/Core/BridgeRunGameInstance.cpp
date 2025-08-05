#include "Core/BridgeRunGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

// === OnlineSubsystem 헤더들 ===
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"

// === Steam SDK 헤더 ===
#if STEAM_SDK_ENABLED
#include "steam_api.h"
#endif

TWeakObjectPtr<UBridgeRunGameInstance> UBridgeRunGameInstance::Instance;

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // 기본값 설정
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

    // 로컬 설정 자동 로드
    LoadLocalSettings();

    // 기본 호환성 설정
    PlayerNameText = FText::FromString(CurrentPlayerName);
}

UBridgeRunGameInstance* UBridgeRunGameInstance::GetInstance()
{
    return Instance.Get();
}

// =========================
// 클라이언트 로컬 설정
// =========================

void UBridgeRunGameInstance::SaveLocalSettings()
{
    // 실제 구현시 SaveGame 시스템 사용
    // 현재는 메모리에만 저장
}

void UBridgeRunGameInstance::LoadLocalSettings()
{
    // 실제 구현시 SaveGame 시스템에서 로드
    // 현재는 기본값 사용
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
// 맵 전환 임시 데이터
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
// 편의 함수들
// =========================

void UBridgeRunGameInstance::SetCurrentPlayerName(const FString& NewPlayerName)
{
    CurrentPlayerName = NewPlayerName;
    PlayerNameText = FText::FromString(NewPlayerName);
    LastUsedNickname = NewPlayerName; // 다음에 사용할 기본값으로 저장
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

    // 모든 Steam API 호출 임시 주석 처리
    /*
#if STEAM_SDK_ENABLED
    // Steam 코드 전체 주석...
#endif
    */

    // OnlineSubsystem만 확인
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