# 브릿지런 개발일지 (스프린트 16)

## 📅 개발 기간
2025년 6월 16일 ~ 2025년 6월 29일 (2주)

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 16에서는 실제 서비스를 위한 서버 인프라 구축에 집중하고 있습니다:
- Steam 플랫폼 완전 통합 시스템 구축
- 데디케이티드 서버 환경의 완전한 구현
- 서버-클라이언트 분리 아키텍처 완성
- 기존 시스템들의 서버 환경 호환성 확보

## 2. Steam SDK 연동 시스템

### 2.1 Steam 플랫폼 통합 배경

스프린트 15에서 문서화 작업을 통해 명확해진 방향성을 바탕으로, 실제 서비스 수준의 플랫폼 통합 작업을 시작했습니다.

![Steam 통합 아키텍처](../Images/Sprints_img/sprint16/steam_integration_architecture.jpg)
*Steam SDK와 BridgeRun 시스템의 통합 구조도*

### 2.2 Steam 통합 진행 과정
```
Steam SDK 통합 단계:
[Steam SDK 설치] → [프로젝트 설정] → [기본 API 연동] → [플레이어 정보] → [세션 관리]
        ↓              ↓              ↓              ↓              ↓
    개발 환경 준비    빌드 설정 수정   초기화 테스트   인증 시스템    로비 연동
```

### 2.3 Steam 기본 연동 구현

```cpp
// Steam API 초기화 및 기본 설정
class BRIDGERUN_API USteamIntegrationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // Steam 플레이어 정보
    UFUNCTION(BlueprintCallable)
    FString GetPlayerSteamName();
    
    UFUNCTION(BlueprintCallable)
    bool IsPlayerLoggedIn();
    
    UFUNCTION(BlueprintCallable)
    FString GetPlayerSteamID();
    
protected:
    bool bSteamInitialized = false;
    FString CachedPlayerName;
    uint64 CachedSteamID;
};

// Steam API 초기화 구현
void USteamIntegrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Steam API 초기화 시도
    if (SteamAPI_Init())
    {
        bSteamInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("Steam API 초기화 성공"));
        
        // 기본 플레이어 정보 캐싱
        if (SteamUser())
        {
            CachedSteamID = SteamUser()->GetSteamID().ConvertToUint64();
            CachedPlayerName = UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API 초기화 실패"));
        bSteamInitialized = false;
    }
}
```

![Steam 플레이어 정보](../Images/Sprints_img/sprint16/steam_player_info_display.jpg)
*게임 내에서 표시되는 Steam 플레이어 정보*

### 2.4 Steam 세션 관리 구현

```cpp
// Steam 로비 시스템과 연동
class BRIDGERUN_API USteamLobbyManager : public UObject
{
public:
    // Steam 로비 생성
    UFUNCTION(BlueprintCallable)
    bool CreateSteamLobby(int32 MaxPlayers = 12);
    
    // Steam 로비 참가
    UFUNCTION(BlueprintCallable)
    bool JoinSteamLobby(uint64 LobbyID);
    
    // 로비 멤버 관리
    UFUNCTION(BlueprintCallable)
    TArray<FSteamLobbyMember> GetLobbyMembers();
    
protected:
    // Steam 콜백 처리
    STEAM_CALLBACK(USteamLobbyManager, OnLobbyCreated, LobbyCreated_t);
    STEAM_CALLBACK(USteamLobbyManager, OnLobbyEntered, LobbyEnter_t);
};
```

## 3. 데디케이티드 서버 구축

### 3.1 서버-클라이언트 완전 분리

기존 Listen Server 방식에서 완전한 Dedicated Server 환경으로 전환 작업을 진행했습니다.

![서버 아키텍처 변화](../Images/Sprints_img/sprint16/server_architecture_transition.jpg)
*Listen Server에서 Dedicated Server로의 전환 과정*

### 3.2 서버 환경 구성
```
데디케이티드 서버 구조:

Before (Listen Server):
[Host Player + Server] ←→ [Client 1] ←→ [Client 2] ←→ [Client 3]

After (Dedicated Server):
[Dedicated Server] ←→ [Client 1] ←→ [Client 2] ←→ [Client 3] ←→ [Client 4]
```

