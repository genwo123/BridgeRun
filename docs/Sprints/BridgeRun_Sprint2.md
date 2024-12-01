# 브릿지런 개발일지 (스프린트 2)

## 🗓 개발 기간
2024년 11월 18일 ~ 2024년 12월 1일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

브릿지런의 핵심 게임플레이 요소인 점수 시스템과 다리 건설 시스템을 구현하였습니다. 주요 목표는 다음과 같습니다:
- 트로피와 트로피존을 통한 점수 획득 시스템 구축
- 다릿줄 기반의 다리 건설 시스템 개발
- 판자 설치 및 검증 시스템 구현
- 아이템 자동 생성 시스템 개발

## 2. 점수 시스템 구현

### 2.1 트로피 시스템 개발
![트로피 점수 시스템](./images/sprint2/trophy_score_system.png)

Item 클래스를 상속받아 Trophy 전용 클래스를 구현했습니다. 주요 기능은 다음과 같습니다:
- 트로피 획득 및 운반 메커닉
- 물리 기반 트로피 상호작용
- 트로피존 감지 및 점수 연동

```cpp
void AItem_Trophy::PickUp(ACitizen* Player)
{
    if (!Player) return;
    bIsHeld = true;
    
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    }

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
    }

    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToActor(Player, AttachRules);
    SetActorRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}
```

### 2.2 트로피존 점수 시스템
![트로피존 작동](./images/sprint2/trophy_zone_operation.png)

트로피존은 다음과 같은 기능을 제공합니다:
- 실시간 타이머 시스템으로 점수 획득 시간 측정
- 트로피 보관 시간에 따른 점수 계산
- 텍스트 렌더링을 통한 시각적 피드백

```cpp
void ATrophyZone::OnScoreTimerComplete()
{
    if (!IsValid(PlacedTrophy)) return;
    
    CurrentScore += ScoreAmount;
    FString ScoreString = FString::Printf(TEXT("Score: %d"), CurrentScore);
    ScoreText->SetText(FText::FromString(ScoreString));
    TimerText->SetText(FText::FromString(TEXT("")));
    
    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
}

void ATrophyZone::UpdateTimer()
{
    if (!IsValid(PlacedTrophy) || !IsValid(TimerText)) return;
    
    RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);
    if (RemainingTime > 0.0f)
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}
```

## 3. 다리 건설 시스템

### 3.1 BuildableZone 기본 구조
![다릿줄 시스템](./images/sprint2/buildable_zone_ropes.png)

스플라인 컴포넌트를 활용하여 다음과 같은 로프 시스템을 구현했습니다:
- 상단/하단 로프 각각 좌우 배치 (총 4개의 로프)
- 물리 기반 충돌 처리
- 시각적 디버깅을 위한 로프 렌더링

```cpp
ABuildableZone::ABuildableZone()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // 하단 로프 설정
    LeftBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftBottomRope"));
    LeftBottomRope->SetupAttachment(RootComponent);
    LeftBottomRope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LeftBottomRope->ComponentTags.Add(FName("LeftRope"));

    RightBottomRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightBottomRope"));
    RightBottomRope->SetupAttachment(RootComponent);
    RightBottomRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 0.0f));

    // 상단 로프 설정
    LeftTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("LeftTopRope"));
    LeftTopRope->SetupAttachment(RootComponent);
    LeftTopRope->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    
    RightTopRope = CreateDefaultSubobject<USplineComponent>(TEXT("RightTopRope"));
    RightTopRope->SetupAttachment(RootComponent);
    RightTopRope->SetRelativeLocation(FVector(0.0f, BridgeWidth, 200.0f));
}
```

### 3.2 건설 가능 영역 검증 시스템
![건설 프리뷰](./images/sprint2/build_preview_validation.png)

플레이어의 건설 시도가 유효한지 실시간으로 검증하는 시스템을 구현했습니다:
- 로프 간 거리 기반 설치 가능 여부 판정
- 실시간 프리뷰 메시 색상 변경
- 물리 기반 충돌 검사

```cpp
bool ABuildableZone::IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint)
{
    bool bStartValid = IsPointNearRope(StartPoint, LeftBottomRope);
    bool bEndValid = IsPointNearRope(EndPoint, RightBottomRope);

    if (!bStartValid || !bEndValid)
        return false;

    float Distance = FVector::Distance(StartPoint, EndPoint);
    float MaxAllowedLength = BridgeWidth * 1.5f;
    
    return Distance <= MaxAllowedLength;
}

bool ABuildableZone::IsPointNearRope(const FVector& Point, USplineComponent* Rope, float Tolerance)
{
    return GetDistanceFromRope(Point, Rope) <= Tolerance;
}
```

### 3.3 판자 설치 시스템
![판자 설치](./images/sprint2/plank_placement.png)

BuildingComponent를 통해 실제 판자 설치를 구현했습니다:
- 프리뷰 메시를 통한 실시간 피드백
- 회전 및 위치 조정 기능
- 설치 후 물리 속성 적용

## 4. 아이템 스폰 시스템

### 4.1 ItemSpawnZone 구현
![아이템 스폰 영역](./images/sprint2/item_spawn_zone.png)

자동화된 아이템 생성 및 관리 시스템을 구현했습니다:
- 설정된 간격으로 아이템 자동 생성
- 최대 아이템 수량 관리
- 오버랩 이벤트 기반 아이템 관리

```cpp
void AItemSpawnZone::SpawnItem()
{
    if (CurrentItemCount >= MaxItemCount || ItemsToSpawn.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, ItemsToSpawn.Num() - 1);
    TSubclassOf<AItem> ItemToSpawn = ItemsToSpawn[RandomIndex];
    
    if (ItemToSpawn)
    {
        FVector SpawnLocation = GetRandomPointInVolume();
        FRotator SpawnRotation = FRotator(0.f);
        GetWorld()->SpawnActor<AItem>(ItemToSpawn,
            SpawnLocation,
            SpawnRotation);
    }
}
```

## 5. 컴포넌트 시스템 확장
![컴포넌트 구조](./images/sprint2/component_structure.png)

### 5.1 PlayerModeComponent
- 일반/건설/전투 모드 전환 시스템
- 모드별 입력 처리 분리
- 이벤트 기반 상태 관리

```cpp
void UPlayerModeComponent::SetPlayerMode(EPlayerMode NewMode)
{
    if (CurrentMode != NewMode)
    {
        EPlayerMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnPlayerModeChanged.Broadcast(NewMode, OldMode);
    }
}
```

## 6. 발생한 문제점과 해결

### 6.1 트로피존 상호작용 문제
- 문제: 트로피와 트로피존 간의 상호작용이 일방향으로만 작동
  - 트로피에서 트로피존은 인식되지만 반대는 안 됨
  - 로그를 통한 디버깅 결과 캐스팅 문제로 확인

- 해결: 
  - 콜리전 설정을 C++에서 BP로 이전
  - 컴포넌트 초기화 순서 최적화
  - 트로피존 전용 콜리전 채널 추가

```cpp
// 트로피존 콜리전 설정
if (CollisionComponent)
{
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
}
```

### 6.2 다리 설치 유효성 검사
- 문제: 로프 간격에 따른 설치 가능 여부 판정이 부정확
- 해결: 허용 오차 값 조정 및 거리 계산 로직 개선

## 7. 다음 스프린트 계획
- 전투 시스템 상세 구현
- 팀 시스템 개발
- UI/UX 개선
- 네트워크 기능 준비
- 직업별 특수 능력 구현
