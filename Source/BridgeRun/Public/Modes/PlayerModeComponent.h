// Public/Modes/PlayerModeComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Modes/PlayerModeTypes.h"
#include "PlayerModeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerModeChanged, EPlayerMode, NewMode, EPlayerMode, OldMode);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UPlayerModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerModeComponent();

	UFUNCTION(Server, Reliable)
	void SetPlayerMode(EPlayerMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Player Mode")
	EPlayerMode GetCurrentMode() const { return CurrentMode; }

	UPROPERTY(BlueprintAssignable, Category = "Player Mode")
	FOnPlayerModeChanged OnPlayerModeChanged;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentMode(EPlayerMode OldMode);

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMode)
	EPlayerMode CurrentMode;
};