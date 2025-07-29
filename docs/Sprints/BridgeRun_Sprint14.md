# 브릿지런 개발일지 (스프린트 14)

## 📅 개발 기간
2025년 5월 19일 ~ 2025년 6월 1일 (2주)

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 14에서는 공모전 데모를 위한 완성도 높은 게임플레이 구현에 집중했습니다:
- 2분 데모에 최적화된 게임 플로우 완성
- 경쟁적 게임플레이 요소 강화  
- 명확한 게임 시작/종료 시스템 구축
- 동적 팀 UI 시스템 구현

## 2. 동적 팀 UI 시스템

### 2.1 가변적 팀 표시 시스템

기존 고정된 4팀 UI에서 실제 활성화된 팀 수에 따라 동적으로 변화하는 UI를 구현했습니다.

![동적 팀 UI 시스템](../Images/Sprints_img/sprint14/dynamic_team_ui.jpg)
*활성화된 팀 수에 따라 자동으로 조정되는 UI*

### 2.2 팀 감지 시스템 구조도
```
팀 UI 동적 생성 플로우:
[로비 팀 검사] → [활성 팀 카운트] → [UI 레이아웃 계산] → [위젯 생성] → [실시간 업데이트]
       ↓              ↓                ↓               ↓             ↓
   플레이어 배정     2-4팀 가변       위치/크기 조정    동적 생성     점수 동기화
```

### 2.3 핵심 구현 코드
```cpp
// 활성화된 팀 자동 감지 및 UI 생성
void UGameHUD::UpdateTeamDisplay_Implementation()
{
    TArray<int32> ActiveTeams;
    
    // 게임 인스턴스에서 활성 팀 수집
    if (UBridgeRunGameInstance* GameInstance = GetGameInstance<UBridgeRunGameInstance>())
    {
        for (const FTeamInfo& Team : GameInstance->AvailableTeams)
        {
            if (Team.PlayerNames.Num() > 0)
            {
                ActiveTeams.Add(Team.TeamId);
            }
        }
    }
    
    // UI 위젯 동적 생성
    CreateTeamWidgets(ActiveTeams);
    AdjustUILayout(ActiveTeams.Num());
}

// 팀별 점수 실시간 업데이트
void UGameHUD::UpdateTeamScore(int32 TeamId, int32 NewScore)
{
    if (TeamScoreWidgets.Contains(TeamId))
    {
        TeamScoreWidgets[TeamId]->SetScore(NewScore);
        TeamScoreWidgets[TeamId]->UpdateVisualEffects();
    }
}
```

![팀 점수 UI](../Images/Sprints_img/sprint14/team_score_realtime.gif)
*실시간으로 업데이트되는 팀별 점수 표시*

### 2.4 발생한 문제와 임시 해결책

**팀 텍스트 컬러 적용 실패:**
- **문제**: 동적 생성된 UI 위젯에서 팀 색상이 제대로 매핑되지 않음
- **원인**: 런타임 머티리얼 생성과 위젯 생성 타이밍 불일치
- **임시 해결**: 데모용 4팀 고정 UI로 대체하여 안정성 확보

![팀 컬러 문제](../Images/Sprints_img/sprint14/team_color_issue.jpg)
*팀 색상 매핑 문제와 임시 해결방안*

## 3. 게임 플로우 시스템

### 3.1 준비단계 이동 제한 시스템

공정한 게임 시작을 위한 준비 시간 동안의 이동 제한 벽 시스템을 구현했습니다.

![준비단계 벽 시스템](../Images/Sprints_img/sprint14/preparation_walls.jpg)
*준비 시간 동안 팀별 스폰 영역을 분리하는 벽*

### 3.2 게임 시작 플로우
```
라운드 시작 시퀀스:
[게임 시작] → [준비 시간] → [벽 활성화] → [카운트다운] → [벽 제거] → [게임 진행]
     ↓           ↓           ↓             ↓            ↓            ↓
  팀별 스폰    30초 대기    이동 제한      10-3-2-1      자유 이동    토템 쟁탈
```

