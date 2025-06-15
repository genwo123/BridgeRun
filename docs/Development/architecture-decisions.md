# 📐 BridgeRun 아키텍처 결정 기록
**Architecture Decision Records (ADR)** - 핵심 설계 결정과 그 배경, 그리고 결과

---

## 📋 의사결정 개요

이 문서는 BridgeRun 개발 과정에서 내린 주요 아키텍처 결정들을 기록합니다. 각 결정의 **배경**, **고려사항**, **선택 이유**, 그리고 **결과**를 명확히 문서화하여 향후 유사한 상황에서 참고할 수 있도록 합니다.

---

## 🏗️ ADR-001: 컴포넌트 기반 아키텍처 채택

📅 **결정 일자**: 2024년 12월 (Sprint 3)

### 🎯 결정 내용
캐릭터 시스템을 **모놀리식 클래스**에서 **컴포넌트 기반 아키텍처**로 전환

### 📊 상황 분석

**기존 문제점:**
* `ACitizen` 클래스가 800줄 이상의 거대한 클래스로 성장
* 건설, 전투, 인벤토리, 이동 로직이 모두 한 클래스에 혼재
* 새로운 기능 추가 시 기존 코드에 미치는 영향 범위가 불명확
* 팀 개발 시 코드 충돌 빈발

```cpp
// 문제가 된 모놀리식 구조
class ACitizen : public ACharacter 
{
    // 800줄의 모든 것을 다하는 갓 클래스
    void HandleCombat();        // 전투 관련 100줄
    void HandleBuilding();      // 건설 관련 200줄  
    void HandleInventory();     // 인벤토리 관련 150줄
    void HandleMovement();      // 이동 관련 100줄
    void HandleUI();           // UI 관련 100줄
    void HandleNetwork();      // 네트워크 관련 150줄
    // ... 수십 개의 혼재된 책임들
};
```

### 🔍 고려된 대안

| 대안 | 장점 | 단점 | 결정 |
|:---:|:---:|:---:|:---:|
| 모놀리식 유지 | 단순함, 성능 | 유지보수 불가, 확장성 없음 | ❌ 기각 |
| 상속 기반 분리 | OOP 친숙함 | 다중상속 복잡성, 경직성 | ❌ 기각 |
| 컴포넌트 기반 | 모듈화, 재사용성, SOLID | 초기 복잡성 증가 | ✅ 채택 |
| ECS 패턴 | 최고 성능, 데이터 중심 | 학습곡선, 언리얼과 부조화 | 🔄 향후 고려 |

### 💡 선택 이유

**1. SOLID 원칙 준수**
* **S**ingle Responsibility: 각 컴포넌트가 단일 책임
* **O**pen/Closed: 새 컴포넌트 추가로 확장, 기존 코드 수정 없음
* **D**ependency Inversion: 인터페이스 기반 통신

**2. 언리얼 엔진과의 조화**
* UE5의 네이티브 컴포넌트 시스템 활용
* 블루프린트와 자연스러운 통합
* 네트워크 복제 시스템과 호환

**3. 팀 개발 효율성**
* 개발자별 컴포넌트 담당으로 충돌 최소화
* 독립적인 테스트 가능
* 재사용 가능한 모듈

### 🎯 구현 결과

```cpp
// 컴포넌트 기반 새로운 구조
class ACitizen : public ACharacter 
{
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBuildingComponent* BuildingComp;     // 건설 전담
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  
    class UCombatComponent* CombatComp;         // 전투 전담
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UInvenComponent* InvenComp;           // 인벤토리 전담
    
    // 각 컴포넌트는 명확한 인터페이스로 통신
    void OnModeChanged(EPlayerMode NewMode) {
        BuildingComp->SetActive(NewMode == EPlayerMode::Building);
        CombatComp->SetActive(NewMode == EPlayerMode::Combat);
    }
};
```

### 📈 측정된 효과

| 메트릭 | 이전 | 이후 | 개선율 |
|:---:|:---:|:---:|:---:|
| 파일 크기 | 800줄 | 평균 150줄 | -81% |
| 함수 복잡도 | 평균 15 | 평균 6 | -60% |
| 코드 충돌 | 주 3-4회 | 주 0-1회 | -75% |
| 새 기능 개발 시간 | 2-3일 | 4-6시간 | -80% |

---

## 🌐 ADR-002: 데디케이티드 서버 아키텍처 선택

📅 **결정 일자**: 2025년 1월 (Sprint 4)

### 🎯 결정 내용
네트워크 아키텍처를 **P2P 방식**에서 **데디케이티드 서버 방식**으로 선택

### 📊 상황 분석

