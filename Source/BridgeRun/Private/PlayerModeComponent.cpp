// PlayerModeComponent.cpp
#include "PlayerModeComponent.h"

UPlayerModeComponent::UPlayerModeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentMode = EPlayerMode::Normal;
}

void UPlayerModeComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerModeComponent::SetPlayerMode(EPlayerMode NewMode)
{
    if (CurrentMode != NewMode)
    {
        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnPlayerModeChanged.Broadcast(NewMode, OldMode);
    }
}