### 3.3 벽 시스템 구현
```cpp
// 준비단계 벽 관리 시스템
UCLASS()
class BRIDGERUN_API APreparationWallManager : public AActor
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, Category = "Walls")
    TArray<class AStaticMeshActor*> PreparationWalls;
    
    UFUNCTION(Server, Reliable)
    void ActivatePreparationWalls();
    
    UFUNCTION(Server, Reliable)
    void DeactivatePreparationWalls();
    
    UFUNCTION(NetMulticast, Reliable)
    void MulticastWallStateChange(bool bActivate);
};

// 준비 시간 종료 시 벽 제거
void AGameModeBase::OnPreparationTimeEnd()
{
    if (PreparationWallManager)
    {
        PreparationWallManager->DeactivatePreparationWalls();
    }
    
    // 게임 시작 알림
    BroadcastGameStart();
}
```

### 3.4 라운드 시스템 구현

2분 데모를 위한 최적화된 라운드 시스템을 구현했습니다.

![라운드 타이머](../Images/Sprints_img/sprint14/round_timer_system.jpg)
*명확한 라운드 진행 상황 표시*

```cpp
// 2분 데모용 라운드 관리
void ABridgeRunGameMode::StartDemoRound()
{
    const float DemoRoundDuration = 120.0f; // 2분
    
    // 라운드 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        RoundTimerHandle,
        this,
        &ABridgeRunGameMode::EndDemoRound,
        DemoRoundDuration,
        false
    );
    
    // UI 업데이트
    BroadcastRoundStart(DemoRoundDuration);
}
```

## 4. 게임 종료 시스템

### 4.1 팀별 순위 및 성과 표시

게임 종료 시 명확한 승부 결과를 보여주는 시스템을 구현했습니다.

![게임 결과 화면](../Images/Sprints_img/sprint14/game_results_screen.jpg)
*팀별 순위와 개인 성과를 보여주는 결과 화면*

### 4.2 결과 처리 플로우
```
게임 종료 프로세스:
[시간 종료] → [점수 집계] → [순위 계산] → [결과 생성] → [UI 표시] → [로비 복귀]
     ↓           ↓           ↓           ↓           ↓           ↓
  타이머 만료   최종 점수   등수 산정   데이터 정리   결과 화면   재시작 준비
```

### 4.3 순위 계산 시스템
```cpp
// 팀별 최종 순위 계산
struct FTeamRanking
{
    int32 TeamId;
    int32 FinalScore;
    int32 Rank;
    TArray<FPlayerStats> PlayerStats;
};

void ABridgeRunGameMode::CalculateFinalRankings()
{
    TArray<FTeamRanking> Rankings;
    
    // 팀별 점수 수집
    for (const auto& TeamPair : TeamScores)
    {
        FTeamRanking Ranking;
        Ranking.TeamId = TeamPair.Key;
        Ranking.FinalScore = TeamPair.Value;
        
        // 개별 플레이어 성과 수집
        CollectPlayerStats(Ranking.TeamId, Ranking.PlayerStats);
        Rankings.Add(Ranking);
    }
    
    // 점수순 정렬
    Rankings.Sort([](const FTeamRanking& A, const FTeamRanking& B) {
        return A.FinalScore > B.FinalScore;
    });
    
    // 순위 배정
    for (int32 i = 0; i < Rankings.Num(); i++)
    {
        Rankings[i].Rank = i + 1;
    }
    
    // 결과 브로드캐스트
    BroadcastGameResults(Rankings);
}
```

### 4.4 성과 정보 시스템

플레이어별 상세한 성과 정보를 수집하고 표시합니다.

```cpp
// 플레이어 개별 성과 추적
USTRUCT(BlueprintType)
struct FPlayerStats
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;
    
    UPROPERTY(BlueprintReadWrite)
    int32 PlanksBuild;      // 건설한 판자 수
    
    UPROPERTY(BlueprintReadWrite)
    int32 ShotsHit;         // 명중한 공격 수
    
    UPROPERTY(BlueprintReadWrite)
    float TrophyHoldTime;   // 트로피 보유 시간
    
    UPROPERTY(BlueprintReadWrite)
    int32 TeamContribution; // 팀 기여도 점수
};
```

**⚠️ 성능 문제로 인한 임시 제거:**
과도한 로그 데이터 수집으로 인한 성능 저하가 발생하여, 데모 버전에서는 게임 종료 화면을 임시 제거하고 간단한 승리 메시지로 대체했습니다.

## 5. 전투 시스템 고도화

### 5.1 차별화된 피격 반응 시스템

