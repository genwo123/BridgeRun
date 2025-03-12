// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Citizen.h"
#include <BridgeRunGameMode.h>

ABridgeRunPlayerState::ABridgeRunPlayerState()
{
    // �⺻ ����
    TeamID = -1;
}

void ABridgeRunPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // TeamID ���� ����
    DOREPLIFETIME(ABridgeRunPlayerState, TeamID);
}

void ABridgeRunPlayerState::SetTeamID(int32 NewTeamID)
{
    TeamID = NewTeamID;
}

int32 ABridgeRunPlayerState::GetTeamID() const
{
    return TeamID;
}

void ABridgeRunPlayerState::OnRep_TeamID()
{
    // TeamID�� ����� �� ������ �ڵ�
    UE_LOG(LogTemp, Log, TEXT("PlayerState TeamID changed to %d"), TeamID);

    // ���� ĳ���Ͱ� �ִ� ��� �� ��Ƽ���� ������Ʈ
    AController* OwningController = Cast<AController>(GetOwner());
    if (OwningController)
    {
        APawn* ControlledPawn = OwningController->GetPawn();
        if (ControlledPawn)
        {
            ACitizen* Character = Cast<ACitizen>(ControlledPawn);
            if (Character)
            {
                Character->TeamID = TeamID;
                Character->SetTeamMaterial(TeamID);
            }
        }
    }
}