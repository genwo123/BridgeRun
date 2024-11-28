// PlayerModeComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerModeTypes.h"
#include "PlayerModeComponent.generated.h"

// 델리게이트 선언 위치 주의: USTRUCT나 UCLASS 선언 전에 와야 함
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerModeChanged, EPlayerMode, NewMode, EPlayerMode, OldMode);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UPlayerModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerModeComponent();

    UFUNCTION(BlueprintCallable, Category = "Player Mode")
    void SetPlayerMode(EPlayerMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Player Mode")
    EPlayerMode GetCurrentMode() const { return CurrentMode; }

    // 델리게이트 선언
    UPROPERTY(BlueprintAssignable, Category = "Player Mode")
    FOnPlayerModeChanged OnPlayerModeChanged;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    EPlayerMode CurrentMode;
};