트로피 보유 여부에 따른 서로 다른 피격 반응을 구현했습니다.

![피격 시스템 비교](../Images/Sprints_img/sprint14/hit_system_comparison.jpg)
*트로피 보유 시 vs 일반 상태의 피격 반응 차이*

### 5.2 피격 반응 로직
```
피격 시스템 분기:
[공격 적중] → [대상 상태 확인] → [반응 타입 결정] → [효과 적용] → [네트워크 동기화]
     ↓              ↓                ↓              ↓              ↓
  Hit Event    트로피 보유 여부    드랍 vs 넉백    즉시 실행      모든 클라이언트
```

### 5.3 구현 코드
```cpp
// 차별화된 피격 처리 시스템
void UCombatComponent::ProcessHitEffect_Implementation(AActor* HitActor)
{
    if (!HitActor) return;
    
    // 캐릭터 컴포넌트 확인
    if (UInvenComponent* HitInvenComp = HitActor->FindComponentByClass<UInvenComponent>())
    {
        // 트로피 보유 여부 확인
        if (HitInvenComp->HasTrophy())
        {
            // 트로피 강제 드랍
            HitInvenComp->ForceDrop(EInventorySlot::Trophy);
            ApplyTrophyDropEffect(HitActor);
        }
        else
        {
            // 일반 넉백 효과
            ApplyKnockbackEffect(HitActor);
        }
    }
}

// 트로피 드랍 효과
void UCombatComponent::ApplyTrophyDropEffect(AActor* Target)
{
    // 시각적 효과
    SpawnTrophyDropParticle(Target->GetActorLocation());
    
    // 사운드 효과
    PlayTrophyDropSound();
    
    // UI 알림
    BroadcastTrophyDropped(Target);
}

// 넉백 효과
void UCombatComponent::ApplyKnockbackEffect(AActor* Target)
{
    if (UPrimitiveComponent* TargetComp = Target->FindComponentByClass<UPrimitiveComponent>())
    {
        FVector KnockbackDirection = (Target->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
        TargetComp->AddImpulse(KnockbackDirection * KnockbackForce, NAME_None, true);
    }
}
```

![트로피 드랍 효과](../Images/Sprints_img/sprint14/trophy_drop_effect.gif)
*트로피 보유자 피격 시 드랍 효과*

## 6. 맵 개선 및 최적화

### 6.1 총 스폰존 중앙 배치

게임 밸런싱 강화를 위해 총 스폰존을 맵 중앙으로 이동했습니다.

![총 스폰존 재배치](../Images/Sprints_img/sprint14/gun_spawn_rebalancing.jpg)
*밸런싱을 위한 총 스폰존 중앙 배치*

### 6.2 밸런싱 개선 효과
```
밸런싱 개선 결과:

Before (기존):
- 특정 팀이 총 독점 가능
- 불공정한 전투 상황 발생
- 게임 재미 저하

After (개선):
- 모든 팀이 동등한 접근 기회
- 치열한 총 쟁탈전 유도
- 전략적 재미 증가
```

### 6.3 BGM 시스템 적용

맵에 배경음악 시스템을 적용하여 몰입감을 향상시켰습니다.

```cpp
// 맵 블루프린트에 BGM 적용
void AGameMap::BeginPlay()
{
    Super::BeginPlay();
    
    // BGM 컴포넌트 초기화
    if (BGMComponent && BackgroundMusic)
    {
        BGMComponent->SetSound(BackgroundMusic);
        BGMComponent->SetVolumeMultiplier(0.7f);
        BGMComponent->FadeIn(2.0f);
        BGMComponent->Play();
    }
}

// 게임 상황별 음악 변경
void AGameMap::OnGameStateChanged(EGameState NewState)
{
    switch(NewState)
    {
        case EGameState::Preparation:
            PlayPreparationMusic();
            break;
        case EGameState::InProgress:
            PlayCombatMusic();
            break;
        case EGameState::Finished:
            PlayVictoryMusic();
            break;
    }
}
```

![BGM 시스템](../Images/Sprints_img/sprint14/bgm_system_integration.jpg)
*맵 블루프린트에 통합된 BGM 시스템*

## 7. 발생한 문제점 및 해결 과정

### 7.1 팀 텍스트 컬러 적용 실패

**문제 상황:**
동적으로 생성되는 팀 UI에서 팀별 색상이 제대로 적용되지 않는 문제가 발생했습니다.

