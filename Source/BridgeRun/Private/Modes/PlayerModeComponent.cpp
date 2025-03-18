// Private/Modes/PlayerModeComponent.cpp
#include "Modes/PlayerModeComponent.h"
#include "Net/UnrealNetwork.h"

UPlayerModeComponent::UPlayerModeComponent()
{
    // ������Ʈ �⺻ ����
    PrimaryComponentTick.bCanEverTick = false;
    CurrentMode = EPlayerMode::Normal;
    SetIsReplicatedByDefault(true);
}

void UPlayerModeComponent::BeginPlay()
{
    Super::BeginPlay();
    // �ʱ�ȭ ������ �ʿ��ϸ� ���⿡ �߰�
}

void UPlayerModeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ��� ���� ����
    DOREPLIFETIME(UPlayerModeComponent, CurrentMode);
}

void UPlayerModeComponent::SetPlayerMode_Implementation(EPlayerMode NewMode)
{
    // ���� ���� �˻�
    if (!GetOwner()->HasAuthority())
        return;

    // ��� ������ �ʿ��� ��쿡�� ó��
    if (CurrentMode != NewMode)
    {
        // ��� ��ȯ ��ȿ�� �˻�
        if (!IsValidModeTransition(CurrentMode, NewMode))
        {
            // ���� �ڵ��� �α� ���� �״�� ����
            UE_LOG(LogTemp, Warning, TEXT("Invalid mode transition from %s to %s"),
                *UEnum::GetValueAsString(CurrentMode),
                *UEnum::GetValueAsString(NewMode));
            return;
        }

        // ��� ���� �� �̺�Ʈ �߻�
        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnRep_CurrentMode(OldMode);
    }
}

void UPlayerModeComponent::OnRep_CurrentMode(EPlayerMode OldMode)
{
    // ��� ���� �̺�Ʈ ���
    OnPlayerModeChanged.Broadcast(CurrentMode, OldMode);
}

bool UPlayerModeComponent::IsValidModeTransition(EPlayerMode FromMode, EPlayerMode ToMode) const
{
    // ���� ��忡�� �Ǽ� ���� ���� ��ȯ ����
    if (FromMode == EPlayerMode::Combat && ToMode == EPlayerMode::Build)
    {
        return false;
    }

    // ���� ��忡���� �Ϲ� ���θ� ��ȯ ����
    if (FromMode == EPlayerMode::Combat && ToMode != EPlayerMode::Normal)
    {
        return false;
    }

    // �� �� ��ȯ�� ��� ���
    return true;
}