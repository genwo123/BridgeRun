// CombatComponent.cpp
#include "CombatComponent.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCombatComponent::OnCombatModeEntered()
{
    // ���� ��� ���� �� �ʿ��� �ʱ�ȭ �۾�
}