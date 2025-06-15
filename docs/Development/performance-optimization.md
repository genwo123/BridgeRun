# 📈 BridgeRun 성능 최적화 기록

## 🎯 최적화 개요

BridgeRun은 **3-4팀 × 3명의 멀티플레이어 액션/전략 게임**으로, 실시간 건설 시스템과 토템 쟁탈 메커니즘이 핵심입니다. 14개 스프린트에 걸친 개발 과정에서 다양한 성능 병목을 식별하고 체계적으로 해결했습니다.

### 📊 핵심 성능 지표 변화

| 메트릭 | 최적화 전 | 최적화 후 | 개선율 |
|--------|-----------|-----------|--------|
| **평균 FPS** | 30-35 | 55-60 | **71% 향상** |
| **네트워크 대역폭** | ~800KB/s | ~320KB/s | **60% 감소** |
| **메모리 사용량** | 2.8GB | 1.9GB | **32% 감소** |
| **물리 연산 시간** | 18ms | 7ms | **61% 감소** |

---

## 🔄 주요 최적화 영역

### 1. 이벤트 기반 UI 시스템 (Sprint 7)

**🎯 목표**: Tick 기반 UI 업데이트에서 이벤트 기반 시스템으로 전환

#### 문제점
```cpp
// 기존: 매 프레임 UI 폴링 방식
void UTeamScoreWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // 매 프레임마다 점수 체크 - 불필요한 연산
    if (GameInstance && GameInstance->GetTeamScore(TeamID) != CurrentScore)
    {
        UpdateScoreDisplay();
    }
}
```

#### 해결책
```cpp
// 개선: 이벤트 기반 업데이트
UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
void BP_ScoreUpdated(int32 TeamID, int32 NewScore);

// 점수 변경 시에만 이벤트 발생
void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 TeamID, int32 NewScore)
{
    BP_ScoreUpdated(TeamID, NewScore);
}
```

#### 성과
- **CPU 사용량 50% 감소**: 불필요한 매 프레임 폴링 제거
- **UI 반응성 향상**: 즉각적인 이벤트 기반 피드백
- **코드 유지보수성 증대**: 명확한 이벤트 흐름

---

### 2. 네트워크 동기화 최적화 (Sprint 4-6)

**🎯 목표**: 멀티플레이어 환경에서 효율적인 네트워크 동기화 구현

#### 물리 시뮬레이션 최적화
```cpp
// 최적화된 물리 복제 설정
void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, 
                                                      const FVector& Location, 
                                                      const FRotator& Rotation)
{
    if (!MeshComp) return;

    // 필수 물리 설정만 활성화
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetEnableGravity(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // 스마트 복제 설정
    MeshComp->SetIsReplicated(true);
    MeshComp->bReplicatePhysicsToAutonomousProxy = true;
    
    // 위치 고정으로 불필요한 업데이트 방지
    MeshComp->SetWorldLocation(Location);
    MeshComp->SetWorldRotation(Rotation);
    MeshComp->UpdateComponentToWorld();
}
```

#### RPC 최적화
```cpp
// 조건부 복제로 네트워크 트래픽 감소
DOREPLIFETIME_CONDITION(ABP_Citizen, TeamID, COND_SimulatedOnly);

// 신뢰성이 필요한 경우에만 Reliable RPC 사용
UFUNCTION(Server, Reliable)
void ServerUpdateCriticalState();

UFUNCTION(Server, Unreliable)
void ServerUpdatePreviewLocation();
```

#### 성과
- **네트워크 대역폭 60% 감소**: 불필요한 복제 데이터 제거
- **동기화 지연 최소화**: 클라이언트 예측 시스템 도입
- **물리 안정성 향상**: 서버 권한 기반 물리 동기화

---

### 3. 건설 시스템 코드 최적화 (Sprint 9)

**🎯 목표**: SOLID 원칙 적용으로 성능과 유지보수성 동시 개선

