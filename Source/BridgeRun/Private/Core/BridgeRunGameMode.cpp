// Copyright BridgeRun Game, Inc. All Rights Reserved.
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
#include "Core/BridgeRunGameState.h"
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

    // 기본 3라운드 설정 (각각 3분)
    RoundSettingsArray.Add(FRoundSettings(180.0f)); // 라운드 1: 3분
    RoundSettingsArray.Add(FRoundSettings(180.0f)); // 라운드 2: 3분  
    RoundSettingsArray.Add(FRoundSettings(180.0f)); // 라운드 3: 3분
}

void ABridgeRunGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 기존 복제 속성들은 제거하고 새로운 시스템은 GameState에서 관리
}

// BridgeRunGameMode.cpp - InitializeActiveTeams 함수 수정

void ABridgeRunGameMode::InitializeActiveTeams()
{
    if (!HasAuthority()) return;

    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState)
    {
        UE_LOG(LogTemp, Error, TEXT("GameState is null in InitializeActiveTeams"));
        return;
    }

    // ★ 강제 팀 할당 제거 - PlayerState에서 실제 사용 중인 팀만 활성화 ★
    TArray<int32> ActiveTeamIDs;
    TSet<int32> UsedTeams; // 중복 제거용

    // 모든 플레이어의 PlayerState에서 팀 ID 수집
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->PlayerState)
        {
            ABridgeRunPlayerState* BRPlayerState = Cast<ABridgeRunPlayerState>(PC->PlayerState);
            if (BRPlayerState)
            {
                int32 TeamID = BRPlayerState->GetTeamID();
                if (TeamID >= 0 && TeamID < 4) // 유효한 팀 ID
                {
                    UsedTeams.Add(TeamID);
                    UE_LOG(LogTemp, Warning, TEXT("Player %s using TeamID %d"),
                        *PC->GetName(), TeamID);
                }
            }
        }
    }

    // Set을 Array로 변환
    ActiveTeamIDs = UsedTeams.Array();

    // ★ 최소 2팀 보장 (실제 사용 중인 팀이 없을 때만) ★
    if (ActiveTeamIDs.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("No teams in use by players, using default teams"));
        ActiveTeamIDs.Empty();
        ActiveTeamIDs.Add(0);  // 빨강팀
        ActiveTeamIDs.Add(1);  // 파랑팀
    }

    // 팀 ID 정렬 (일관성을 위해)
    ActiveTeamIDs.Sort();

    // GameState에 팀 초기화
    BRGameState->InitializeTeams(ActiveTeamIDs);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d teams based on player selections"), ActiveTeamIDs.Num());

    // 활성화된 팀 로그 출력
    for (int32 TeamID : ActiveTeamIDs)
    {
        UE_LOG(LogTemp, Log, TEXT("Active Team: %d (%s)"), TeamID, *BRGameState->GetTeamName(TeamID));
    }
}

void ABridgeRunGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 서버 설정
    if (GetWorld()->GetNetMode() == NM_ListenServer)
    {
        GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = true;
    }

    // 1초 후 팀 초기화
    FTimerHandle TeamInitHandle;
    GetWorld()->GetTimerManager().SetTimer(TeamInitHandle, this, &ABridgeRunGameMode::InitializeActiveTeams, 1.0f, false);

    // 2초 후 게임 시작
    FTimerHandle DelayHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &ABridgeRunGameMode::StartStrategyPhase, 2.0f, false);
}

// PostLogin에서 로그 추가 (BridgeRunGameMode.cpp)

void ABridgeRunGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer) return;

    UE_LOG(LogTemp, Error, TEXT("========== PostLogin START: %s =========="), *NewPlayer->GetName());

    // PlayerState 확인
    ABridgeRunPlayerState* PS = Cast<ABridgeRunPlayerState>(NewPlayer->PlayerState);
    if (PS)
    {
        int32 TeamID = PS->GetTeamID();
    }

    // GameInstance 확인
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        FString PlayerID = NewPlayer->GetName();
        int32 GameInstTeamID = GameInst->GetPlayerTeamIDForTransition(PlayerID);
    }

    // TeamManager 호출 여부 확인
    if (TeamManagerComponent)
    {
        TeamManagerComponent->AssignPlayerToTeam(NewPlayer);
        UE_LOG(LogTemp, Error, TEXT("PostLogin: AssignPlayerToTeam completed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: TeamManagerComponent is NULL!"));
    }

    UE_LOG(LogTemp, Error, TEXT("========== PostLogin END =========="));
}

