// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Citizen.h"
#include <BridgeRunGameMode.h>

ABridgeRunPlayerState::ABridgeRunPlayerState()
{
    // 기본 설정
    TeamID = -1;
}

void ABridgeRunPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // TeamID 복제 설정
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
    // TeamID가 변경될 때 실행할 코드
    UE_LOG(LogTemp, Log, TEXT("PlayerState TeamID changed to %d"), TeamID);

    // 소유 캐릭터가 있는 경우 팀 머티리얼 업데이트
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