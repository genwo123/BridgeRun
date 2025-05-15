// Copyright BridgeRun Game, Inc. All Rights Reserved.
// �ھ� ���� ���
#include "Core/BridgeRunGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// ���� �����ӿ�ũ ���
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"

// �긴���� ���� ���
#include "Core/BridgeRunGameInstance.h"
#include "Core/BridgeRunPlayerState.h"
#include "Core/TeamManagerComponent.h"
#include "Characters/Citizen.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // ��Ʈ��ũ Ȱ��ȭ
    bReplicates = true;

    // �⺻ ĳ���� Ŭ���� ����
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_Citizen"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // Ŀ���� PlayerState Ŭ���� ����
    PlayerStateClass = ABridgeRunPlayerState::StaticClass();

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

    if (!NewPlayer) return;

    // PlayerState���� �� ID Ȯ��
    ABridgeRunPlayerState* PS = Cast<ABridgeRunPlayerState>(NewPlayer->PlayerState);
    int32 ExistingTeamID = -1;

    // PlayerState���� �̹� ������ �� ID Ȯ��
    if (PS)
    {
        ExistingTeamID = PS->GetTeamID();
    }

    // GameInstance������ �� ID Ȯ��
    if (ExistingTeamID < 0)
    {
        if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
        {
            FString PlayerID = NewPlayer->GetName();
            ExistingTeamID = GameInst->GetPlayerTeamID(PlayerID);

            // GameInstance���� ������ �� ID�� ������ PlayerState�� ����
            if (ExistingTeamID >= 0 && PS)
            {
                PS->SetTeamID(ExistingTeamID);
                UE_LOG(LogTemp, Log, TEXT("�κ񿡼� ������ �� ID (%d)�� PlayerState�� �����߽��ϴ�."), ExistingTeamID);
            }
        }
    }

    // �κ񿡼� ������ �� ID�� �ִ� ���, TeamManagerComponent�� �ش� ������ ��û
    if (ExistingTeamID >= 0)
    {
        if (TeamManagerComponent)
        {
            // RequestTeamChange �Լ��� �̹� �����Ǿ� �־ Ȱ��
            bool bSuccess = TeamManagerComponent->RequestTeamChange(NewPlayer, ExistingTeamID);
            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("�κ񿡼� ������ �� %d�� ���������� �����߽��ϴ�."), ExistingTeamID);
                return; // �� ���� ���������� ���⼭ ����
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("�κ񿡼� ������ �� %d�� ������ �� �����ϴ�."), ExistingTeamID);
                // ���� �� �Ʒ����� �⺻ �� �Ҵ� ���� ����
            }
        }
    }

    // ���� �� ID�� ���ų� ���� ������ ��쿡�� ���� �Ҵ�
    if (TeamManagerComponent)
    {
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // �α� ���
        if (PS)
        {
            UE_LOG(LogTemp, Log, TEXT("PostLogin: Player %s assigned to team %d"),
                *NewPlayer->GetName(), PS->GetTeamID());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: TeamManagerComponent not found!"));
    }
}

void ABridgeRunGameMode::Logout(AController* Exiting)
{
    // �� ������ TeamManagerComponent�� ����
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
    SetGameTimer(JobSystemTimerHandle, &ABridgeRunGameMode::HandleJobSystemActivation, JobSystemActivationTime);

    // ���� ������Ʈ �˸�
    UpdateGameState();
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

    // ���� ������Ʈ �˸�
    UpdateGameState();
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
        CurrentRound++;
        GetWorld()->GetTimerManager().SetTimer(
            GameTimerHandle,
            this,
            &ABridgeRunGameMode::StartNewRound,
            PostRoundDelay,
            false
        );
    }

    // ���� ������Ʈ �˸�
    UpdateGameState();
}

void ABridgeRunGameMode::EndGame()
{
    CurrentGameState = EGameState::GameOver;

    // Ÿ�̸� ����
    ClearGameTimer(RoundTimerHandle);
    ClearGameTimer(JobSystemTimerHandle);

    // ���� �ν��Ͻ����� ���� �� ��������
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        int32 WinningTeam = GameInst->GetWinningTeam();

        // ���� ó�� ����
        if (WinningTeam >= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! Winning Team: %d"), WinningTeam);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! No winner determined."));
        }
    }

    // ���� ������Ʈ �˸�
    UpdateGameState();
}

