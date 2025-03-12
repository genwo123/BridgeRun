// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "TeamManagerComponent.generated.h"

// 팀 정보 구조체
USTRUCT(BlueprintType)
struct BRIDGERUN_API FTeamInfo
{
    GENERATED_BODY()

    // 팀 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID;

    // 팀 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FString TeamName;

    // 팀 색상
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
 * 게임모드에 추가하여 사용
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BRIDGERUN_API UTeamManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 생성자 및 초기화
    UTeamManagerComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 팀 색상 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    void InitializeTeamColors();

    // 플레이어를 팀에 할당하는 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignPlayerToTeam(AController* PlayerController);

    // 활성 팀 수 설정 (2-4)
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetActiveTeamCount(int32 Count);

    // 현재 활성화된 팀 수 가져오기
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetActiveTeamCount() const { return ActiveTeamCount; }

    // 특정 팀의 색상 가져오기
    UFUNCTION(BlueprintPure, Category = "Team")
    FLinearColor GetTeamColor(int32 TeamID) const;

    // 특정 팀의 이름 가져오기
    UFUNCTION(BlueprintPure, Category = "Team")
    FString GetTeamName(int32 TeamID) const;

    // 플레이어의 팀 ID 가져오기
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetPlayerTeamID(AController* PlayerController) const;

    // 활성화된 팀 정보 가져오기
    UFUNCTION(BlueprintCallable, Category = "Team")
    TArray<FTeamInfo> GetActiveTeams() const;

    // 플레이어를 재배정하는 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    void ReallocatePlayersToTeams();

    // 플레이어가 팀을 변경하는 함수
    UFUNCTION(BlueprintCallable, Category = "Team")
    bool RequestTeamChange(AController* PlayerController, int32 RequestedTeamID);

    // 팀 활성화 상태 업데이트
    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTeamActiveStatus();







protected:
    // 최대 팀 수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxTeams = 4;

    // 활성화된 팀 수 (2-4)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Team", meta = (ClampMin = "2", ClampMax = "4"))
    int32 ActiveTeamCount = 2;

    // 팀 정보 배열
    UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Team")
    TArray<FTeamInfo> TeamInfo;

    // 팀 활성화 상태 배열
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    TArray<bool> TeamActive;

    // 팀 인원 제한
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxPlayersPerTeam = 8;

private:
    // 플레이어-팀 맵핑을 저장할 변수
    UPROPERTY()
    TMap<AController*, int32> PlayerTeamMap;

    // 최적의 팀 찾기 (가장 인원이 적은 팀)
    int32 GetOptimalTeamForTeam() const;

    // 팀 변경 시 필요한 경우 플레이어 리스폰 처리
    void RespawnPlayerInTeam(AController* PlayerController, int32 TeamID);


    AActor* FindPlayerStartForTeam(AController* Controller, const FString& TeamTag);
};