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
    // 전투 모드 진입 시 필요한 초기화 작업
}