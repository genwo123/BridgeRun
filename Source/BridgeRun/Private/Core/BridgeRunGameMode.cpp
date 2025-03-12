// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameMode.h"
#include "Characters/Citizen.h"
#include "GameFramework/PlayerStart.h"
#include "Core/BridgeRunGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Core/TeamManagerComponent.h"
#include "Core/BridgeRunPlayerState.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // 네트워크 활성화
    bReplicates = true;

    // 기본 캐릭터 클래스 설정 (BP_Citizen으로 변경)
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_Citizen"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // 커스텀 PlayerState 클래스 설정
    PlayerStateClass = ABridgeRunPlayerState::StaticClass();

    // 초기 설정
    CurrentGameState = EGameState::WaitingToStart;
    CurrentRound = 0;
    RoundTimeRemaining = 0.0f;
    bJobSystemActive = false;

    // 기본 스폰 위치 설정
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));

    // 팀 관리 컴포넌트 생성
    TeamManagerComponent = CreateDefaultSubobject<UTeamManagerComponent>(TEXT("TeamManager"));
}

void ABridgeRunGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABridgeRunGameMode, CurrentGameState);
    DOREPLIFETIME(ABridgeRunGameMode, CurrentRound);
    DOREPLIFETIME(ABridgeRunGameMode, RoundTimeRemaining);
    DOREPLIFETIME(ABridgeRunGameMode, bJobSystemActive);
}

void ABridgeRunGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 서버 설정
    if (GetWorld()->GetNetMode() == NM_ListenServer)
    {
        GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = true;
    }
}

void ABridgeRunGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer && TeamManagerComponent)
    {
        // 팀 배정 (TeamManagerComponent 위임)
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // 게임 시작 조건 체크
        if (CanStartGame() && CurrentGameState == EGameState::WaitingToStart)
        {
            StartGame();
        }
    }
}

void ABridgeRunGameMode::Logout(AController* Exiting)
{
    // 팀 관리는 TeamManagerComponent에 위임하므로 별도 처리 없음
    Super::Logout(Exiting);
}

void ABridgeRunGameMode::StartGame()
{
    if (!CanStartGame()) return;

    CurrentGameState = EGameState::InProgress;
    CurrentRound = 1;

    // 라운드 시작
    StartNewRound();

    // 직업 시스템 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        JobSystemTimerHandle,
        this,
        &ABridgeRunGameMode::HandleJobSystemActivation,
        JobSystemActivationTime,
        false
    );
}

void ABridgeRunGameMode::StartNewRound()
{
    if (CurrentGameState != EGameState::InProgress) return;

    RoundTimeRemaining = RoundDuration;

    // 라운드 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        RoundTimerHandle,
        this,
        &ABridgeRunGameMode::HandleRoundTimer,
        1.0f,
        true
    );
}

void ABridgeRunGameMode::EndCurrentRound()
{
    if (CurrentGameState != EGameState::InProgress) return;

    CurrentGameState = EGameState::RoundEnd;
    GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);

    // 다음 라운드 준비 또는 게임 종료
    if (CurrentRound >= 3)  // 3라운드로 게임 종료
    {
        EndGame();
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(
            GameTimerHandle,
            this,
            &ABridgeRunGameMode::StartNewRound,
            PostRoundDelay,
            false
        );
        CurrentRound++;
    }
}

void ABridgeRunGameMode::EndGame()
{
    CurrentGameState = EGameState::GameOver;

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(JobSystemTimerHandle);

    // 게임 인스턴스에서 승자 팀 가져오기
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        int32 WinningTeam = GameInst->GetWinningTeam();

        // 여기에 승자 처리 로직 추가
        if (WinningTeam >= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! Winning Team: %d"), WinningTeam);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! No winner determined."));
        }
    }
}

void ABridgeRunGameMode::HandleRoundTimer()
{
    if (RoundTimeRemaining > 0)
    {
        RoundTimeRemaining--;
        if (RoundTimeRemaining <= 0)
        {
            EndCurrentRound();
        }
    }
}

void ABridgeRunGameMode::HandleJobSystemActivation()
{
    bJobSystemActive = true;
}

bool ABridgeRunGameMode::CanStartGame() const
{
    if (!TeamManagerComponent)
        return false;

    int32 TotalPlayers = 0;

    // 팀 매니저에서 활성화된 팀 정보 가져오기
    TArray<FTeamInfo> ActiveTeams = TeamManagerComponent->GetActiveTeams();

    // 모든 팀의 플레이어 수 합산
    for (const FTeamInfo& Team : ActiveTeams)
    {
        TotalPlayers += Team.PlayerCount;
    }

    return TotalPlayers >= MinPlayersToStart;
}

void ABridgeRunGameMode::UpdateGameState()
{
    ForceNetUpdate();
}