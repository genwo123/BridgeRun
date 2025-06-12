// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BridgeRunGameInstance.generated.h"

/**
 * 플레이어 정보 저장 구조체 (팀 정보 + 이름 통합)
 */
USTRUCT(BlueprintType)
struct BRIDGERUN_API FPlayerTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerID;           // 플레이어 고유 ID (자동 생성)

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerName;         // 플레이어 닉네임

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 TeamID = -1;          // 팀 ID

    FPlayerTeamInfo() {}

    // 생성자 (ID 자동 생성)
    FPlayerTeamInfo(const FString& InPlayerName, int32 InTeamID)
        : PlayerName(InPlayerName), TeamID(InTeamID)
    {
        // PlayerID 자동 생성 (타임스탬프 + 랜덤)
        PlayerID = FString::Printf(TEXT("Player_%lld_%d"),
            FDateTime::Now().GetTicks(),
            FMath::RandRange(1000, 9999));
    }

    // 기존 호환용 생성자
    FPlayerTeamInfo(const FString& InPlayerID, const FString& InPlayerName, int32 InTeamID)
        : PlayerID(InPlayerID), PlayerName(InPlayerName), TeamID(InTeamID)
    {
    }
};

/**
 * 게임 인스턴스 클래스 - 로비 + 팀 관리 통합
 */
UCLASS(BlueprintType, Blueprintable, Config = Game)
class BRIDGERUN_API UBridgeRunGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBridgeRunGameInstance();
    virtual void Init() override;

    // =========================
    // 로비 시스템 호환성 (기존 LobbyGameInstance 대체)
    // =========================

    /** 기존 로비 시스템 호환용 - 플레이어 이름 (FText) */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    FText PlayerNameText;

    /** 기존 로비 시스템 호환용 - 스킨 인덱스 */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Compatibility")
    int32 SkinIndex = 0;

    /** 싱글톤 패턴 호환 (기존 로비 코드에서 사용) */
    UFUNCTION(BlueprintCallable, Category = "Lobby Compatibility")
    static UBridgeRunGameInstance* GetInstance();

    // =========================
    // 플레이어 관리 함수 (단순화)
    // =========================

    /** 현재 플레이어 이름 설정 */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetCurrentPlayerName(const FString& NewPlayerName);

    /** 현재 플레이어 이름 가져오기 (FString) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetCurrentPlayerName() const;

    /** 현재 플레이어 이름 가져오기 (FText - 로비 호환용) */
    UFUNCTION(BlueprintPure, Category = "Player")
    FText GetCurrentPlayerNameAsText() const;

    // =========================
    // 팀 점수 관리 함수
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
    // 플레이어 정보 관리 함수 (단순화)
    // =========================

    /** 플레이어 정보 저장 (이름으로 자동 ID 생성) */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    FString AddPlayerInfo(const FString& PlayerName, int32 TeamID);

    /** 플레이어 정보 저장 (ID 직접 지정) */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void SavePlayerTeamInfo(const FString& PlayerID, int32 TeamID, const FString& PlayerName = "");

    /** 플레이어 ID로 팀 ID 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetPlayerTeamID(const FString& PlayerID) const;

    /** 플레이어 ID로 이름 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Teams")
    FString GetPlayerNameByID(const FString& PlayerID) const;

    /** 모든 플레이어 정보 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<FPlayerTeamInfo> GetAllPlayersInfo() const;

    /** 특정 팀의 플레이어들 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<FPlayerTeamInfo> GetPlayersByTeam(int32 TeamID) const;

    /** 플레이어 정보 전체 삭제 */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void ClearPlayersTeamInfo();

    /** 디버그용: 모든 플레이어 정보 출력 */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void PrintPlayersTeamInfo() const;




    // =========================
    // 팀 균형 관리 함수
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
    // 팀 점수 배열
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<int32> TeamScores;

    // 플레이어 정보 배열
    UPROPERTY(BlueprintReadWrite, Category = "Teams")
    TArray<FPlayerTeamInfo> PlayersTeamInfo;

    // 현재 플레이어 이름 (FString)
    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CurrentPlayerName;

    // 팀 수 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumberOfTeams = 4;

    // 팀 균형 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 MinimumTeamsRequired = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
    int32 MaxPlayerDifferenceAllowed = 1;

    // 유효한 팀 ID 확인
    bool IsValidTeamID(int32 TeamID) const;

private:
    // 싱글톤 패턴 지원
    static TWeakObjectPtr<UBridgeRunGameInstance> Instance;
};