### 3.3 서버 전용 게임 모드 구현

```cpp
// 데디케이티드 서버 전용 설정
UCLASS()
class BRIDGERUN_API ABridgeRunDedicatedGameMode : public ABridgeRunGameMode
{
    GENERATED_BODY()
    
public:
    ABridgeRunDedicatedGameMode();
    
    virtual void StartPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    
protected:
    // 서버 전용 초기화
    void InitializeDedicatedServerSettings();
    
    // 클라이언트 연결 관리
    void HandleClientConnection(APlayerController* NewPlayer);
    void HandleClientDisconnection(AController* ExitingPlayer);
    
    // 서버 성능 모니터링
    FTimerHandle ServerMonitoringHandle;
    void MonitorServerPerformance();
    
    // 서버 상태 추적
    UPROPERTY()
    int32 ConnectedPlayersCount = 0;
    
    UPROPERTY()
    float ServerStartTime;
};

// 서버 시작 시 초기화
void ABridgeRunDedicatedGameMode::StartPlay()
{
    Super::StartPlay();
    
    if (GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogTemp, Log, TEXT("데디케이티드 서버 모드로 시작"));
        
        InitializeDedicatedServerSettings();
        ServerStartTime = GetWorld()->GetTimeSeconds();
        
        // 30초마다 서버 상태 모니터링
        GetWorld()->GetTimerManager().SetTimer(
            ServerMonitoringHandle,
            this,
            &ABridgeRunDedicatedGameMode::MonitorServerPerformance,
            30.0f,
            true
        );
    }
}
```

![서버 상태 모니터링](../Images/Sprints_img/sprint16/server_monitoring_dashboard.jpg)
*데디케이티드 서버 성능 및 상태 모니터링*

## 4. 기존 시스템 호환성 작업

### 4.1 시스템별 서버 환경 적응 현황

기존에 구현된 시스템들이 데디케이티드 서버 환경에서 제대로 작동하도록 수정 작업을 진행했습니다.

![시스템 호환성 매트릭스](../Images/Sprints_img/sprint16/system_compatibility_matrix.jpg)
*각 시스템별 서버 환경 호환성 점검 결과*

### 4.2 호환성 점검 및 수정 결과
```
시스템별 호환성 현황:

✅ 완전 호환 (수정 불필요):
- 아이템 스폰 시스템: 서버 권한 기반으로 이미 구현
- 트로피 점수 시스템: GameState 활용으로 호환성 우수  
- 팀 관리 시스템: PlayerState 기반으로 문제없음

⚠️ 부분 수정 완료:
- BuildingComponent: UI 업데이트 로직을 클라이언트로 분리
- CombatComponent: 이펙트 처리를 클라이언트 전용으로 수정
- InvenComponent: 인벤토리 UI 동기화 개선

🔧 수정 진행 중:
- 로비 시스템: Steam 세션과 통합하여 재구성
- 게임 결과 시스템: 서버-클라이언트 데이터 분리
```

### 4.3 UI 로직 분리 작업

```cpp
// Before: UI와 로직이 혼재
void UBuildingComponent::UpdateBuildPreview()
{
    // 서버에서도 UI 업데이트 시도 (문제 발생)
    if (PreviewWidget)
    {
        PreviewWidget->SetVisibility(ESlateVisibility::Visible);
    }
    
    // 건설 로직
    ValidateBuildLocation();
}

// After: 로직과 UI 완전 분리
void UBuildingComponent::UpdateBuildPreview_Implementation()
{
    // 서버: 로직만 처리
    if (HasAuthority())
    {
        ValidateBuildLocation();
        MulticastUpdatePreview(bIsValidLocation);
    }
}

UFUNCTION(NetMulticast, Reliable)
void UBuildingComponent::MulticastUpdatePreview(bool bValidLocation)
{
    // 클라이언트: UI만 처리
    if (GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        UpdatePreviewUI(bValidLocation);
    }
}
```

## 5. 네트워크 최적화 작업

### 5.1 네트워크 트래픽 분석 및 최적화

