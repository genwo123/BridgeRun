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

기존의 단일 플레이어 게임에서 멀티플레이어 환경으로 전환하기 위해 GameState를 구현했습니다:

```cpp
// BridgeRunGameState.h
USTRUCT(BlueprintType)
struct FBasicTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 TeamId;

    UPROPERTY(BlueprintReadWrite)
    int32 Score;

    FBasicTeamInfo()
    {
        TeamId = 0;
        Score = 0;
    }
};

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

    UFUNCTION(NetMulticast, Reliable)
    virtual void UpdateTeamScore(int32 TeamId, int32 NewScore);
};
```

### 2.2 인벤토리 시스템 동기화

인벤토리 시스템에 네트워크 동기화 기능을 추가했습니다:

```cpp
// InvenComponent.h
UCLASS()
class BRIDGERUN_API UInvenComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EInventorySlot CurrentSelectedSlot;

    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData PlankData;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void UpdateItemCount(EInventorySlot Slot, int32 Amount);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

### 2.3 전투 시스템 동기화

전투 시스템의 상태와 액션을 모든 클라이언트에 동기화하도록 개선했습니다:

```cpp
// CombatComponent.h
UCLASS()
class BRIDGERUN_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated)
    bool bHasGun;

    UPROPERTY(Replicated)
    class AItem_Telescope* EquippedTelescope;

    UFUNCTION(Server, Reliable)
    void HandleShoot();

    UFUNCTION(Server, Reliable)
    void HandleAim();
};
```

실제 구현 시 발생했던 주요 문제점들:
1. Item 복제 문제
   - 문제: 아이템 상태가 클라이언트 간에 동기화되지 않음
   - 해결: Replicated 속성과 RPC 함수를 적절히 조합하여 해결

2. 전투 액션 동기화 문제
   - 문제: 발사와 조준이 다른 클라이언트에 즉시 반영되지 않음
   - 해결: Server RPC를 통한 신뢰성 있는 동기화 구현

## 3. 폴더 구조 개선

### 3.1 새로운 폴더 구조

프로젝트의 확장성과 유지보수성을 고려하여 다음과 같은 구조로 재구성했습니다:

```plaintext
Source/BridgeRun/
├── Core/                  // 핵심 게임 시스템
│   ├── BridgeRun.h/.cpp
│   ├── BridgeRunGameMode.h/.cpp
│   └── BridgeRunGameState.h/.cpp
│
├── Characters/            // 캐릭터 관련
│   ├── BridgeRunCharacter.h/.cpp
│   └── Citizen.h/.cpp
│
├── Modes/                 // 게임 모드 시스템
│   ├── Components/
│   │   ├── PlayerModeComponent.h/.cpp
│   │   ├── CombatComponent.h/.cpp
│   │   ├── BuildingComponent.h/.cpp
│   │   └── InvenComponent.h/.cpp
│   └── PlayerModeTypes.h
│
├── Items/                 // 아이템 시스템
│   ├── Item.h/.cpp
│   ├── Item_Gun.h/.cpp
│   ├── Item_Plank.h/.cpp
│   ├── Item_Telescope.h/.cpp
│   ├── Item_Tent.h/.cpp
│   └── Item_Trophy.h/.cpp
│
└── Zones/                 // 게임플레이 구역
    ├── BuildableZone.h/.cpp
    ├── ItemSpawnZone.h/.cpp
    └── TrophyZone.h/.cpp
```

### 3.2 경로 수정 작업

모든 include 경로를 새로운 구조에 맞게 수정했습니다:

```cpp
// 기존 코드
#include "Item.h"
#include "PlayerModeComponent.h"

// 수정된 코드
#include "Items/Item.h"
#include "Modes/Components/PlayerModeComponent.h"
```

### 3.3 컴포넌트 의존성 개선

각 컴포넌트의 책임을 명확히 분리하고 의존성을 최소화했습니다:
- CombatComponent: 전투 관련 로직 전담
- BuildingComponent: 건설 시스템 전담
- InvenComponent: 인벤토리 관리 전담

## 4. 다음 스프린트 계획

### 4.1 Zone 시스템 네트워크 동기화
- BuildableZone의 멀티플레이어 지원
- TrophyZone 점수 시스템 동기화
- ItemSpawnZone 스폰 로직 네트워크 처리

### 4.2 실제 네트워크 환경 테스트
- 서버-클라이언트 간 지연 시간 측정
- 네트워크 최적화
- 버그 수정 및 안정화

이번 스프린트에서 기본적인 네트워크 구조를 구축했으며, 다음 스프린트에서는 Zone 시스템의 네트워크 동기화와 실제 환경에서의 테스트를 진행할 예정입니다.


# 브릿지런 개발일지 (스프린트 5)

## 📅 개발 기간
2025년 1월 1일 ~ 2025년 1월 14일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 5에서는 네트워크 동기화와 아이템 시스템의 안정화에 집중했습니다:
- 멀티플레이어 환경에서의 아이템 동기화
- Trophy 시스템 개선
- Item 클래스 구조 재설계

## 2. Item 시스템 개선

### 2.1 기본 클래스 재설계
![아이템 동기화 문제](./images/sprint5/item_sync_issue.jpg)

*초기 발생한 아이템 동기화 문제*

기존의 Item 클래스를 네트워크 지원 구조로 개선했습니다:

```cpp
UCLASS()
class BRIDGERUN_API AItem : public AActor
{
    GENERATED_BODY()
    
public:
    // 복제될 속성들
    UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
    bool bIsPickedUp;

