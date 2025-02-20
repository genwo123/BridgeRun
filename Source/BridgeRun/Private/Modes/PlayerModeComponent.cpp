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
        // ��� ��ȯ ��ȿ�� üũ �߰�
        if (!IsValidModeTransition(CurrentMode, NewMode))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid mode transition from %s to %s"),
                *UEnum::GetValueAsString(CurrentMode),
                *UEnum::GetValueAsString(NewMode));
            return;
        }

        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnRep_CurrentMode(OldMode);
    }
}

void UPlayerModeComponent::OnRep_CurrentMode(EPlayerMode OldMode)
{
    OnPlayerModeChanged.Broadcast(CurrentMode, OldMode);
}


bool UPlayerModeComponent::IsValidModeTransition(EPlayerMode FromMode, EPlayerMode ToMode) const
{
    // Combat ��忡�� Build ������ ���� ��ȯ ����
    if (FromMode == EPlayerMode::Combat && ToMode == EPlayerMode::Build)
    {
        return false;
    }

    // Combat ��忡���� Normal ���θ� ��ȯ ����
    if (FromMode == EPlayerMode::Combat && ToMode != EPlayerMode::Normal)
    {
        return false;
    }

    return true;
}