void ABridgeRunGameMode::Logout(AController* Exiting)
{
    // 팀 관리는 TeamManagerComponent에 위임
    Super::Logout(Exiting);
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
            TeamID = GameInst->GetPlayerTeamIDForTransition(PlayerID);

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
// === 라운드 시스템 함수들 ===

float ABridgeRunGameMode::GetRoundPlayTime(int32 RoundNumber) const
{
    int32 Index = RoundNumber - 1; // 1-based -> 0-based
    if (RoundSettingsArray.IsValidIndex(Index))
    {
        return RoundSettingsArray[Index].PlayTime;
    }
    return 180.0f; // 기본값
}

float ABridgeRunGameMode::GetStrategyTime(int32 RoundNumber) const
{
    return (RoundNumber == 1) ? FirstStrategyTime : OtherStrategyTime;
}

void ABridgeRunGameMode::StartStrategyPhase()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    // 현재 라운드에 맞는 전략 시간 설정
    float StrategyTime = GetStrategyTime(BRGameState->GetCurrentRoundNumber());

    // 상태 업데이트
    BRGameState->SetCurrentPhase(EGamePhase::StrategyTime);
    BRGameState->SetPhaseTimeRemaining(StrategyTime);

    // 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, this, &ABridgeRunGameMode::UpdatePhaseTimer, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("Strategy Phase Started - Round %d, Time: %.0f"),
        BRGameState->GetCurrentRoundNumber(), StrategyTime);
}

void ABridgeRunGameMode::StartRoundPlaying()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    // 현재 라운드에 맞는 플레이 시간 설정
    float PlayTime = GetRoundPlayTime(BRGameState->GetCurrentRoundNumber());

    // 상태 업데이트
    BRGameState->SetCurrentPhase(EGamePhase::RoundPlaying);
    BRGameState->SetPhaseTimeRemaining(PlayTime);

    // ★ 라운드 시작 이벤트 호출 (이 한 줄만 추가!) ★
    OnRoundStart();

    GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, this, &ABridgeRunGameMode::UpdatePhaseTimer, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("Round Playing Started - Round %d"), BRGameState->GetCurrentRoundNumber());
}


void ABridgeRunGameMode::EndRound()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    // 현재 라운드 순위 계산
    CalculateRoundRankings();

    // 라운드 종료 상태로 변경
    BRGameState->SetCurrentPhase(EGamePhase::RoundEnd);
    BRGameState->SetPhaseTimeRemaining(RoundEndWaitTime);

    // 게임 일시정지 및 UI 표시
    ShowRoundEndResults();

    // 타이머 재시작 (UI 표시 시간)
    GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, this, &ABridgeRunGameMode::UpdatePhaseTimer, 1.0f, true);

}


void ABridgeRunGameMode::ShowRoundEndResults()
{
    // 모든 플레이어에게 UI 표시 및 마우스 활성화
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC)
        {
            // 마우스 커서 표시
            PC->bShowMouseCursor = true;
            PC->bEnableClickEvents = true;
            PC->bEnableMouseOverEvents = true;

            // 입력 모드를 UI로 변경
            FInputModeUIOnly InputMode;
            PC->SetInputMode(InputMode);

            UE_LOG(LogTemp, Log, TEXT("Enabled mouse cursor for player: %s"), *PC->GetName());
        }
    }

    // 블루프린트 이벤트 호출 (UI 위젯 생성)
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (BRGameState)
    {
        if (BRGameState->GetCurrentRoundNumber() >= GetMaxRounds())
        {
            GameOverUI(); // 최종 게임 종료 UI
        }
        else
        {
            OnRoundEndUI(BRGameState->GetCurrentRoundNumber()); // 라운드 종료 UI
        }
    }
}

void ABridgeRunGameMode::HideRoundEndResults()
{
    // 모든 플레이어의 마우스 비활성화 및 게임 입력 모드로 복원
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC)
        {
            // 마우스 커서 숨김
            PC->bShowMouseCursor = false;
            PC->bEnableClickEvents = false;
            PC->bEnableMouseOverEvents = false;

            // 입력 모드를 게임으로 변경
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);

            UE_LOG(LogTemp, Log, TEXT("Disabled mouse cursor for player: %s"), *PC->GetName());
        }
    }
}