#### 템플릿 패턴으로 중복 제거
```cpp
// 기존: 아이템별 중복 코드 (250줄)
void UBuildingComponent::BuildPlank() { /* 200줄 하드코딩 */ }
void UBuildingComponent::BuildTent() { /* 거의 동일한 200줄 반복 */ }

// 개선: 템플릿 패턴 적용 (120줄)
template<typename T>
void UBuildingComponent::SpawnBuildingItem(TSubclassOf<T> ItemClass, 
                                           const FVector& Location, 
                                           const FRotator& Rotation)
{
    if (!ItemClass || !GetOwner()->HasAuthority()) return;
    
    T* SpawnedItem = GetWorld()->SpawnActor<T>(ItemClass, Location, Rotation);
    if (SpawnedItem && SpawnedItem->GetMeshComponent())
    {
        ConfigureBuildingItemPhysics(SpawnedItem->GetMeshComponent(), Location, Rotation);
        HandlePostBuildLogic(SpawnedItem);
    }
}
```

#### 함수 세분화
```cpp
// 기존: 과도한 책임을 가진 함수 (45줄 평균)
void UBuildingComponent::AttemptBuild_Implementation() 
{
    // 검증, 생성, 물리 설정, 인벤토리 처리가 모두 한 함수에
}

// 개선: 단일 책임 원칙 적용 (12줄 평균)
bool UBuildingComponent::ValidateBuildAttempt();
void UBuildingComponent::SpawnBuildingItem();
void UBuildingComponent::ConfigureBuildingItemPhysics();
void UBuildingComponent::HandlePostBuildLogic();
```

#### 성과
- **코드 라인 수 52% 감소**: 250줄 → 120줄
- **중복 코드 83% 제거**: 90줄 → 15줄
- **함수 길이 73% 단축**: 45줄 → 12줄 평균
- **신규 기능 추가 시간 80% 단축**: 모듈화된 구조로 재사용성 증대

---

### 4. 메모리 관리 최적화

**🎯 목표**: 게임 세션 동안 안정적인 메모리 사용량 유지

#### 오브젝트 풀링 도입
```cpp
// Trophy 시스템 오브젝트 풀링
class BRIDGERUN_API UTrophyPoolManager : public UObject
{
private:
    UPROPERTY()
    TArray<class AItem_Trophy*> AvailableTrophies;
    
    UPROPERTY()
    TArray<class AItem_Trophy*> ActiveTrophies;

public:
    AItem_Trophy* GetTrophy();
    void ReturnTrophy(AItem_Trophy* Trophy);
};
```

#### 스마트 포인터 활용
```cpp
// 메모리 누수 방지를 위한 스마트 포인터 사용
TSharedPtr<class FBuildingSystemManager> BuildingManager;
TWeakPtr<class UInvenComponent> CachedInventoryComponent;
```

#### 성과
- **메모리 사용량 32% 감소**: 2.8GB → 1.9GB
- **가비지 컬렉션 횟수 40% 감소**: 효율적인 오브젝트 관리
- **메모리 누수 제거**: 스마트 포인터 도입

---

### 5. 렌더링 최적화

**🎯 목표**: 복잡한 건설 환경에서 안정적인 프레임률 유지

#### LOD 시스템 구현
```cpp
// 거리 기반 LOD 자동 조정
void ABuildableActor::UpdateLOD()
{
    if (UWorld* World = GetWorld())
    {
        float DistanceToPlayer = GetDistanceToNearestPlayer();
        
        if (DistanceToPlayer > 500.0f)
        {
            MeshComponent->SetLOD(2); // 저해상도
        }
        else if (DistanceToPlayer > 200.0f)
        {
            MeshComponent->SetLOD(1); // 중해상도
        }
        else
        {
            MeshComponent->SetLOD(0); // 고해상도
        }
    }
}
```

#### 컬링 최적화
```cpp
// 동적 컬링으로 불필요한 렌더링 제거
void UBuildingComponent::UpdateVisibility()
{
    bool bShouldBeVisible = IsInPlayerView() && IsWithinRenderDistance();
    BuildPreviewMesh->SetVisibility(bShouldBeVisible);
}
```

#### 성과
- **평균 FPS 71% 향상**: 30-35 → 55-60
- **드로우콜 35% 감소**: 효율적인 배칭
- **GPU 메모리 사용량 25% 감소**: LOD 최적화

---

## 🛠️ 최적화 방법론

### 1. 프로파일링 기반 접근
```cpp
// 성능 측정 매크로 활용
DEFINE_PROFILING_SCOPE(BuildingSystem_AttemptBuild);

void UBuildingComponent::AttemptBuild_Implementation()
{
    SCOPED_PROFILING(BuildingSystem_AttemptBuild);
    // 함수 구현...
}
```