    UPROPERTY(ReplicatedUsing = OnRep_OwningPlayer)
    class ACharacter* OwningPlayer;

    // 네트워크 RPC 함수들
    UFUNCTION(Server, Reliable)
    void PickUp(ACharacter* Character);

    UFUNCTION(Server, Reliable)
    void Drop();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnPickedUp(ACharacter* NewOwner);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnDropped();
};
```

### 2.2 Transform 시스템 도입

아이템 위치/회전 문제 해결을 위한 새로운 Transform 시스템을 구현했습니다:

```cpp
// Item.h
UFUNCTION(BlueprintNativeEvent, Category = "Item")
FTransform GetPickupTransform(ACharacter* Player) const;

// Item_Trophy.cpp
FTransform AItem_Trophy::GetPickupTransform_Implementation(ACharacter* Player) const
{
    return FTransform(FRotator::ZeroRotator, FVector(100.0f, 0.0f, 50.0f));
}

// Item_Gun.cpp
FTransform AItem_Gun::GetPickupTransform_Implementation(ACharacter* Player) const
{
    return FTransform(Player->GetActorRotation(), FVector(30.0f, 0.0f, 0.0f));
}
```

## 3. Trophy 시스템 구현

### 3.1 점수 시스템
![트로피 점수 시스템](./images/sprint5/trophy_score_system.jpg)

*트로피 점수 획득 시스템*

```cpp
void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    if (!HasAuthority()) return;

    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}
```

### 3.2 타이머 시스템 개선
![타이머 동기화](./images/sprint5/trophy_timer_sync.jpg)

*실시간 타이머 동기화 구현*

초기 버전의 문제점:
```cpp
// 문제가 있던 초기 구현
UPROPERTY(Replicated)
float RemainingTime;
```

개선된 버전:
```cpp
// 개선된 구현
UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
float RemainingTime;

void ATrophyZone::OnRep_RemainingTime()
{
    if (IsValid(TimerText))
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}
```

## 4. 발생한 문제점과 해결

### 4.1 아이템 부착 문제
![아이템 부착 문제](./images/sprint5/item_attachment_issue.jpg)

*클라이언트별 아이템 부착 불일치*

초기 발생한 문제점:
- 클라이언트마다 아이템 위치가 다르게 보이는 현상
- 캐릭터와 아이템이 겹치는 현상
- 아이템별 고유 위치가 적용되지 않는 문제

문제 원인 분석:
```cpp
// 문제가 있던 코드
void AItem::AttachToPlayer(ACharacter* Player)
{
    MeshComponent->AttachToComponent(
        Player->GetMesh(),
        AttachRules
    );
}
```

해결 방안:
```cpp
void AItem::AttachToPlayer(ACharacter* Player)
{
    if (!IsValid(Player)) return;

    // Actor 전체를 부착
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    AttachToActor(Player, AttachRules);

    // 아이템별 Transform 적용
    FTransform ItemTransform = GetPickupTransform(Player);
    SetActorRelativeLocation(ItemTransform.GetLocation());
    SetActorRelativeRotation(ItemTransform.GetRotation());
}
```

### 4.2 타이머 시각화 문제

초기 문제점:
- 타이머가 작동하지만 UI에 표시되지 않는 현상
- 클라이언트 간 타이머 표시 불일치

문제 해결:
```cpp
void ATrophyZone::UpdateTimer()
{
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    if (UWorld* World = GetWorld())
    {
        RemainingTime = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);
        
        // 타이머 텍스트 업데이트
        if (IsValid(TimerText))
        {
            FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
            TimerText->SetText(FText::FromString(TimerString));
        }

        ForceNetUpdate();
    }
}
```

## 5. 다음 스프린트 계획

### 5.1 개선 필요 사항
- 아이템 충돌 시스템 개선
- 네트워크 성능 최적화
- BuildableZone과의 연동 강화

### 5.2 신규 기능 계획
- 아이템 타입 확장
- 팀 시스템 구현
- UI/UX 개선

이번 스프린트를 통해 멀티플레이어 환경에서의 아이템 시스템 안정성이 크게 향상되었습니다. 특히 Trophy 시스템의 실시간 동기화 구현으로 게임의 핵심 메커니즘이 더욱 견고해졌습니다.
