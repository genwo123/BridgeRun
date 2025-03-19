// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

/**
 * 팀 정보를 저장하는 기본 구조체
 */
USTRUCT(BlueprintType)
struct FBasicTeamInfo
{
    GENERATED_BODY()

    /** 팀 고유 식별자 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 TeamId = 0;

    /** 팀 점수 */
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    int32 Score = 0;

    FBasicTeamInfo() = default;
};

/**
 * 브리지런 게임의 전체 상태를 관리하는 클래스
 */
UCLASS()
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ABridgeRunGameState();

    /** 팀 점수 업데이트 (모든 클라이언트에 전파) */
    UFUNCTION(NetMulticast, Reliable, Category = "Team")
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);

    /** 모든 팀 정보 */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<FBasicTeamInfo> Teams;

    /** 남은 경기 시간 (초) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    float MatchTime;

protected:
    /** 네트워크 복제 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore);
};