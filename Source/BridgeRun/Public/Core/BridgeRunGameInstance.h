// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * 팀 정보 구조체 (맵 전환시 임시 저장용)
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
 * BridgeRun 게임 인스턴스 - 클라이언트 로컬 설정 + 맵 전환 데이터만 관리
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();
    virtual void Init() override;

    // =========================
    // 클라이언트 로컬 설정 (메인 역할)
    // =========================

    /** 마지막 사용한 닉네임 (다음 접속시 기본값) */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    FString LastUsedNickname = TEXT("");

    /** 선호하는 스킨 인덱스 */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    int32 PreferredSkinIndex = 0;

    /** 그래픽 설정 */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    int32 GraphicsQuality = 2; // 0=Low, 1=Medium, 2=High, 3=Epic

    /** 사운드 설정 */
    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Local Settings")
    float BGMVolume = 1.0f;

    // 로컬 설정 관리
    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void SaveLocalSettings();

    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void LoadLocalSettings();

    UFUNCTION(BlueprintCallable, Category = "Local Settings")
    void ResetToDefaults();

    // =========================
    // 로비 시스템 호환성 (SimpleLobbySystem용)
    // =========================

    UFUNCTION(BlueprintCallable, Category = "Steam Debug")
    void CheckSteamSDKStatus();

    /** 로비 시스템 호환용 플레이어 이름 */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    FText PlayerNameText;

    /** 로비 시스템 호환용 스킨 인덱스 */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    int32 SkinIndex = 0;

    /** 싱글톤 패턴 호환 */
    UFUNCTION(BlueprintCallable, Category = "Lobby Compatibility")
    static UBridgeRunGameInstance* GetInstance();

    // =========================
    // UI 네비게이션 상태
    // =========================

    /** 로그인 화면 건너뛰기 */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bSkipLoginScreen = false;

    /** 닉네임 설정 완료 여부 */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bHasPlayerName = false;

    /** 게임에서 복귀 여부 */
    UPROPERTY(BlueprintReadWrite, Category = "UI Navigation")
    bool bReturningFromGame = false;

    // =========================
    // 맵 전환 임시 데이터 (최소한만)
    // =========================

    /** 맵 전환시 팀 정보 임시 저장 */
    UPROPERTY(BlueprintReadWrite, Category = "Map Transition")
    TArray<FPlayerTeamInfo> TempPlayerTeamInfo;

    /** 맵 전환용 팀 정보 저장 */
    UFUNCTION(BlueprintCallable, Category = "Map Transition")
    void SaveTeamInfoForTransition(const TArray<FPlayerTeamInfo>& TeamInfo);

    /** 맵 전환용 팀 정보 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Map Transition")
    TArray<FPlayerTeamInfo> GetTeamInfoFromTransition() const;

    /** 맵 전환용 특정 플레이어 팀 ID 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Map Transition")
    int32 GetPlayerTeamIDForTransition(const FString& PlayerID) const;

    /** 맵 전환 데이터 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Map Transition")
    void ClearTransitionData();

    // =========================
    // 편의 함수들
    // =========================

    /** 현재 플레이어 닉네임 설정 */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetCurrentPlayerName(const FString& NewPlayerName);

    /** 현재 플레이어 닉네임 가져오기 (FString) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetCurrentPlayerName() const;

    /** 현재 플레이어 닉네임 가져오기 (FText - 로비 호환용) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FText GetCurrentPlayerNameAsText() const;

protected:
    // 현재 플레이어 이름 (내부 관리용)
    UPROPERTY(BlueprintReadWrite, Category = "Internal")
    FString CurrentPlayerName;

private:
    // 싱글톤 패턴 지원
    static TWeakObjectPtr<UBridgeRunGameInstance> Instance;
};