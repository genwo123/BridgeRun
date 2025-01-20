// Public/Core/BridgeRunGameState.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BridgeRunGameState.generated.h"

USTRUCT(BlueprintType)
struct FBasicTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 TeamId;

    UPROPERTY(BlueprintReadWrite)
    int32 Score;

    FBasicTeamInfo()
    {
        TeamId = 0;
        Score = 0;
    }
};

UCLASS()
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ABridgeRunGameState();

    UPROPERTY(Replicated, BlueprintReadOnly)
    TArray<FBasicTeamInfo> Teams;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float MatchTime;

    UFUNCTION(NetMulticast, Reliable)
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};