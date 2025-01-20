// Private/Modes/PlayerModeComponent.cpp
#include "Modes/PlayerModeComponent.h"
#include "Net/UnrealNetwork.h"

UPlayerModeComponent::UPlayerModeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentMode = EPlayerMode::Normal;
    SetIsReplicatedByDefault(true);
}

void UPlayerModeComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerModeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPlayerModeComponent, CurrentMode);
}

void UPlayerModeComponent::SetPlayerMode_Implementation(EPlayerMode NewMode)
{
    if (!GetOwner()->HasAuthority()) return;

    if (CurrentMode != NewMode)
    {
        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnRep_CurrentMode(OldMode);
    }
}

void UPlayerModeComponent::OnRep_CurrentMode(EPlayerMode OldMode)
{
    OnPlayerModeChanged.Broadcast(CurrentMode, OldMode);
}