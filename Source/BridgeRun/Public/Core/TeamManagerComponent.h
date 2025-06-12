// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TeamManagerComponent.generated.h"

// 팀 정보 구조체
USTRUCT(BlueprintType)
struct BRIDGERUN_API FTeamInfo
{
    GENERATED_BODY()

    // 팀 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FString TeamName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FLinearColor TeamColor;

    // 현재 팀 인원 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 PlayerCount;

    FTeamInfo()
        : TeamID(0)
        , TeamName(TEXT(""))
        , TeamColor(FLinearColor::White)
        , PlayerCount(0)
    {
    }
};

/**
 * 팀 관리를 담당하는 컴포넌트
 * 현재: 자동 팀 배정 로직 사용 (임시)
 * 향후: 로비 시스템과 연동하여 플레이어가 팀 선택
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BRIDGERUN_API UTeamManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 기본 함수
    UTeamManagerComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 팀 초기화 및 설정 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    void InitializeTeamColors();

    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTeamActiveStatus();

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetActiveTeamCount(int32 Count);

    // 팀 정보 접근 함수
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetActiveTeamCount() const { return ActiveTeamCount; }

    UFUNCTION(BlueprintPure, Category = "Team")
    FLinearColor GetTeamColor(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team")
    FString GetTeamName(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, Category = "Team")
    TArray<FTeamInfo> GetActiveTeams() const;

    // 플레이어 팀 관리 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignPlayerToTeam(AController* PlayerController);

    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetPlayerTeamID(AController* PlayerController) const;

    UFUNCTION(BlueprintCallable, Category = "Team|Future")
    void ReallocatePlayersToTeams();

    // 향후 로비 시스템용 함수
    UFUNCTION(BlueprintCallable, Category = "Team|Future")
    bool RequestTeamChange(AController* PlayerController, int32 RequestedTeamID);

protected:
    // 팀 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxTeams = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Team", meta = (ClampMin = "2", ClampMax = "4"))
    int32 ActiveTeamCount = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxPlayersPerTeam = 3;

    // 팀 데이터
    UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Team")
    TArray<FTeamInfo> TeamInfo;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    TArray<bool> TeamActive;

private:
    // 플레이어-팀 맵핑
    UPROPERTY()
    TMap<AController*, int32> PlayerTeamMap;

    // 내부 유틸리티 함수
    int32 GetOptimalTeamForTeam() const; // 기존 함수 (4팀 모두 고려)

    // ★ 새로 추가: 활성화된 팀만 고려하는 함수 ★
    int32 GetOptimalActiveTeam() const;

    void RespawnPlayerInTeam(AController* PlayerController, int32 TeamID);
    AActor* FindPlayerStartForTeam(AController* Controller, const FString& TeamTag);
};