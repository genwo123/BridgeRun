# 브릿지런 개발일지 (스프린트 4)

## 📅 개발 기간
2024년 12월 30일 ~ 2025년 1월 12일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 4에서는 멀티플레이어 게임을 위한 네트워크 시스템 구현과 코드 구조 개선에 집중했습니다:
- 실시간 네트워크 동기화 시스템 구축
- 폴더 구조 재설계 및 정리
- 컴포넌트 간 의존성 개선

## 2. 실시간 네트워크 동기화 시스템

### 2.1 GameState 구현

![블루프린트 상속 오류](plank_blueprint_inheritance_error.JPG)

*블루프린트 상속 구조에서 발생한 네트워크 동기화 문제*

기존의 단일 플레이어 게임에서 멀티플레이어 환경으로 전환하기 위해 GameState를 다음과 같이 구현했습니다:

```cpp
// BridgeRunGameState.h
UCLASS()
class BRIDGERUN_API ABridgeRunGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ABridgeRunGameState();

    UPROPERTY(Replicated, BlueprintReadOnly)
    TArray<FBasicTeamInfo> Teams;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float MatchTime;

    // RPC를 처음 써보면서 _Implementation 패턴을 배웠습니다
    UFUNCTION(NetMulticast, Reliable)
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);
};
```

### 2.2 _Implementation 패턴 학습

![클라이언트 구조 문제](plank_client_structure_anomaly.JPG)

*클라이언트에서 발생한 구조적 문제*

처음으로 언리얼의 RPC 시스템을 사용하면서, 선언과 구현을 분리하는 패턴이 인상적이었습니다:

```cpp
// BuildingComponent.h
UCLASS()
class BRIDGERUN_API UBuildingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable)
    void ServerAttemptBuild();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnBuildComplete();
};

// BuildingComponent.cpp
void UBuildingComponent::ServerAttemptBuild_Implementation()
{
    // 서버에서의 검증 및 실행
    if (HasAuthority() && CanBuild())
    {
        PerformBuildLogic();
        MulticastOnBuildComplete();
    }
}

void UBuildingComponent::MulticastOnBuildComplete_Implementation()
{
    // 모든 클라이언트에서 실행되는 시각적 효과 등
    SpawnBuildEffect();
    UpdateUI();
}
```

### 2.3 프리뷰 시스템 동기화

![프리뷰 위치 오류](plank_preview_position_error.JPG)

*프리뷰 위치가 클라이언트마다 다르게 표시되는 문제*

프리뷰 시스템의 위치 동기화를 위해 RPC 시스템을 활용했습니다:

```cpp
void UBuildingComponent::UpdatePreviewLocation()
{
    if (!PreviewMeshComponent || !OwningPlayer)
        return;

    FHitResult HitResult;
    if (GetBuildLocation(HitResult))
    {
        // 클라이언트에서 서버로 위치 업데이트 요청
        ServerUpdatePreviewLocation(HitResult.Location);
    }
}

void UBuildingComponent::ServerUpdatePreviewLocation_Implementation(const FVector& NewLocation)
{
    // 서버에서 위치 검증
    if (ValidateLocation(NewLocation))
    {
        // 검증된 위치를 모든 클라이언트에 전파
        MulticastSetPreviewLocation(NewLocation);
    }
}
```

### 2.4 서버-클라이언트 동기화 구현

![서버 클라이언트 불일치](plank_server_client_discrepancy.JPG)

*서버와 클라이언트 간의 상태 불일치 문제*

물리 기반 오브젝트의 동기화를 위해 다음과 같은 시스템을 구현했습니다:

```cpp
UCLASS()
class BRIDGERUN_API ABuildableActor : public AActor
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(Server, Reliable)
    void ServerUpdateTransform();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSyncTransform();

protected:
    // 물리 시뮬레이션 결과를 주기적으로 동기화
    virtual void OnRep_ReplicatedMovement() override;
};
```

## 3. 주요 개선 사항

### 3.1 물리 동기화 시스템
- 클라이언트와 서버 간의 물리 상태 동기화
- 네트워크 지연을 고려한 보간 시스템 구현
- 클라이언트 사이드 예측 시스템 적용

### 3.2 RPC 시스템 최적화
- Reliable/Unreliable RPC의 적절한 사용
- 네트워크 부하 최소화를 위한 호출 빈도 조절
- 클라이언트 권한 검증 시스템 구현

## 4. 다음 단계 계획

### 4.1 개선 필요 사항
- 네트워크 지연 보상 시스템 구현
- 물리 동기화 성능 최적화
- 클라이언트 예측 시스템 개선

### 4.2 향후 구현 기능
- 플레이어 간 상호작용 시스템
- 실시간 점수 동기화 시스템
- 팀 밸런싱 시스템

# 브릿지런 개발일지 (스프린트 4) - Part 2

