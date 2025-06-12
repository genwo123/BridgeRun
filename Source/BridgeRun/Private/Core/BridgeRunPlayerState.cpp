﻿// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Citizen.h"
#include "Engine/Engine.h"

ABridgeRunPlayerState::ABridgeRunPlayerState()
{
    // 네트워크 복제 활성화
    bReplicates = true;
    bAlwaysRelevant = true;

    // 초기 팀 ID는 -1 (미할당)
    TeamID = -1;

    // 모든 통계 초기화 (0으로)
    TotalPlanksBuilt = 0;
    TotalTentsBuilt = 0;
    TotalTrophyScore = 0;
    TotalHitCount = 0;

    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;
}

void ABridgeRunPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 기존 팀 정보
    DOREPLIFETIME(ABridgeRunPlayerState, TeamID);
    DOREPLIFETIME(ABridgeRunPlayerState, DisplayName);

    // 누적 통계 (게임 전체)
    DOREPLIFETIME(ABridgeRunPlayerState, TotalPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalHitCount);

    // 라운드 통계 (라운드별)
    DOREPLIFETIME(ABridgeRunPlayerState, RoundPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundHitCount);
}

// =========================
// 기존 팀 관련 함수들 (유지)
// =========================

void ABridgeRunPlayerState::SetTeamID(int32 NewTeamID)
{
    if (TeamID != NewTeamID)
    {
        TeamID = NewTeamID;
        UE_LOG(LogTemp, Warning, TEXT("PlayerState: SetTeamID %d for %s"), TeamID, *GetPlayerName());

        // ★ 서버에서 OnRep 함수 수동 호출 (클라이언트는 자동 호출됨) ★
        if (HasAuthority())
        {
            OnRep_TeamID();
        }

        // ★ 네트워크 업데이트 강제 실행 ★
        ForceNetUpdate();
    }
}

int32 ABridgeRunPlayerState::GetTeamID() const
{
    return TeamID;
}



void ABridgeRunPlayerState::OnRep_TeamID()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_TeamID: PlayerState TeamID changed to %d"), TeamID);

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

                // ★ 핵심 수정: MulticastSetTeamMaterial 사용 ★
                Character->MulticastSetTeamMaterial(TeamID);

                UE_LOG(LogTemp, Warning, TEXT("OnRep_TeamID: Applied team %d material to character %s"),
                    TeamID, *Character->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("OnRep_TeamID: Character is not ACitizen"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnRep_TeamID: No controlled pawn"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_TeamID: No owning controller"));
    }
}

void ABridgeRunPlayerState::ServerAddPlankBuilt_Implementation()
{
    if (!HasAuthority()) return;

    // 누적 통계와 라운드 통계 둘 다 증가
    TotalPlanksBuilt++;
    RoundPlanksBuilt++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Plank built. Total: %d, Round: %d"),
        *GetPlayerName(), TotalPlanksBuilt, RoundPlanksBuilt);
}

void ABridgeRunPlayerState::ServerAddTentBuilt_Implementation()
{
    if (!HasAuthority()) return;

    // 누적 통계와 라운드 통계 둘 다 증가
    TotalTentsBuilt++;
    RoundTentsBuilt++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Tent built. Total: %d, Round: %d"),
        *GetPlayerName(), TotalTentsBuilt, RoundTentsBuilt);
}

void ABridgeRunPlayerState::ServerAddTrophyScore_Implementation(int32 TrophyPoints)
{
    if (!HasAuthority()) return;

    // 누적 통계와 라운드 통계 둘 다 증가
    TotalTrophyScore += TrophyPoints;
    RoundTrophyScore += TrophyPoints;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Trophy score +%d. Total: %d, Round: %d"),
        *GetPlayerName(), TrophyPoints, TotalTrophyScore, RoundTrophyScore);
}

void ABridgeRunPlayerState::ServerAddHitCount_Implementation()
{
    if (!HasAuthority()) return;

    // 누적 통계와 라운드 통계 둘 다 증가
    TotalHitCount++;
    RoundHitCount++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Hit count. Total: %d, Round: %d"),
        *GetPlayerName(), TotalHitCount, RoundHitCount);
}

void ABridgeRunPlayerState::ServerResetRoundStats_Implementation()
{
    if (!HasAuthority()) return;

    // 라운드 통계만 초기화 (누적 통계는 유지)
    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Round stats reset"), *GetPlayerName());
}

// =========================
// 복제 함수들 (실시간 업데이트 이벤트 발생)
// =========================

void ABridgeRunPlayerState::OnRep_TotalStats()
{
    // BP에서 바인딩할 수 있는 이벤트 브로드캐스트
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, (int32)EStatType::Trophy); // 대표로 Trophy 타입
    }

}

void ABridgeRunPlayerState::OnRep_RoundStats()
{
    // BP에서 바인딩할 수 있는 이벤트 브로드캐스트
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, (int32)EStatType::Planks); // 대표로 Planks 타입
    }

}

void ABridgeRunPlayerState::SetDisplayName(const FString& NewDisplayName)
{
    UE_LOG(LogTemp, Error, TEXT("SetDisplayName Called! Authority: %s, Name: %s"),
        HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *NewDisplayName);

    if (HasAuthority())
    {
        DisplayName = NewDisplayName;
        UE_LOG(LogTemp, Log, TEXT("Server: DisplayName set to: %s"), *DisplayName);
    }
    else
    {
        ServerSetDisplayName(NewDisplayName);
        UE_LOG(LogTemp, Log, TEXT("Client: Sending DisplayName to server: %s"), *NewDisplayName);
    }
}

void ABridgeRunPlayerState::ServerSetDisplayName_Implementation(const FString& NewDisplayName)
{
    DisplayName = NewDisplayName;
    UE_LOG(LogTemp, Log, TEXT("Server: DisplayName received from client: %s"), *DisplayName);

    ForceNetUpdate();
}

bool ABridgeRunPlayerState::IsDisplayNameValid() const
{
    return !DisplayName.IsEmpty() && DisplayName.Len() > 0 && DisplayName.Len() <= 20;
}

void ABridgeRunPlayerState::OnRep_DisplayName()
{
    UE_LOG(LogTemp, Log, TEXT("Player %s display name replicated to: %s"),
        *GetPlayerName(), *DisplayName);

    // UI 업데이트 이벤트 브로드캐스트 (필요한 경우)
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, -1); // 특별한 타입으로 이름 변경 알림
    }
}