**게임 특성상의 요구사항:**
* 3-4팀 × 3명 = 최대 12명 동시 플레이
* 실시간 물리 시뮬레이션 (건설 시스템)
* 정확한 점수 계산 및 순위 결정
* 치팅 방지 필요성

### 🔍 고려된 대안

| 아키텍처 | 장점 | 단점 | 게임 적합성 |
|:---:|:---:|:---:|:---:|
| **P2P (Peer-to-Peer)** | 서버 비용 없음, 낮은 지연시간 | 치팅 취약, 동기화 복잡 | ❌ 부적합 |
| **Listen Server** | 구현 간단, 비용 절약 | 호스트 이탈 시 문제 | 🔄 1차 개발용 |
| **Dedicated Server** | 안정성, 공정성, 확장성 | 서버 비용, 구현 복잡 | ✅ 최종 선택 |

### 💡 선택 이유

**1. 게임 공정성 보장**
```cpp
// 서버 권한 검증 예시
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 서버에서만 실행되는 검증 로직
    if (!HasAuthority()) return;
    
    if (!ValidateBuildLocation() || !ValidateInventory()) {
        return; // 클라이언트 조작 차단
    }
    
    // 검증 통과 시에만 실행
    SpawnBuilding();
}
```

**2. 물리 시뮬레이션 일관성**
* 서버에서 단일 물리 시뮬레이션 실행
* 클라이언트는 결과만 동기화
* 플랫폼별 부동소수점 차이 해결

**3. 확장성 고려**
* 매치메이킹 시스템 구축 가능
* 서버 성능 모니터링
* 부하 분산 및 스케일링

### 🎯 구현 결과

```cpp
// RPC 패턴 체계화
class UBuildingComponent : public UActorComponent
{
    // 클라이언트 → 서버 요청
    UFUNCTION(Server, Reliable)
    void AttemptBuild();
    
    // 서버 → 모든 클라이언트 알림
    UFUNCTION(NetMulticast, Reliable) 
    void MulticastOnBuildComplete();
    
protected:
    // 서버 구현부
    virtual void AttemptBuild_Implementation();
    virtual void MulticastOnBuildComplete_Implementation();
};
```

### 📈 네트워크 성능 지표

| 메트릭 | 목표 | 현재 달성 | 상태 |
|:---:|:---:|:---:|:---:|
| RTT (왕복 지연) | <100ms | 85ms 평균 | ✅ 달성 |
| 패킷 손실률 | <1% | 0.3% | ✅ 달성 |
| 대역폭 사용량 | <1MB/분 | 0.7MB/분 | ✅ 달성 |
| 동시 접속자 | 12명 | 12명 안정 | ✅ 달성 |

---

## 🎨 ADR-003: UI 시스템 - 이벤트 기반 설계 채택

📅 **결정 일자**: 2025년 1월 (Sprint 7)

### 🎯 결정 내용
UI 업데이트 방식을 **Tick 기반**에서 **이벤트 기반**으로 전환

### 📊 상황 분석

**기존 Tick 기반 문제점:**
```cpp
// 성능 문제가 있던 기존 방식
void UBuildingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // 매 프레임마다 실행 (60FPS = 초당 60회!)
    UpdateInventoryDisplay();      // 인벤토리 변경 체크
    UpdateBuildingModeUI();       // 건설 모드 UI 업데이트
    UpdateTimerDisplay();         // 타이머 업데이트
    UpdateScoreDisplay();         // 점수 UI 업데이트
}
```

**성능 측정 결과:**
* UI Tick 함수들이 전체 CPU 사용량의 15% 차지
* 프레임 드롭 발생 (60FPS → 45FPS)
* 불필요한 연산으로 배터리 소모 증가

### 🔍 고려된 대안

| 방식 | 장점 | 단점 | 성능 |
|:---:|:---:|:---:|:---:|
| **Tick 기반** | 구현 간단, 실시간 업데이트 | CPU 과부하, 배터리 소모 | ❌ 나쁨 |
| **Timer 기반** | Tick보다 나음 | 여전히 주기적 실행 | 🔄 보통 |
| **이벤트 기반** | 최고 성능, 반응성 | 초기 구현 복잡 | ✅ 최고 |

### 💡 선택 이유

**1. BlueprintImplementableEvent 활용**
```cpp
// C++에서 이벤트 정의
UCLASS()
class UBuildingComponent : public UActorComponent
{
    // 블루프린트에서 구현할 이벤트
    UFUNCTION(BlueprintImplementableEvent)
    void OnInventoryChanged(const TArray<FItemInfo>& NewInventory);
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnBuildModeToggled(bool bIsBuildMode);
};

// 상태 변경 시에만 이벤트 발생
void UBuildingComponent::AddItem(AItem* NewItem)
{
    Inventory.Add(NewItem);
    OnInventoryChanged(GetInventoryInfo()); // 이벤트 발생
}
```

