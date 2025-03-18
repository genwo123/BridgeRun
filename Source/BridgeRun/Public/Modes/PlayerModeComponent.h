// Public/Modes/PlayerModeComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Modes/PlayerModeTypes.h"
#include "PlayerModeComponent.generated.h"

// 모드 변경 시 발생하는 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerModeChanged, EPlayerMode, NewMode, EPlayerMode, OldMode);

/**
 * 플레이어 모드를 관리하는 컴포넌트
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UPlayerModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 기본 함수
    UPlayerModeComponent();

    // 모드 관리 함수
    UFUNCTION(Server, Reliable)
    void SetPlayerMode(EPlayerMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Player Mode")
    EPlayerMode GetCurrentMode() const { return CurrentMode; }

    // 이벤트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Player Mode")
    FOnPlayerModeChanged OnPlayerModeChanged;

protected:
    // 라이프사이클 함수
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 복제 이벤트 처리
    UFUNCTION()
    void OnRep_CurrentMode(EPlayerMode OldMode);

    // 모드 전환 유효성 검사
    bool IsValidModeTransition(EPlayerMode FromMode, EPlayerMode ToMode) const;

private:
    // 상태 데이터
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMode)
    EPlayerMode CurrentMode;
};