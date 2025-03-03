# 브릿지런 개발 회고록 (스프린트 6)

📅 회고 일자
2025년 2월 16일

👨‍💻 작성자
김건우

## 1. 개발 내용 요약

이번 스프린트에서는 팀 시스템 구현과 낙사/리스폰 메커니즘 개발에 중점을 두었습니다. 주요 개발 내용은 다음과 같습니다:

### 1.1 팀 시스템 구현

**목표**: 팀 기반 게임플레이를 위한 기본 구조 설계
**결과**: 성공적으로 구현 완료
**세부 내용**:
- GameState에 TeamInfo 구조체 추가 (팀 ID, 점수, 플레이어 카운트)
- 플레이어 자동 팀 배정 로직 구현
- 팀 정보의 네트워크 복제 설정

```cpp
// 핵심 코드
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
    // ... 스폰 로직
}
```

### 1.2 낙사 판정 및 리스폰 시스템 구현

**목표**: 플레이어 낙사 시 적절한 리스폰 처리
**결과**: 완벽하게 구현
**세부 내용**:
- 맵 하단에 DeathVolume 구현
- 플레이어 캐릭터의 사망 상태 처리
- 타이머 기반 5초 후 리스폰 메커니즘
- 팀 ID 기반 스폰 위치 선택

```cpp
void ADeathVolume::HandleOverlap(UPrimitiveComponent* OverlappedComponent, 
                                  AActor* OtherActor, 
                                  UPrimitiveComponent* OtherComp, 
                                  int32 OtherBodyIndex, 
                                  bool bFromSweep, 
                                  const FHitResult& SweepResult)
{
    if (!OtherActor || !GetWorld()) return;

    // 플레이어 캐릭터 처리
    if (ACitizen* Character = Cast<ACitizen>(OtherActor))
    {
        FVector DeathLocation = Character->GetActorLocation();
        Character->HandleDeath();
        
        // 타이머로 5초 후 리스폰
        FTimerHandle RespawnTimerHandle;
        FTimerDelegate RespawnDelegate;
        RespawnDelegate.BindUFunction(this, FName("RespawnPlayer"), Character);
        GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, 5.0f, false);
    }
}
```

### 1.3 트로피 낙사 처리 시스템

**목표**: 트로피 낙사 시 자동 리스폰 시스템 구현
**결과**: 구현 완료 (물리/충돌 시스템 이슈 일부 해결 중)
**세부 내용**:
- 트로피 감지 및 리스폰 처리
- NavMesh 기반 안전한 리스폰 위치 계산
- 물리 기반 트로피의 네트워크 동기화 개선

```cpp
void ADeathVolume::RespawnTrophy(AItem_Trophy* Trophy, const FVector& DeathLocation)
{
    if (!Trophy || !GetWorld()) return;
    
    // 적절한 리스폰 위치 계산
    FVector RespawnLocation = GetTrophyRespawnLocation(DeathLocation);
    Trophy->ServerTryRespawn(RespawnLocation);
}

FVector ADeathVolume::GetTrophyRespawnLocation(const FVector& DeathLocation) const
{
    // NavMesh 기반 안전한 위치 계산
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem) return FVector(0, 0, 200);
    
    FNavLocation ResultLocation;
    bool bSuccess = NavSystem->GetRandomReachablePointInRadius(
        DeathLocation,
        500.0f,
        ResultLocation
    );
    
    return bSuccess ? ResultLocation.Location : FVector(0, 0, 200);
}
```

## 2. 성공 및 개선 사항

### 🌟 성공한 부분

- **팀 시스템 기반 구축**: GameMode를 확장하여 팀 정보를 관리하고, 플레이어가 참여할 때 자동으로 팀을 배정하는 시스템을 구현했습니다. 이를 통해 향후 팀 기반 게임플레이 기능을 확장할 수 있는 기반을 마련했습니다.

- **낙사 및 리스폰 메커니즘**: DeathVolume을 이용한 낙사 감지 및 타이머 기반 리스폰 시스템이 안정적으로 구현되었습니다. 플레이어가 맵 밖으로 떨어질 경우 5초 후 적절한 위치에 리스폰되는 기능이 정상적으로 작동합니다.

- **팀별 스폰 포인트**: 팀 ID를 기반으로 적절한 스폰 위치를 선택하는 기능을 구현했습니다. 이를 통해 팀별로 다른 시작 위치를 갖는 게임플레이가 가능해졌습니다.

### 🔧 개선이 필요한 부분

- **트로피 물리/충돌 시스템**: 트로피가 낙사 후 리스폰될 때 일부 물리/충돌 이슈가 발생했습니다. 특히 서버와 클라이언트 간에 충돌 동작이 일치하지 않는 문제가 있었습니다. 향후 스프린트에서 더 개선이 필요합니다.

- **팀 밸런싱 알고리즘**: 현재 구현된 팀 자동 배정 알고리즘은 단순히 플레이어 수가 가장 적은 팀을 선택합니다. 플레이어 스킬 레벨이나 다른 요소를 고려한 더 복잡한 밸런싱 알고리즘이 필요할 수 있습니다.

- **네트워크 최적화**: 리스폰 과정에서 불필요한 네트워크 업데이트가 발생하는 경우가 있어 최적화가 필요합니다.

