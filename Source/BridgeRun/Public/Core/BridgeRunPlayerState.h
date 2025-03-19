// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BridgeRunPlayerState.generated.h"

/**
 * �긮���� ������ �÷��̾� ���� Ŭ����
 * �÷��̾� �� ������ �����մϴ�
 */
UCLASS()
class BRIDGERUN_API ABridgeRunPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    /** �⺻ ������ */
    ABridgeRunPlayerState();

    /** ��Ʈ��ũ ���� ���� */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** �� ID ���� */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID);

    /** �� ID �������� */
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetTeamID() const;

    /** �÷��̾��� �� ID */
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID;

    /** �� ID ���� �� ȣ��Ǵ� �Լ� */
    UFUNCTION()
    void OnRep_TeamID();
};