// Public/Modes/PlayerModeComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Modes/PlayerModeTypes.h"
#include "PlayerModeComponent.generated.h"

// ��� ���� �� �߻��ϴ� �̺�Ʈ ��������Ʈ
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerModeChanged, EPlayerMode, NewMode, EPlayerMode, OldMode);

/**
 * �÷��̾� ��带 �����ϴ� ������Ʈ
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UPlayerModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // �⺻ �Լ�
    UPlayerModeComponent();

    // ��� ���� �Լ�
    UFUNCTION(Server, Reliable)
    void SetPlayerMode(EPlayerMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Player Mode")
    EPlayerMode GetCurrentMode() const { return CurrentMode; }

    // �̺�Ʈ ��������Ʈ
    UPROPERTY(BlueprintAssignable, Category = "Player Mode")
    FOnPlayerModeChanged OnPlayerModeChanged;

protected:
    // ����������Ŭ �Լ�
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ���� �̺�Ʈ ó��
    UFUNCTION()
    void OnRep_CurrentMode(EPlayerMode OldMode);

    // ��� ��ȯ ��ȿ�� �˻�
    bool IsValidModeTransition(EPlayerMode FromMode, EPlayerMode ToMode) const;

private:
    // ���� ������
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMode)
    EPlayerMode CurrentMode;
};