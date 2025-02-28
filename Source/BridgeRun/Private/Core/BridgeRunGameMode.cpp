// Private/Core/BridgeRunGameMode.cpp
#include "Core/BridgeRunGameMode.h"
#include "Characters/Citizen.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // ��Ʈ��ũ Ȱ��ȭ
    bReplicates = true;

    // �⺻ Ŭ���� ����
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // �ʱ� ����
    CurrentGameState = EGameState::WaitingToStart;
    CurrentRound = 0;
    bJobSystemActive = false;

    // �⺻ ���� ��ġ ����
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));

    // �� �ʱ�ȭ
    for (int32 i = 0; i < MaxTeams; i++)
    {
        FTeamInfo NewTeam;
        NewTeam.TeamID = i;
        TeamInfo.Add(NewTeam);
    }
}

void ABridgeRunGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABridgeRunGameMode, CurrentGameState);
    DOREPLIFETIME(ABridgeRunGameMode, CurrentRound);
    DOREPLIFETIME(ABridgeRunGameMode, RoundTimeRemaining);
    DOREPLIFETIME(ABridgeRunGameMode, bJobSystemActive);
    DOREPLIFETIME(ABridgeRunGameMode, TeamInfo);
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

    if (NewPlayer)
    {
        // �� ����
        AssignPlayerToTeam(NewPlayer);

        // ���� ���� ���� üũ
        if (CanStartGame() && CurrentGameState == EGameState::WaitingToStart)
        {
            StartGame();
        }
    }
}

void ABridgeRunGameMode::Logout(AController* Exiting)
{
    if (APlayerState* PS = Exiting->PlayerState)
    {
        for (FTeamInfo& Team : TeamInfo)
        {
            if (Team.PlayerCount > 0)
            {
                Team.PlayerCount--;
            }
        }
    }

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

    // ���� ����
    int32 WinningTeam = -1;
    int32 HighestScore = -1;

    for (const FTeamInfo& Team : TeamInfo)
    {
        if (Team.Score > HighestScore)
        {
            HighestScore = Team.Score;
            WinningTeam = Team.TeamID;
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

void ABridgeRunGameMode::AssignPlayerToTeam(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;

    int32 TeamID = GetOptimalTeamForTeam();

    // �� ���� ������Ʈ
    for (FTeamInfo& Team : TeamInfo)
    {
        if (Team.TeamID == TeamID)
        {
            Team.PlayerCount++;
            break;
        }
    }

    // �÷��̾� ���� ó��
    AActor* StartSpot = FindPlayerStart(NewPlayer, FString::FromInt(TeamID));
    if (StartSpot)
    {
        if (NewPlayer->GetPawn())
        {
            NewPlayer->GetPawn()->Destroy();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        if (APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
            DefaultPawnClass,
            StartSpot->GetActorLocation(),
            StartSpot->GetActorRotation(),
            SpawnParams))
        {
            NewPlayer->Possess(NewPawn);
        }
    }
}

int32 ABridgeRunGameMode::GetOptimalTeamForTeam() const
{
    int32 OptimalTeam = 0;
    int32 MinPlayers = MAX_int32;

    for (const FTeamInfo& Team : TeamInfo)
    {
        if (Team.PlayerCount < MinPlayers)
        {
            MinPlayers = Team.PlayerCount;
            OptimalTeam = Team.TeamID;
        }
    }

    return OptimalTeam;
}

void ABridgeRunGameMode::AddTeamScore(int32 TeamID, int32 Score)
{
    if (CurrentGameState != EGameState::InProgress) return;

    for (FTeamInfo& Team : TeamInfo)
    {
        if (Team.TeamID == TeamID)
        {
            Team.Score += Score;
            break;
        }
    }
}

int32 ABridgeRunGameMode::GetTeamScore(int32 TeamID) const
{
    for (const FTeamInfo& Team : TeamInfo)
    {
        if (Team.TeamID == TeamID)
        {
            return Team.Score;
        }
    }
    return 0;
}

bool ABridgeRunGameMode::CanStartGame() const
{
    int32 TotalPlayers = 0;
    for (const FTeamInfo& Team : TeamInfo)
    {
        TotalPlayers += Team.PlayerCount;
    }

    return TotalPlayers >= MinPlayersToStart;
}

void ABridgeRunGameMode::UpdateGameState()
{
    ForceNetUpdate();
}