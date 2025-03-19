// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BridgeRunPlayerState.generated.h"

/**
 * 브리지런 게임의 플레이어 상태 클래스
 * 플레이어 팀 정보를 관리합니다
 */
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ABridgeRunPlayerState();

    /** 네트워크 복제 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 팀 ID 설정 */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);

    /** 팀 ID 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const;

    /** 플레이어의 팀 ID */
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    /** 팀 ID 변경 시 호출되는 함수 */
    UFUNCTION()
    void OnRep_TeamID();
};