데디케이티드 서버 환경에서의 네트워크 성능을 분석하고 최적화했습니다.

![네트워크 트래픽 분석](../Images/Sprints_img/sprint16/network_traffic_analysis.jpg)
*12명 동시 플레이 시 네트워크 트래픽 분석 결과*

### 5.2 최적화 전략 및 구현
```
네트워크 최적화 방안:

1. 데이터 복제 최적화:
   - 초기화 시에만 필요한 데이터: COND_InitialOnly
   - 소유자에게만 필요한 데이터: COND_OwnerOnly
   - 시뮬레이션 대상에게만: COND_SimulatedOnly

2. RPC 호출 최소화:
   - 중요한 액션만 Reliable RPC 사용
   - 빈번한 업데이트는 Replicated 변수 활용
   - 불필요한 Multicast 제거

3. 업데이트 빈도 조절:
   - 실시간성이 중요하지 않은 데이터는 업데이트 빈도 감소
   - 델타 압축 활용으로 대역폭 절약
```

### 5.3 성능 최적화 결과

```cpp
// 최적화된 데이터 복제 설정
void ACitizen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 팀 ID는 초기화 시에만 전송
    DOREPLIFETIME_CONDITION(ACitizen, TeamID, COND_InitialOnly);
    
    // 체력은 소유자와 서버만
    DOREPLIFETIME_CONDITION(ACitizen, Health, COND_OwnerOnly);
    
    // 위치는 시뮬레이션 대상에게만
    DOREPLIFETIME_CONDITION(ACitizen, ReplicatedMovement, COND_SimulatedOnly);
    
    // 현재 아이템은 모든 클라이언트에게 (시각적 표시용)
    DOREPLIFETIME(ACitizen, CurrentHeldItem);
}
```

![성능 개선 결과](../Images/Sprints_img/sprint16/performance_improvement_results.jpg)
*네트워크 최적화 전후 성능 비교*

## 6. 진행 중인 작업과 도전 과제

### 6.1 Steam Networking 통합 검토

Steam의 고급 네트워킹 기능을 활용할지 검토하고 있습니다.

```cpp
// Steam P2P 네트워킹 활용 계획
class USteamNetworkingManager : public UObject
{
public:
    // Steam P2P 연결 설정
    UFUNCTION(BlueprintCallable)
    bool SetupSteamP2PConnection(uint64 TargetSteamID);
    
    // NAT 통과 지원
    UFUNCTION(BlueprintCallable)
    bool EnableNATTraversal();
    
    // 연결 품질 모니터링
    UFUNCTION(BlueprintCallable)
    float GetConnectionQuality(uint64 SteamID);
    
protected:
    // Steam 네트워킹 콜백
    STEAM_CALLBACK(USteamNetworkingManager, OnP2PSessionRequest, P2PSessionRequest_t);
    STEAM_CALLBACK(USteamNetworkingManager, OnP2PSessionConnectFail, P2PSessionConnectFail_t);
};
```

### 6.2 서버 배포 및 운영 준비

실제 서비스를 위한 서버 배포 방안을 검토하고 있습니다.

```
서버 배포 옵션 검토:

Option 1: 로컬 전용 서버
- 장점: 완전한 제어, 비용 절약
- 단점: 접근성 제한, 확장성 부족

Option 2: 클라우드 서버 (AWS/Azure)
- 장점: 확장성, 안정성
- 단점: 운영비용, 복잡성

Option 3: Hybrid 방식
- 장점: 유연성, 단계적 확장
- 단점: 관리 복잡성
```

### 6.3 현재 직면한 기술적 도전

![기술적 도전 과제](../Images/Sprints_img/sprint16/technical_challenges.jpg)
*현재 해결 중인 주요 기술적 이슈들*

**주요 도전 과제들:**

1. **Steam SDK 학습 곡선**
   - C++ 기반 API의 복잡성
   - 언리얼 엔진과의 통합 이슈
   - 콜백 시스템 이해 및 활용

2. **기존 시스템 호환성**
   - UI 코드와 게임 로직의 완전한 분리
   - 네트워크 동기화 로직 재검증
   - 성능 최적화와 기능성의 균형