**2. C++/블루프린트 장점 결합**
* C++: 게임 로직과 성능 최적화
* 블루프린트: UI 배치와 애니메이션

**3. 메모리 효율성**
```cpp
// 스마트 포인터로 메모리 관리
UPROPERTY()
TWeakObjectPtr<UUserWidget> CachedWidget;

void ShowUI()
{
    if (!CachedWidget.IsValid()) {
        CachedWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
    }
    CachedWidget->SetVisibility(ESlateVisibility::Visible);
}
```

### 🎯 구현 결과

**Before/After 비교:**

```cpp
// 🔴 Before: Tick 지옥
void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    // 매 프레임 실행
    if (InventoryComponent) {
        UpdateSlots(InventoryComponent->GetItems());
    }
}

// 🟢 After: 이벤트 기반
void UInventoryComponent::OnItemAdded(AItem* Item)
{
    Items.Add(Item);
    // 변경 시에만 UI 업데이트
    OnInventoryUpdated.Broadcast(Items);
}
```

### 📈 성능 개선 결과

| 메트릭 | 이전 (Tick) | 이후 (Event) | 개선율 |
|:---:|:---:|:---:|:---:|
| CPU 사용량 | 15% | 7% | -53% |
| 평균 FPS | 45 | 58 | +29% |
| UI 업데이트 빈도 | 60회/초 | 실제 변경시만 | -95% |
| 메모리 사용량 | 124MB | 98MB | -21% |

---

## 🧩 ADR-004: 템플릿 기반 건설 시스템 설계

📅 **결정 일자**: 2025년 2월 (Sprint 9 대규모 리팩토링)

### 🎯 결정 내용
건설 시스템을 **하드코딩 방식**에서 **C++ 템플릿 패턴**으로 전환

### 📊 상황 분석

**기존 코드의 심각한 문제:**
```cpp
// 🔴 v1.0: 200줄 중복 코드의 악몽
void UBuildingComponent::BuildPlank()
{
    // 200줄의 하드코딩
    FVector Location = GetPlayerLocation();
    FRotator Rotation = GetPlayerRotation();
    
    // 인벤토리 체크
    if (PlankCount <= 0) return;
    PlankCount--;
    
    // 스폰 로직
    AItem_Plank* NewPlank = GetWorld()->SpawnActor<AItem_Plank>();
    NewPlank->SetActorLocation(Location);
    NewPlank->SetActorRotation(Rotation);
    
    // 물리 설정
    UStaticMeshComponent* MeshComp = NewPlank->GetStaticMeshComponent();
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetSimulatePhysics(true);
    
    // 네트워크 설정
    NewPlank->SetReplicates(true);
    NewPlank->SetReplicateMovement(true);
    
    // ... 계속 반복되는 코드들
}

void UBuildingComponent::BuildTent()
{
    // 거의 동일한 200줄이 또 반복됨!
    FVector Location = GetPlayerLocation();
    FRotator Rotation = GetPlayerRotation();
    
    if (TentCount <= 0) return;
    TentCount--;
    
    AItem_Tent* NewTent = GetWorld()->SpawnActor<AItem_Tent>();
    // ... 동일한 패턴 반복
}
```

**코드 품질 문제:**
* 90줄의 중복 코드 (DRY 원칙 위반)
* 새 아이템 추가 시마다 200줄 복사-붙여넣기
* 버그 수정 시 여러 곳을 동시에 수정해야 함
* 타입 안전성 없음

### 🔍 기술적 진화 과정

**v2.0: 기본 함수 분리**
```cpp
void UBuildingComponent::BuildItem(TSubclassOf<AItem> ItemClass, int32& ItemCount)
{
    if (ItemCount <= 0) return;
    
    // 공통 로직 분리
    ItemCount--;
    AItem* NewItem = SpawnItemActor(ItemClass);
    ConfigurePhysics(NewItem);
    // ... 하지만 여전히 타입 안전성 문제
}
```