// 라운드 순위 계산 함수
void ABridgeRunGameMode::CalculateRoundRankings()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    TArray<FTeamVictoryData>& Teams = BRGameState->TeamVictoryPoints;

    // 현재 라운드 점수 순으로 정렬
    TArray<FTeamVictoryData> SortedTeams = Teams;
    SortedTeams.Sort([](const FTeamVictoryData& A, const FTeamVictoryData& B) {
        return A.CurrentRoundScore > B.CurrentRoundScore; // 점수 높은 순
        });

    // 각 팀의 라운드 결과(순위) 업데이트 및 승점 계산
    for (int32 Rank = 0; Rank < SortedTeams.Num(); Rank++)
    {
        int32 TeamID = SortedTeams[Rank].TeamID;

        for (FTeamVictoryData& Team : Teams)
        {
            if (Team.TeamID == TeamID)
            {
                Team.RoundResults.Add(Rank + 1); // 1등, 2등, 3등...

                // 승점 계산
                int32 VictoryPoints = 0;
                switch (Rank + 1)
                {
                case 1: VictoryPoints = 5; break;
                case 2: VictoryPoints = 2; break;
                case 3: VictoryPoints = -1; break;
                case 4: VictoryPoints = -2; break;
                default: VictoryPoints = -2; break;
                }

                Team.TotalVictoryPoints += VictoryPoints;

                UE_LOG(LogTemp, Log, TEXT("Team %d - Round %d: %d등 (%d점), 승점: %d, 총 승점: %d"),
                    TeamID, BRGameState->GetCurrentRoundNumber(), Rank + 1,
                    Team.CurrentRoundScore, VictoryPoints, Team.TotalVictoryPoints);
                break;
            }
        }
    }
}


// 기존 OnPhaseTimeEnd() 함수를 이렇게 수정하세요
void ABridgeRunGameMode::OnPhaseTimeEnd()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);

    switch (BRGameState->GetCurrentPhase())
    {
    case EGamePhase::StrategyTime:
        StartRoundPlaying();
        break;

    case EGamePhase::RoundPlaying:
        EndRound(); // 여기서 UI가 표시됨
        break;

    case EGamePhase::RoundEnd:
        // ★ UI 숨김 추가 ★
        HideRoundEndResults();

        if (BRGameState->GetCurrentRoundNumber() >= GetMaxRounds())
        {
            EndGame(); // 최종 게임 종료
        }
        else
        {
            // 다음 라운드로
            BRGameState->SetCurrentRoundNumber(BRGameState->GetCurrentRoundNumber() + 1);
            StartStrategyPhase();
        }
        break;

    case EGamePhase::GameEnd:
        // 게임 완전 종료 처리
        UE_LOG(LogTemp, Log, TEXT("Game completely finished"));
        break;
    }
}


void ABridgeRunGameMode::EndGame()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    // 게임 종료 상태로 변경
    BRGameState->SetCurrentPhase(EGamePhase::GameEnd);
    BRGameState->SetPhaseTimeRemaining(0.0f);

    // ★ GameState를 통해 Multicast 호출
    BRGameState->MulticastGameOverUI();

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
}


void ABridgeRunGameMode::UpdatePhaseTimer()
{
    ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>();
    if (!BRGameState) return;

    float CurrentTime = BRGameState->GetPhaseTimeRemaining();

    // ★ 이 로그로 값 확인 ★
    UE_LOG(LogTemp, Warning, TEXT("time: %.1f, Round : %d"),
        CurrentTime, (int32)BRGameState->GetCurrentPhase());

    CurrentTime -= 1.0f;
    BRGameState->SetPhaseTimeRemaining(CurrentTime);

    if (CurrentTime <= 0.0f)
    {
        OnPhaseTimeEnd();
    }
}

void ABridgeRunGameMode::StartGameForAllPlayers(const TArray<int32>& TeamCounts)
{
    if (HasAuthority())
    {
        if (ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>())
        {
            BRGameState->StartGameWithTeams(TeamCounts);
        }
    }
}

void ABridgeRunGameMode::ServerStartGame_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("ServerStartGame called!"));

    InitializeActiveTeams();

    if (ABridgeRunGameState* BRGameState = GetGameState<ABridgeRunGameState>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting bGameStarted = true"));
        BRGameState->bGameStarted = true;
    }
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