3. **서버 인프라 설계**
   - 확장 가능한 아키텍처 구성
   - 모니터링 및 로깅 시스템
   - 장애 복구 및 안정성 확보

## 7. 다음 주 완료 목표

### 7.1 Steam 통합 완성
- 플레이어 인증 및 정보 표시 완료
- 기본 로비 생성/참가 기능 구현
- Steam Friends와의 연동 테스트

### 7.2 데디케이티드 서버 안정화
- 모든 기존 시스템의 서버 환경 호환 완료
- 네트워크 성능 최적화 완료
- 12명 동시 플레이 안정성 확보

### 7.3 배포 준비
- 서버 빌드 자동화 구성
- 기본 모니터링 시스템 구축
- 실제 환경 테스트 준비

## 8. 회고 및 현재까지의 학습

### 8.1 서버 인프라 구축의 현실적 복잡성

스프린트 16을 통해 **실제 서비스 수준의 서버 구축**이 생각보다 훨씬 복잡하고 다양한 고려사항이 필요함을 깨달았습니다:

**기술적 복잡성:**
- Steam SDK 통합: 단순한 API 호출이 아닌 생태계 이해 필요
- 데디케이티드 서버: 아키텍처 전환의 근본적 변화
- 네트워크 최적화: 성능과 안정성의 섬세한 균형

**개발 방법론의 진화:**
- 문서화 → 실제 구현의 연결고리 체험
- 단계적 접근 방식의 중요성 인식
- 문제 발생 시 체계적 분석과 해결 과정

### 8.2 플랫폼 통합의 가치와 도전

**Steam 플랫폼 통합 경험:**
- 게임 개발 ≠ 플랫폼 개발의 차이점 인식
- 사용자 경험을 위한 플랫폼 기능의 중요성
- 기술적 구현과 비즈니스 가치의 연결

**실무 준비도 향상:**
- 상용 서비스를 위한 기술 스택 이해
- 확장성과 유지보수성을 고려한 설계
- 실제 사용자를 위한 안정성 확보

### 8.3 스프린트 15 문서화 작업의 가치

이전 스프린트의 문서화 작업이 현재 개발에 실질적 도움이 되었습니다:

**명확한 방향성 제시:**
- 체계화된 문서를 통한 개발 목표 명확화
- 기존 시스템에 대한 정확한 현황 파악
- 우선순위 기반 개발 계획 수립

**기술적 연속성 확보:**
- 이전 스프린트들의 성과 위에 견고한 확장
- 문제 해결 패턴의 일관성 유지
- 포트폴리오 스토리텔링의 연결고리

### 8.4 현재 진행 상황과 앞으로의 계획

**현재 달성 수준 (약 70%):**
- Steam 기본 연동: 80% 완료
- 데디케이티드 서버: 60% 완료
- 시스템 호환성: 75% 완료
- 네트워크 최적화: 65% 완료

**남은 1주 집중 계획:**
1. **Steam 로비 시스템 완성** (Critical)
2. **모든 시스템 서버 호환성 확보** (Critical)  
3. **성능 최적화 완료** (High)
4. **배포 환경 기본 구성** (Medium)

**기대하는 최종 결과:**
스프린트 16 완료 시 BridgeRun이 **실제 Steam에서 플레이 가능한 멀티플레이어 게임**이 되는 것이 목표입니다. 단순한 로컬 테스트 게임에서 실제 서비스 가능한 게임으로의 완전한 도약을 이루고자 합니다.

### 8.5 개발자로서의 성장

**기술적 깊이:**
- 플랫폼 SDK 통합 경험
- 서버 아키텍처 설계 능력
- 대규모 시스템 최적화 기법

**프로젝트 관리:**
- 복잡한 기술 작업의 단계별 분해
- 리스크 관리 및 우선순위 설정
- 현실적 목표 설정과 달성 전략

이번 스프린트는 BridgeRun 프로젝트에서 가장 도전적이면서도 의미있는 전환점이 되고 있습니다. 남은 기간 동안 계획한 목표들을 달성하여 완성도 높은 멀티플레이어 게임을 완성하겠습니다.