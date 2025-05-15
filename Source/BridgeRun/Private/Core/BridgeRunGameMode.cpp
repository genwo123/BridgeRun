// Copyright BridgeRun Game, Inc. All Rights Reserved.
// 코어 엔진 헤더
#include "Core/BridgeRunGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// 게임 프레임워크 헤더
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"

// 브릿지런 게임 헤더
#include "Core/BridgeRunGameInstance.h"
#include "Core/BridgeRunPlayerState.h"
#include "Core/TeamManagerComponent.h"
#include "Characters/Citizen.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // 네트워크 활성화
    bReplicates = true;

    // 기본 캐릭터 클래스 설정
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_Citizen"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // 커스텀 PlayerState 클래스 설정
    PlayerStateClass = ABridgeRunPlayerState::StaticClass();

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

    if (!NewPlayer) return;

    // PlayerState에서 팀 ID 확인
    ABridgeRunPlayerState* PS = Cast<ABridgeRunPlayerState>(NewPlayer->PlayerState);
    int32 ExistingTeamID = -1;

    // PlayerState에서 이미 설정된 팀 ID 확인
    if (PS)
    {
        ExistingTeamID = PS->GetTeamID();
    }

    // GameInstance에서도 팀 ID 확인
    if (ExistingTeamID < 0)
    {
        if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
        {
            FString PlayerID = NewPlayer->GetName();
            ExistingTeamID = GameInst->GetPlayerTeamID(PlayerID);

            // GameInstance에서 가져온 팀 ID가 있으면 PlayerState에 설정
            if (ExistingTeamID >= 0 && PS)
            {
                PS->SetTeamID(ExistingTeamID);
                UE_LOG(LogTemp, Log, TEXT("로비에서 가져온 팀 ID (%d)를 PlayerState에 설정했습니다."), ExistingTeamID);
            }
        }
    }

    // 로비에서 가져온 팀 ID가 있는 경우, TeamManagerComponent에 해당 팀으로 요청
    if (ExistingTeamID >= 0)
    {
        if (TeamManagerComponent)
        {
            // RequestTeamChange 함수는 이미 구현되어 있어서 활용
            bool bSuccess = TeamManagerComponent->RequestTeamChange(NewPlayer, ExistingTeamID);
            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("로비에서 선택한 팀 %d를 성공적으로 적용했습니다."), ExistingTeamID);
                return; // 팀 변경 성공했으니 여기서 종료
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("로비에서 선택한 팀 %d를 적용할 수 없습니다."), ExistingTeamID);
                // 실패 시 아래에서 기본 팀 할당 로직 실행
            }
        }
    }

    // 기존 팀 ID가 없거나 적용 실패한 경우에만 새로 할당
    if (TeamManagerComponent)
    {
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // 로그 출력
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
    // 팀 관리는 TeamManagerComponent에 위임
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
    SetGameTimer(JobSystemTimerHandle, &ABridgeRunGameMode::HandleJobSystemActivation, JobSystemActivationTime);

    // 상태 업데이트 알림
    UpdateGameState();
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

    // 상태 업데이트 알림
    UpdateGameState();
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
        CurrentRound++;
        GetWorld()->GetTimerManager().SetTimer(
            GameTimerHandle,
            this,
            &ABridgeRunGameMode::StartNewRound,
            PostRoundDelay,
            false
        );
    }

    // 상태 업데이트 알림
    UpdateGameState();
}

void ABridgeRunGameMode::EndGame()
{
    CurrentGameState = EGameState::GameOver;

    // 타이머 정리
    ClearGameTimer(RoundTimerHandle);
    ClearGameTimer(JobSystemTimerHandle);

    // 게임 인스턴스에서 승자 팀 가져오기
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        int32 WinningTeam = GameInst->GetWinningTeam();

        // 승자 처리 로직
        if (WinningTeam >= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! Winning Team: %d"), WinningTeam);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Game Over! No winner determined."));
        }
    }

    // 상태 업데이트 알림
    UpdateGameState();
}

void ABridgeRunGameMode::HandleRoundTimer()
{
    if (RoundTimeRemaining > 0)
    {
        RoundTimeRemaining--;

        // 남은 시간이 변경될 때만 네트워크 상태 업데이트
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

    // 팀 매니저에서 활성화된 팀 정보 가져오기
    TArray<FTeamInfo> ActiveTeams = TeamManagerComponent->GetActiveTeams();

    // 모든 팀의 플레이어 수 합산
    int32 TotalPlayers = 0;
    for (const FTeamInfo& Team : ActiveTeams)
    {
        TotalPlayers += Team.PlayerCount;
    }

    return TotalPlayers >= MinPlayersToStart;
}

void ABridgeRunGameMode::UpdateGameState()
{
    // 네트워크 상태 업데이트 강제
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
    // 먼저 기본 RestartPlayer 호출
    Super::RestartPlayer(NewPlayer);

    // 플레이어가 스폰되지 않았으면 종료
    if (!NewPlayer || !NewPlayer->GetPawn())
        return;

    // PlayerState에서 팀 ID 가져오기
    int32 TeamID = -1;
    ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(NewPlayer->PlayerState);

    if (BridgeRunPS)
    {
        TeamID = BridgeRunPS->GetTeamID();
    }

    // 유효한 팀 ID가 이미 있으면, 그대로 캐릭터에 적용
    if (TeamID >= 0)
    {
        if (ACitizen* Character = Cast<ACitizen>(NewPlayer->GetPawn()))
        {
            // 팀 ID 설정 및 머티리얼 적용
            Character->TeamID = TeamID;
            Character->MulticastSetTeamMaterial(TeamID);

            UE_LOG(LogTemp, Log, TEXT("RestartPlayer: 플레이어 %s를 팀 %d로 초기화했습니다."),
                *NewPlayer->GetName(), TeamID);

            return; // 성공적으로 적용했으니 여기서 종료
        }
    }

    // PlayerState에 팀 ID가 없는 경우 GameInstance에서 확인
    if (TeamID < 0)
    {
        if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetWorld()->GetGameInstance()))
        {
            FString PlayerID = NewPlayer->GetName();
            TeamID = GameInst->GetPlayerTeamID(PlayerID);

            if (TeamID >= 0 && BridgeRunPS)
            {
                BridgeRunPS->SetTeamID(TeamID);

                // 캐릭터에 적용
                if (ACitizen* Character = Cast<ACitizen>(NewPlayer->GetPawn()))
                {
                    Character->TeamID = TeamID;
                    Character->MulticastSetTeamMaterial(TeamID);

                    UE_LOG(LogTemp, Log, TEXT("RestartPlayer: GameInstance에서 가져온 팀 %d를 적용했습니다."), TeamID);
                    return; // 성공적으로 적용했으니 여기서 종료
                }
            }
        }
    }

    // 이전 방법으로도 팀 ID를 가져오지 못한 경우에만 TeamManagerComponent를 통해 새로 할당
    if (TeamManagerComponent)
    {
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);

        // 할당 후 로그
        if (BridgeRunPS)
        {
            UE_LOG(LogTemp, Log, TEXT("RestartPlayer: 플레이어 %s를 새로운 팀 %d에 할당했습니다."),
                *NewPlayer->GetName(), BridgeRunPS->GetTeamID());
        }
    }
}