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
    // ��Ʈ��ũ Ȱ��ȭ
    bReplicates = true;

    // �⺻ ĳ���� Ŭ���� ���� (BP_Citizen���� ����)
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_Citizen"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // Ŀ���� PlayerState Ŭ���� ����
    PlayerStateClass = ABridgeRunPlayerState::StaticClass();

    // �ʱ� ����
    CurrentGameState = EGameState::WaitingToStart;
    CurrentRound = 0;
    RoundTimeRemaining = 0.0f;
    bJobSystemActive = false;

    // �⺻ ���� ��ġ ����
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));

    // �� ���� ������Ʈ ����
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

    // ���� ����
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
        // �� ���� (TeamManagerComponent ����)
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // ���� ���� ���� üũ
        if (CanStartGame() && CurrentGameState == EGameState::WaitingToStart)
        {
            StartGame();
        }
    }
}

void ABridgeRunGameMode::Logout(AController* Exiting)
{
    // �� ������ TeamManagerComponent�� �����ϹǷ� ���� ó�� ����
    Super::Logout(Exiting);
}

void ABridgeRunGameMode::StartGame()
{
    if (!CanStartGame()) return;

    CurrentGameState = EGameState::InProgress;
    CurrentRound = 1;

    // ���� ����
    StartNewRound();

    // ���� �ý��� Ÿ�̸� ����
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

    // ���� Ÿ�̸� ����
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

    // ���� ���� �غ� �Ǵ� ���� ����
    if (CurrentRound >= 3)  // 3����� ���� ����
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

    // Ÿ�̸� ����
    GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(JobSystemTimerHandle);

    // ���� �ν��Ͻ����� ���� �� ��������
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        int32 WinningTeam = GameInst->GetWinningTeam();

        // ���⿡ ���� ó�� ���� �߰�
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

    // �� �Ŵ������� Ȱ��ȭ�� �� ���� ��������
    TArray<FTeamInfo> ActiveTeams = TeamManagerComponent->GetActiveTeams();

    // ��� ���� �÷��̾� �� �ջ�
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