**v3.0: C++ 템플릿 패턴 (최종)**
```cpp
// 🟢 혁신적인 템플릿 기반 시스템
template<typename T>
T* UBuildingComponent::SpawnBuildingItem(TSubclassOf<T> ItemClass, const FVector& Location, const FRotator& Rotation)
{
    static_assert(std::is_base_of_v<AItem, T>, "T must be derived from AItem");
    
    if (!ItemClass || !GetWorld()) {
        return nullptr;
    }

    // 타입 안전한 스폰
    T* NewItem = GetWorld()->SpawnActor<T>(ItemClass, Location, Rotation);
    if (!NewItem) return nullptr;

    // 템플릿 특화로 타입별 설정
    ConfigureBuildingItemPhysics(NewItem->GetStaticMeshComponent(), Location, Rotation);
    
    return NewItem;
}

// 사용 예시 - 단 3줄로 모든 아이템 지원!
void UBuildingComponent::BuildPlank()
{
    if (auto* NewPlank = SpawnBuildingItem<AItem_Plank>(PlankClass, BuildLocation, BuildRotation)) {
        OnItemBuilt.Broadcast(NewPlank);
    }
}
```

### 💡 선택 이유

**1. 타입 안전성 보장**
```cpp
// 컴파일 타임에 타입 체크
template<typename T>
void ProcessItem(T* Item)
{
    static_assert(std::is_base_of_v<AItem, T>, "잘못된 타입!");
    // 이제 Item은 100% AItem의 파생 클래스
}
```

**2. 코드 재사용성 극대화**
* 새 아이템 추가 시 3줄로 완료
* 기존 200줄 → 현재 12줄 평균

**3. 성능 최적화**
```cpp
// 템플릿 인라인 최적화
template<typename T>
FORCEINLINE void ConfigureItem(T* Item)
{
    // 컴파일러가 타입별로 최적화된 코드 생성
    Item->SetupPhysics();
}
```

### 🎯 구현 결과

**코드 품질 혁신:**

| 메트릭 | v1.0 (하드코딩) | v2.0 (함수분리) | v3.0 (템플릿) | 개선율 |
|:---:|:---:|:---:|:---:|:---:|
| 코드 라인 수 | 250줄 | 180줄 | 120줄 | **-52%** |
| 중복 코드 | 90줄 | 45줄 | 15줄 | **-83%** |
| 함수 길이 평균 | 45줄 | 25줄 | 12줄 | **-73%** |
| 새 기능 추가 시간 | 2시간 | 45분 | 5분 | **-96%** |

**타입 안전성 향상:**
```cpp
// 🔴 이전: 런타임 에러 가능
AItem* Item = SpawnActor<AItem_Plank>(TentClass); // 잘못된 타입!

// 🟢 현재: 컴파일 타임 체크
auto* Plank = SpawnBuildingItem<AItem_Plank>(PlankClass); // 타입 안전!
```

---

## 📊 전체 아키텍처 결정 영향 분석

### 🎯 누적 개선 효과

| 영역 | Sprint 3 이전 | 현재 (Sprint 14) | 총 개선율 |
|:---:|:---:|:---:|:---:|
| **코드 복잡도** | 평균 20 복잡도 | 평균 6 복잡도 | **-70%** |
| **개발 속도** | 주 1개 기능 | 주 3-4개 기능 | **+300%** |
| **버그 발생률** | 주 5-7개 | 주 1-2개 | **-75%** |
| **성능 (FPS)** | 35-45 FPS | 55-60 FPS | **+50%** |

### 🔮 향후 아키텍처 진화 계획

**Phase 1: 성능 최적화** (예정)
* 메모리 풀링 시스템 도입
* 오브젝트 풀링으로 GC 압박 감소

**Phase 2: ECS 패턴 검토** (검토 중)
* 엔티티-컴포넌트-시스템 아키텍처
* 대규모 멀티플레이어 대비

**Phase 3: 모듈화 심화** (장기)
* 플러그인 시스템 구축
* 모드별 독립 모듈

---

## 🎓 핵심 학습 사항

### 💡 설계 철학

1. **"작동하는 코드 ≠ 좋은 코드"**
   - 초기에는 작동만 하면 된다고 생각했음
   - 리팩토링을 통해 진정한 코드 품질의 중요성 깨달음

2. **SOLID 원칙의 실용성**
   - 이론이 아닌 실제 개발에서 체감되는 효과
   - 특히 SRP(단일 책임)과 DIP(의존성 역전)의 위력

3. **C++ 템플릿의 힘**
   - 타입 안전성 + 성능 + 재사용성의 삼박자
   - 컴파일 타임 최적화의 중요성

### 🚀 성공 요인

1. **점진적 개선**: 한 번에 모든 걸 바꾸지 않고 단계적 접근
2. **측정 기반**: 감이 아닌 실제 메트릭으로 개선 효과 검증  
3. **팀 협업**: 아키텍처 결정 시 팀원들과 충분한 논의
4. **실용성 우선**: 이론보다는 프로젝트에 실제 도움이 되는 선택

---

**다음 문서**: [⚡ 성능 최적화 기록](./performance-optimization.md)