void ABridgeRunGameMode::HandleRoundTimer()
{
    if (RoundTimeRemaining > 0)
    {
        RoundTimeRemaining--;

        // ���� �ð��� ����� ���� ��Ʈ��ũ ���� ������Ʈ
        UpdateGameState();

        if (RoundTimeRemaining <= 0)
        {
            EndCurrentRound();
        }
    }
}

void ABridgeRunGameMode::HandleJobSystemActivation()
{
    bJobSystemActive = true;
    UpdateGameState();
}

bool ABridgeRunGameMode::CanStartGame() const
{
    if (!TeamManagerComponent)
        return false;

    // �� �Ŵ������� Ȱ��ȭ�� �� ���� ��������
    TArray<FTeamInfo> ActiveTeams = TeamManagerComponent->GetActiveTeams();

    // ��� ���� �÷��̾� �� �ջ�
    int32 TotalPlayers = 0;
    for (const FTeamInfo& Team : ActiveTeams)
    {
        TotalPlayers += Team.PlayerCount;
    }

    return TotalPlayers >= MinPlayersToStart;
}

void ABridgeRunGameMode::UpdateGameState()
{
    // ��Ʈ��ũ ���� ������Ʈ ����
    ForceNetUpdate();
}

void ABridgeRunGameMode::SetGameTimer(FTimerHandle& TimerHandle, void (ABridgeRunGameMode::* Function)(), float Delay, bool bLooping)
{
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        this,
        Function,
        Delay,
        bLooping
    );
}

void ABridgeRunGameMode::ClearGameTimer(FTimerHandle& TimerHandle)
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}


void ABridgeRunGameMode::RestartPlayer(AController* NewPlayer)
{
    // ���� �⺻ RestartPlayer ȣ��
    Super::RestartPlayer(NewPlayer);

    // �÷��̾ �������� �ʾ����� ����
    if (!NewPlayer || !NewPlayer->GetPawn())
        return;

    // PlayerState���� �� ID ��������
    int32 TeamID = -1;
    ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(NewPlayer->PlayerState);

    if (BridgeRunPS)
    {
        TeamID = BridgeRunPS->GetTeamID();
    }

    // ��ȿ�� �� ID�� �̹� ������, �״�� ĳ���Ϳ� ����
    if (TeamID >= 0)
    {
        if (ACitizen* Character = Cast<ACitizen>(NewPlayer->GetPawn()))
        {
            // �� ID ���� �� ��Ƽ���� ����
            Character->TeamID = TeamID;
            Character->MulticastSetTeamMaterial(TeamID);

            UE_LOG(LogTemp, Log, TEXT("RestartPlayer: �÷��̾� %s�� �� %d�� �ʱ�ȭ�߽��ϴ�."),
                *NewPlayer->GetName(), TeamID);

            return; // ���������� ���������� ���⼭ ����
        }
    }

    // PlayerState�� �� ID�� ���� ��� GameInstance���� Ȯ��
    if (TeamID < 0)
    {
        if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetWorld()->GetGameInstance()))
        {
            FString PlayerID = NewPlayer->GetName();
            TeamID = GameInst->GetPlayerTeamID(PlayerID);

            if (TeamID >= 0 && BridgeRunPS)
            {
                BridgeRunPS->SetTeamID(TeamID);

                // ĳ���Ϳ� ����
                if (ACitizen* Character = Cast<ACitizen>(NewPlayer->GetPawn()))
                {
                    Character->TeamID = TeamID;
                    Character->MulticastSetTeamMaterial(TeamID);

                    UE_LOG(LogTemp, Log, TEXT("RestartPlayer: GameInstance���� ������ �� %d�� �����߽��ϴ�."), TeamID);
                    return; // ���������� ���������� ���⼭ ����
                }
            }
        }
    }

    // ���� ������ε� �� ID�� �������� ���� ��쿡�� TeamManagerComponent�� ���� ���� �Ҵ�
    if (TeamManagerComponent)
    {
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // �Ҵ� �� �α�
        if (BridgeRunPS)
        {
            UE_LOG(LogTemp, Log, TEXT("RestartPlayer: �÷��̾� %s�� ���ο� �� %d�� �Ҵ��߽��ϴ�."),
                *NewPlayer->GetName(), BridgeRunPS->GetTeamID());
        }
    }
}