## 3. 기술적 도전과 해결 방안

### 3.1 팀 정보의 네트워크 복제

**문제 상황**: TeamInfo 구조체의 네트워크 복제 과정에서 모든 클라이언트에게 정보가 정확히 전달되지 않는 문제가 발생했습니다.

**시도한 접근법**:
- 초기 접근: 표준 DOREPLIFETIME 매크로 사용 → 일부 정보만 복제됨
- 중간 접근: DOREPLIFETIME_CONDITION 사용 → 부분적 개선
- 최종 접근: 게임 상태에 TeamInfo 추가 및 ForceNetUpdate 호출 → 해결

**최종 해결책**:
```cpp
void ABridgeRunGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABridgeRunGameMode, TeamInfo);
}

void ABridgeRunGameMode::UpdateGameState()
{
    ForceNetUpdate();
}
```

**학습 포인트**:
- 구조체 배열 복제의 복잡성 이해
- 네트워크 업데이트 타이밍의 중요성
- ForceNetUpdate 사용의 적절한 시점

### 3.2 리스폰 시스템 동기화

**문제 상황**: 멀티플레이어 환경에서 플레이어 리스폰 과정의 동기화 문제가 발생했습니다.

**접근 방법**:
- 클라이언트-서버 간 사망 상태 동기화
- ServerRPC를 통한 리스폰 요청
- MulticastRPC를 통한 사망 처리 동기화

**현재 상태**: 
완전히 해결됨 - 서버 권한 기반의 리스폰 처리로 모든 클라이언트에서 일관된 동작 확보

## 4. 시간 관리 및 일정 평가

### 계획 대비 실제 진행

| 작업 | 계획 시간 | 실제 소요 시간 | 차이 |
|------|-----------|----------------|------|
| 팀 시스템 구현 | 4일 | 4일 | 0일 |
| 리스폰 시스템 | 3일 | 4일 | +1일 |
| 트로피 처리 | 2일 | 3일 | +1일 |
| 테스트 및 디버깅 | 3일 | 3일 | 0일 |

### 시간 관리 문제점

- 트로피 물리/충돌 시스템 디버깅에 예상보다 많은 시간 소요
- 네트워크 동기화 이슈 해결에 추가 시간 필요
- 팀 시스템 테스트에 필요한 멀티플레이어 테스트 구성에 시간 소요

### 개선 방안

- 복잡한 물리 기반 기능은 더 많은 시간 버퍼 할당
- 멀티플레이어 테스트 환경 자동화 고려
- 핵심 기능과 선택적 기능의 명확한 구분

## 5. 배운 점 및 인사이트

### 기술적 인사이트

**팀 기반 게임 아키텍처**:
- 팀 정보의 중앙 관리 중요성
- 플레이어-팀 관계 설계 방법
- 팀 기반 로직의 네트워크 복제 전략

**리스폰 시스템**:
- 상태 관리 및 타이머 활용
- 서버 권한 기반 처리의 중요성
- 클라이언트-서버 간 동기화 방법

**물리/충돌 시스템**:
- 물리 객체의 네트워크 복제 특성
- 충돌 설정의 일관성 유지 방법
- 리스폰 후 상태 초기화 접근법

### 개인적 성장

- 멀티플레이어 시스템 디버깅 기술 향상
- 복잡한 게임 상태 관리 경험 축적
- 네트워크 최적화 관점의 사고 발전

## 6. 다음 스프린트 준비

### 핵심 목표

- 트로피 물리/충돌 시스템 완성
- 점수 시스템 구현 및 UI 연동
- 캐릭터 상태 초기화 개선

### 주요 작업 항목

- PhysicsActor 프로필 기반 트로피 충돌 시스템 구현
- 팀 점수 UI 디자인 및 구현
- 리스폰 시 아이템 및 상태 처리 개선

### 준비 사항

- 물리/충돌 시스템 테스트 시나리오 작성
- UI 디자인 레퍼런스 수집
- 리스폰 관련 버그 목록 정리

## 7. 종합 평가 및 소감

이번 스프린트에서는 팀 기반 게임플레이의 기초를 구축하는 중요한 진전이 있었습니다. 특히 팀 정보 관리 및 플레이어 팀 배정 시스템을 성공적으로 구현하여 향후 팀 기반 게임플레이 기능을 확장할 수 있는 토대를 마련했습니다.

낙사 및 리스폰 시스템은 예상보다 복잡한 네트워크 동기화 이슈가 발생했지만, 서버 권한 모델을 명확히 구현함으로써 안정적인 시스템을 구축할 수 있었습니다. 특히 플레이어와 트로피의 리스폰 처리를 분리하여 각각에 최적화된 방식으로 구현한 것이 효과적이었습니다.

트로피의 물리/충돌 시스템에서는 아직 개선의 여지가 있지만, 기본적인 리스폰 메커니즘은 잘 작동하고 있습니다. 다음 스프린트에서는 이 부분을 더 개선하여 완전히 안정적인 시스템을 구축할 계획입니다.

전체적으로 이번 스프린트는 계획했던 핵심 기능들을 대부분 구현하는 데 성공했으며, 물리/충돌 시스템의 복잡성으로 인해 일부 작업이 다음 스프린트로 이월되었지만 전반적인 진행 속도는 양호했습니다.
