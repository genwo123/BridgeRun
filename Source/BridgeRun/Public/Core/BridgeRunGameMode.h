// Public/Core/BridgeRunGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BridgeRunGameMode.generated.h"

UCLASS(minimalapi)
class ABridgeRunGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABridgeRunGameMode();
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    TArray<FVector> PlayerStartLocations;
};