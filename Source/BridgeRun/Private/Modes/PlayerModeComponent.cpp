// Private/Modes/PlayerModeComponent.cpp
#include "Modes/PlayerModeComponent.h"
#include "Net/UnrealNetwork.h"

UPlayerModeComponent::UPlayerModeComponent()
{
    // 컴포넌트 기본 설정
    PrimaryComponentTick.bCanEverTick = false;
    CurrentMode = EPlayerMode::Normal;
    SetIsReplicatedByDefault(true);
}

void UPlayerModeComponent::BeginPlay()
{
    Super::BeginPlay();
    // 초기화 로직이 필요하면 여기에 추가
}

void UPlayerModeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 모드 상태 복제
    DOREPLIFETIME(UPlayerModeComponent, CurrentMode);
}

void UPlayerModeComponent::SetPlayerMode_Implementation(EPlayerMode NewMode)
{
    // 서버 권한 검사
    if (!GetOwner()->HasAuthority())
        return;

    // 모드 변경이 필요한 경우에만 처리
    if (CurrentMode != NewMode)
    {
        // 모드 전환 유효성 검사
        if (!IsValidModeTransition(CurrentMode, NewMode))
        {
            // 원본 코드의 로그 형식 그대로 유지
            UE_LOG(LogTemp, Warning, TEXT("Invalid mode transition from %s to %s"),
                *UEnum::GetValueAsString(CurrentMode),
                *UEnum::GetValueAsString(NewMode));
            return;
        }

        // 모드 변경 및 이벤트 발생
        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnRep_CurrentMode(OldMode);
    }
}

void UPlayerModeComponent::OnRep_CurrentMode(EPlayerMode OldMode)
{
    // 모드 변경 이벤트 방송
    OnPlayerModeChanged.Broadcast(CurrentMode, OldMode);
}

bool UPlayerModeComponent::IsValidModeTransition(EPlayerMode FromMode, EPlayerMode ToMode) const
{
    // 전투 모드에서 건설 모드로 직접 전환 금지
    if (FromMode == EPlayerMode::Combat && ToMode == EPlayerMode::Build)
    {
        return false;
    }

    // 전투 모드에서는 일반 모드로만 전환 가능
    if (FromMode == EPlayerMode::Combat && ToMode != EPlayerMode::Normal)
    {
        return false;
    }

    // 그 외 전환은 모두 허용
    return true;
}