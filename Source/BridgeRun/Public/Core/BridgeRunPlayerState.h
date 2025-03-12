// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BridgeRunPlayerState.generated.h"
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ABridgeRunPlayerState();
    // 네트워크 복제 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    // 팀 ID 설정 및 가져오기
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const;

    // 팀 ID를 ReplicatedUsing으로 변경
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    UFUNCTION()
    void OnRep_TeamID();

};