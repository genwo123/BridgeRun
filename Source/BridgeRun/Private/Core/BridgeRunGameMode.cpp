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
    // 네트워크 활성화
    bReplicates = true;

    // 기본 클래스 설정
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // 초기 설정
    CurrentGameState = EGameState::WaitingToStart;
    CurrentRound = 0;
    bJobSystemActive = false;

    // 기본 스폰 위치 설정
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));

    // 팀 초기화
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

    // 서버 설정
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
        // 팀 배정
        AssignPlayerToTeam(NewPlayer);

        // 게임 시작 조건 체크
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

    // 승자 결정
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

    // 팀 정보 업데이트
    for (FTeamInfo& Team : TeamInfo)
    {
        if (Team.TeamID == TeamID)
        {
            Team.PlayerCount++;
            break;
        }
    }

    // 플레이어 스폰 처리
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