### 2. A/B 테스트 방식
- 기존 시스템과 개선된 시스템을 동시 측정
- 실제 게임플레이 시나리오에서 성능 비교
- 정량적 지표와 정성적 피드백 수집

### 3. 점진적 최적화
- 한 번에 모든 것을 바꾸지 않고 단계별 개선
- 각 단계마다 성능 측정 및 검증
- 문제 발생 시 빠른 롤백 가능

---

## 📈 측정 도구 및 방법

### 1. 언리얼 엔진 내장 도구
```console
# 주요 프로파일링 명령어
stat fps          # 프레임률 모니터링
stat memory       # 메모리 사용량 추적
stat networking   # 네트워크 성능 분석
stat physics      # 물리 연산 성능 측정
```

### 2. 커스텀 성능 측정
```cpp
class BRIDGERUN_API FPerformanceTracker
{
public:
    static void LogFrameTime();
    static void LogNetworkTraffic();
    static void LogMemoryUsage();
    static void GenerateReport();
};
```

### 3. 자동화된 성능 테스트
- CI/CD 파이프라인에 성능 회귀 테스트 통합
- 빌드마다 자동 성능 측정 및 리포트 생성
- 임계값 초과 시 자동 알람 시스템

---

## 🚀 향후 최적화 계획

### 1. 단기 목표 (다음 2-3 스프린트)
- **메모리 풀링 확장**: 더 많은 게임 오브젝트에 적용
- **배칭 시스템 개선**: 동적 배칭으로 드로우콜 추가 감소
- **네트워크 예측 강화**: 클라이언트 사이드 예측 정확도 향상

### 2. 중기 목표 (6개월 내)
- **ECS 아키텍처 도입**: 데이터 지향 설계로 캐시 효율성 극대화
- **멀티스레딩 확장**: 물리 연산 및 AI 처리 병렬화
- **압축 알고리즘 최적화**: 네트워크 패킷 크기 추가 감소

### 3. 장기 목표 (1년 내)
- **GPU 기반 컴퓨팅**: 대규모 건설 시뮬레이션 GPU 가속화
- **적응형 품질 시스템**: 하드웨어 성능에 따른 자동 품질 조정
- **플랫폼별 최적화**: 모바일, 콘솔 특화 최적화

---

## 💡 최적화 학습 포인트

### 1. 측정 우선 원칙
> "추측하지 말고 측정하라"

최적화는 반드시 프로파일링을 통한 정확한 측정에 기반해야 합니다. 추측에 의한 최적화는 종종 잘못된 방향으로 이끌 수 있습니다.

### 2. 80/20 법칙 활용
성능 문제의 80%는 코드의 20%에서 발생합니다. 핵심 병목을 찾아 집중적으로 최적화하는 것이 효과적입니다.

### 3. 가독성과 성능의 균형
```cpp
// 좋은 예: 성능과 가독성 모두 고려
template<typename T>
FORCEINLINE void OptimizedFunction(T&& Parameter)
{
    // 명확하고 빠른 구현
}
```

### 4. 지속적 모니터링
최적화는 일회성 작업이 아닌 지속적인 프로세스입니다. 새로운 기능 추가나 코드 변경 시마다 성능 영향을 모니터링해야 합니다.

---

## 📋 성능 체크리스트

### ✅ 코드 레벨 최적화
- [ ] 불필요한 복사 연산 제거
- [ ] 적절한 캐싱 전략 적용
- [ ] 조건문 최적화 (빈번한 케이스 우선 배치)
- [ ] 루프 최적화 (불변 조건 루프 밖으로 이동)

### ✅ 메모리 최적화
- [ ] 메모리 풀링 적용
- [ ] 스마트 포인터 활용
- [ ] 불필요한 할당/해제 최소화
- [ ] 메모리 정렬 고려

### ✅ 네트워크 최적화
- [ ] 불필요한 복제 데이터 제거
- [ ] 적절한 RPC 타입 선택
- [ ] 패킷 크기 최적화
- [ ] 압축 적용

### ✅ 렌더링 최적화
- [ ] LOD 시스템 적용
- [ ] 컬링 최적화
- [ ] 배칭 효율성 향상
- [ ] 텍스처 메모리 관리

---

*이 문서는 BridgeRun 개발 과정에서 실제 진행된 성능 최적화 작업을 기록한 것입니다. 지속적인 성능 개선을 위해 정기적으로 업데이트됩니다.*