![팀 컬러 실패](../Images/Sprints_img/sprint14/team_color_failure_debug.jpg)
*팀 색상 적용 실패 디버깅 과정*

**문제 원인:**
1. 런타임에서 머티리얼 인스턴스 생성 타이밍 문제
2. 위젯 생성과 색상 설정 순서 불일치
3. 네트워크 동기화에서 색상 데이터 누락

**임시 해결책:**
```cpp
// 데모용 4팀 고정 UI로 대체
void UGameHUD::CreateFixedTeamUI()
{
    // 하드코딩된 4팀 색상으로 안정성 확보
    TeamColors = {
        {0, FLinearColor::Red},
        {1, FLinearColor::Blue}, 
        {2, FLinearColor::Green},
        {3, FLinearColor::Yellow}
    };
    
    // 고정된 위치에 UI 생성
    CreateStaticTeamWidgets();
}
```

### 7.2 성능 문제로 인한 기능 제거

**발생한 문제:**
로그 시스템에서 과도한 데이터 수집으로 인해 게임 종료 시 심각한 프레임 드랍이 발생했습니다.

**영향도 분석:**
- 게임 종료 시 3-5초간 프리징
- 메모리 사용량 급증 (500MB → 1.2GB)
- 네트워크 패킷 과부하

**대응 방안:**
```cpp
// 임시 해결: 로그 수집 비활성화
void UGameLogSystem::DisableHeavyLogging()
{
    bCollectDetailedStats = false;
    bSavePlayerActions = false;
    
    // 최소한의 기본 정보만 수집
    bCollectBasicScore = true;
    bCollectWinCondition = true;
}
```

## 8. 다음 스프린트 계획 (스프린트 15)

### 8.1 서버 인프라 구축
- Steam 플랫폼 통합 시스템
- 데디케이티드 서버 환경 구축
- 서버-클라이언트 분리 아키텍처

### 8.2 네트워크 시스템 확장
- Steam 네트워킹 기능 활용
- 실제 멀티플레이어 환경 테스트
- 성능 최적화 및 안정성 강화

### 8.3 현재 시스템 호환성 검토
- 기존 시스템들의 데디케이티드 서버 적응
- 네트워크 동기화 로직 재평가
- Steam SDK와 기존 코드 통합

## 9. 회고 및 학습 포인트

### 9.1 공모전 데모 완성의 성취

이번 스프린트에서 가장 큰 성과는 **실제 플레이 가능한 완성도 높은 데모**를 만들어낸 것입니다:

- **명확한 게임 플로우**: 시작부터 종료까지 끊김없는 진행
- **공정한 경쟁 환경**: 준비 시간과 밸런싱을 통한 공정성 확보
- **시각적 완성도**: 실시간 UI 업데이트와 피드백 시스템

### 9.2 기술적 도전과 성장

**동적 UI 시스템 구현:**
- 런타임 위젯 생성과 관리 기술 습득
- 네트워크 환경에서의 UI 동기화 경험
- 성능을 고려한 UI 최적화 방법 학습

**게임플레이 시스템 통합:**
- 여러 시스템 간의 유기적 연동 경험
- 사용자 경험을 고려한 타이밍 조절
- 예외 상황 처리와 안정성 확보

### 9.3 현실적 판단과 타협

**완벽함보다 안정성:**
팀 컬러 시스템에서 완벽한 구현보다는 안정적인 데모를 위해 임시 해결책을 선택한 결정이 옳았습니다. 공모전이라는 명확한 데드라인 앞에서 우선순위를 정확히 판단했습니다.

**성능 vs 기능:**
로그 시스템 기능 제거 결정 역시 사용자 경험을 최우선으로 고려한 현실적 판단이었습니다. 완성도 높은 핵심 기능이 부가적인 고급 기능보다 중요함을 깨달았습니다.

### 9.4 스프린트 14의 의미

스프린트 14는 단순한 기능 개발을 넘어서 **"제품 출시"의 경험**을 제공해주었습니다:
- 데드라인 내 완성품 제작
- 사용자 관점에서의 품질 검증
- 기술적 이상과 현실적 제약의 균형

다음 스프린트에서는 더 안정적이고 확장 가능한 서버 인프라를 구축하여 게임의 완성도를 한 단계 더 높일 예정입니다.