## 1. 폴더 구조 개선

### 1.1 새로운 폴더 구조

프로젝트의 확장성과 유지보수성을 고려하여 다음과 같은 구조로 재구성했습니다:

```plaintext
Source/BridgeRun/
├── vs/
├── Private/
│   ├── Characters/
│   │   ├── BridgeRunCharacter.cpp
│   │   └── Citizen.cpp
│   ├── Core/
│   │   ├── BridgeRun.cpp
│   │   ├── BridgeRunGameMode.cpp
│   │   ├── BridgeRunGameModeBase.cpp
│   │   └── BridgeRunGameState.cpp
│   ├── Item/
│   │   ├── Item.cpp
│   │   ├── Item_Gun.cpp
│   │   ├── Item_Plank.cpp
│   │   ├── Item_Telescope.cpp
│   │   ├── Item_Tent.cpp
│   │   └── Item_Trophy.cpp
│   ├── Modes/
│   │   ├── BuildingComponent.cpp
│   │   ├── CombatComponent.cpp
│   │   ├── InvenComponent.cpp
│   │   └── PlayerModeComponent.cpp
│   └── Zones/
│       ├── BuildableZone.cpp
│       ├── ItemSpawnZone.cpp
│       └── TrophyZone.cpp
└── Public/
    ├── Characters/
    │   ├── BridgeRunCharacter.h
    │   └── Citizen.h
    ├── Core/
    │   ├── BridgeRun.h
    │   ├── BridgeRunGameMode.h
    │   ├── BridgeRunGameModeBase.h
    │   └── BridgeRunGameState.h
    ├── Item/
    │   ├── Item.h
    │   ├── Item_Gun.h
    │   ├── Item_Plank.h
    │   ├── Item_Telescope.h
    │   ├── Item_Tent.h
    │   └── Item_Trophy.h
    ├── Modes/
    │   ├── BuildingComponent.h
    │   ├── CombatComponent.h
    │   ├── InvenComponent.h
    │   ├── PlayerModeComponent.h
    │   └── PlayerModeTypes.h
    └── Zones/
        ├── BuildableZone.h
        ├── ItemSpawnZone.h
        └── TrophyZone.h
```

### 1.2 컴포넌트 의존성 개선
각 컴포넌트의 책임을 명확히 분리하고 의존성을 최소화했습니다:
- CombatComponent: 전투 관련 로직 전담
- BuildingComponent: 건설 시스템 전담
- InvenComponent: 인벤토리 관리 전담

## 2. UI System 개발 시도

CommonUI 플러그인에서 필요한 핵심 기능들을 정리하고 직접 구현을 시도했지만, 몇 가지 문제로 인해 일시적으로 중단했습니다:

### 2.1 필요했던 핵심 기능
1. 스타일 시스템
   - Normal/Hovered/Selected/Disabled 상태별 스타일
   - 각 상태별 이미지 브러시 설정
   - Style 템플릿 저장 및 재사용 기능

2. 사운드 시스템
   - Pressed Sound (클릭 시 사운드)
   - Hovered Sound (마우스 오버 시 사운드)
   - 상태별 사운드 설정

3. 슬롯 시스템
   - 아이템 이미지 표시
   - 수량 오버레이 (Item_Count)
   - 선택 상태 표시

### 2.2 구현 중단 이유
- CommonUI와의 의존성 문제 발생
- Style 에셋 시스템 구현의 복잡성
- 확장성 있는 구조 설계의 어려움

## 3. 다음 스프린트 계획 (스프린트 5)

### 3.1 UI Slate 시스템 재도전
- CommonUI 플러그인 분석 및 필요 기능 선별
- 커스텀 UI 컴포넌트 구현
- 스타일 시스템 설계 및 구현

### 3.2 팀 시스템 구현
- GameModeBase를 활용한 팀 시스템 구현
- 팀별 점수 시스템 구축
- 실시간 점수 동기화 구현
- 승리 조건 및 게임 종료 시스템 구현

### 3.3 네트워크 테스트 계획
- 실제 네트워크 환경에서의 성능 테스트
- 네트워크 지연 상황에서의 동작 검증
- 서버-클라이언트 간 동기화 안정성 테스트

## 4. 회고 및 느낀점

이번 스프린트에서는 처음으로 언리얼 엔진의 네트워크 시스템을 본격적으로 다루면서 많은 것을 배웠습니다. RPC를 통한 서버-클라이언트 통신 방식과 _Implementation을 통한 선언과 구현의 분리는 새로운 경험이었습니다. 

비록 UI Slate 시스템 구현은 잠시 중단되었지만, 다음 스프린트에서는 이를 보완하여 재시도할 예정입니다. 특히 GameModeBase를 활용한 팀 시스템 구현과 승리 조건 설정을 통해 게임의 기본 틀을 완성하는 것이 주요 목표입니다.
