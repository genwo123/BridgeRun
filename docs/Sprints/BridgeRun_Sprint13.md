# 브릿지런 개발일지 (스프린트 13)

## 📅 개발 기간
2025년 5월 5일 ~ 2025년 5월 18일 (2주)

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 13에서는 시스템 안정화와 코드 품질 개선에 집중했습니다:
- 기존 시스템들의 네트워크 동기화 안정화
- SOLID 원칙 기반 코드 구조 개선
- 향후 확장을 위한 팀/로그 시스템 기반 구축
- 아이템 스폰 시스템의 물리적 완성도 향상

## 2. 아이템 스폰 시스템 개선

### 2.1 문제 상황
기존 아이템 스폰 시 다음과 같은 문제들이 발생했습니다:
- 아이템이 땅에 매장되거나 공중에 떠있음
- 부자연스러운 즉시 배치로 인한 몰입감 저하
- 클라이언트 간 최종 위치 불일치

![아이템 스폰 문제 상황](./images/sprint13/item_spawn_issues.jpg)
*기존 아이템 스폰의 문제점들*

### 2.2 개선된 스폰 플로우
```
아이템 스폰 개선 과정:
[스폰 요청] → [높이 조정] → [물리 활성화] → [자연 낙하] → [안정화]
     ↓            ↓            ↓             ↓           ↓
   공중스폰    지면+50cm    중력+임펄스    1초간 낙하   물리 비활성화
```

### 2.3 핵심 개선 코드
```cpp
// 자연스러운 스폰 로직의 핵심
void AItemSpawnZone::SpawnItemAtLocation_Implementation(TSubclassOf<AItem> ItemClass, FVector Location)
{
    // 1. 안전한 높이에서 스폰
    FVector AdjustedLocation = Location;
    AdjustedLocation.Z += 50.0f;
    
    AItem* SpawnedItem = GetWorld()->SpawnActor<AItem>(ItemClass, AdjustedLocation, FRotator::ZeroRotator);
    
    if (SpawnedItem && SpawnedItem->MeshComponent)
    {
        // 2. 물리 시뮬레이션으로 자연스러운 낙하
        SpawnedItem->MeshComponent->SetSimulatePhysics(true);
        SpawnedItem->MeshComponent->SetEnableGravity(true);
        
        // 3. 랜덤 임펄스로 현실감 추가
        FVector RandomImpulse = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(50.0f, 150.0f)
        );
        SpawnedItem->MeshComponent->AddImpulse(RandomImpulse);
        
        // 4. 1초 후 안정화
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [SpawnedItem]() {
            if (SpawnedItem && SpawnedItem->MeshComponent)
            {
                SpawnedItem->MeshComponent->SetSimulatePhysics(false);
            }
        }, 1.0f, false);
    }
}
```

![개선된 아이템 스폰](./images/sprint13/improved_item_spawn.gif)
*자연스럽게 떨어져서 안착하는 아이템들*

### 2.4 개선 결과
- **시각적 품질 향상**: 아이템이 자연스럽게 땅에 안착
- **물리적 현실감**: 스폰 시 자연스러운 낙하 애니메이션  
- **네트워크 안정성**: 모든 클라이언트에서 동일한 최종 위치 보장

## 3. 점수 시스템 고도화

### 3.1 트로피 시각적 피드백 시스템

트로피존에서 점수 변화를 즉시 시각적으로 확인할 수 있도록 개선했습니다.

![트로피 색상 변경 시스템](./images/sprint13/trophy_color_system.jpg)
*팀별 점수에 따라 실시간으로 변하는 트로피 색상*

### 3.2 시스템 구조도
```
점수 변경 플로우:
[점수 변경] → [최고점 팀 계산] → [색상 결정] → [머티리얼 업데이트] → [시각적 반영]
     ↓             ↓               ↓             ↓                ↓
  팀별 점수     우선순위 정렬    팀컬러 매핑   다이나믹 머티리얼   즉시 적용
```

### 3.3 핵심 구현 코드
```cpp
// 트로피 색상 실시간 업데이트
void ATrophyZone::UpdateTrophyVisuals_Implementation()
{
    // 최고 점수 팀 찾기
    int32 HighestScore = 0;
    int32 LeadingTeam = -1;
    
    for (const auto& TeamPair : Teams)
    {
        if (TeamPair.Value > HighestScore)
        {
            HighestScore = TeamPair.Value;
            LeadingTeam = TeamPair.Key;
        }
    }

    // 즉시 색상 반영
    if (LeadingTeam != -1 && TeamColors.Contains(LeadingTeam))
    {
        UMaterialInstanceDynamic* DynamicMaterial = TrophyMesh->CreateDynamicMaterialInstance(0);
        DynamicMaterial->SetVectorParameterValue("TeamColor", TeamColors[LeadingTeam]);
        TrophyMesh->SetMaterial(0, DynamicMaterial);
    }
}

// 점수 변경 시 즉시 시각 업데이트 트리거
void ATrophyZone::AddScore_Implementation(int32 TeamId, int32 Points)
{
    Teams.Contains(TeamId) ? Teams[TeamId] += Points : Teams.Add(TeamId, Points);
    
    UpdateTrophyVisuals(); // 즉시 반영
    OnScoreChanged.Broadcast(TeamId, Teams[TeamId]);
}
```

![점수 시스템 동작](./images/sprint13/scoring_system_demo.gif)
*실시간으로 변화하는 트로피 색상과 점수 UI*

## 4. 코드 구조 개선

### 4.1 리팩토링 배경
기존 코드의 주요 문제점들:
- 하나의 함수가 여러 책임을 담당 (단일 책임 원칙 위반)
- 중복 코드로 인한 유지보수 어려움
- 네트워크 동기화 패턴의 불일치

![코드 구조 개선 전후](./images/sprint13/code_structure_improvement.jpg)
*SOLID 원칙 적용 전후 비교*

### 4.2 파일 구조 모듈화
```
리팩토링 전후 구조:

Before (문제상황):
PlayerModeComponent.cpp (거대한 파일)
├── 건설 로직 (200줄)
├── 전투 로직 (150줄)  
├── 네트워크 로직 (100줄)
└── 기타 로직 (50줄)

After (개선결과):
PlayerModeComponent/ 
├── PlayerModeComponent.h (인터페이스 정의)
├── PlayerModeComponent.cpp (기본 로직)
├── PlayerModeComponent_Building.cpp (건설 전담)
├── PlayerModeComponent_Combat.cpp (전투 전담)
└── PlayerModeComponent_Network.cpp (네트워크 전담)
```

### 4.3 중복 코드 제거
```cpp
// 개선 전: 중복이 많았던 구조
void ProcessPlankBuild() { /* 50줄의 비슷한 로직 */ }
void ProcessTentBuild() { /* 거의 동일한 50줄 */ }

// 개선 후: 템플릿화된 통합 함수
template<typename T>
void ProcessItemBuild(EInventorySlot ItemType, TSubclassOf<T> ItemClass)
{
    if (!ValidateBuildConditions(ItemType)) return;
    
    T* SpawnedItem = SpawnBuildItem<T>(ItemClass);
    if (SpawnedItem)
    {
        ConfigureSpawnedItem(SpawnedItem);
        ConsumeInventoryItem(ItemType);
        UpdateBuildingState();
    }
}
```

### 4.4 네트워크 동기화 표준화
```cpp
// 표준화된 리플리케이션 패턴 적용
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMyActor, ReplicatedProperty);
    DOREPLIFETIME_CONDITION(AMyActor, ConditionalProperty, COND_OwnerOnly);
}

// 일관된 RPC 명명 규칙
UFUNCTION(Server, Reliable)
void ServerFunction_Implementation();

UFUNCTION(NetMulticast, Reliable)  
void MulticastFunction_Implementation();
```

![리팩토링 성과](./images/sprint13/refactoring_results.jpg)
*83% 중복 코드 감소, 모듈별 명확한 책임 분리*

## 5. 팀 시스템 인프라 확장

### 5.1 확장 가능한 팀 관리 구조

향후 로비 시스템과의 연동을 위한 견고한 팀 관리 기반을 구축했습니다.

![팀 시스템 아키텍처](./images/sprint13/team_system_architecture.jpg)
*확장 가능한 팀 관리 시스템 구조*

### 5.2 팀 관리 플로우
```
팀 배정 시스템:
[플레이어 요청] → [유효성 검사] → [중복 제거] → [팀 배정] → [상태 업데이트]
      ↓              ↓              ↓            ↓            ↓
   팀 변경 요청    정원/권한 확인   기존팀 탈퇴    새팀 합류    UI 갱신
```

### 5.3 핵심 구현
```cpp
// 안정적인 팀 배정 로직
bool UBridgeRunGameInstance::AssignPlayerToTeam(const FString& PlayerName, int32 TeamId)
{
    // 1. 유효성 검사
    if (PlayerName.IsEmpty() || TeamId < 0 || TeamId >= AvailableTeams.Num())
        return false;
    
    FTeamInfo& Team = AvailableTeams[TeamId];
    
    // 2. 팀 정원 및 중복 참가 확인
    if (Team.PlayerNames.Num() >= Team.MaxPlayers || Team.PlayerNames.Contains(PlayerName))
        return false;
    
    // 3. 다른 팀에서 제거 (팀 변경 지원)
    for (FTeamInfo& OtherTeam : AvailableTeams)
        OtherTeam.PlayerNames.Remove(PlayerName);
    
    // 4. 새 팀에 배정
    Team.PlayerNames.Add(PlayerName);
    return true;
}
```

![팀 배정 UI](./images/sprint13/team_assignment_ui.jpg)
*실시간 팀 배정 및 정원 표시 UI*

## 6. 로그 시스템 설계

### 6.1 데이터 수집 아키텍처

게임 플레이 분석을 위한 포괄적인 로그 시스템을 설계했습니다.

### 6.2 로그 시스템 구조도
```
데이터 수집 플로우:
[게임 액션] → [로그 생성] → [메모리 저장] → [분석 처리] → [결과 출력]
     ↓            ↓            ↓             ↓             ↓
  플레이어 행동   구조화 데이터   임시 버퍼    통계 계산    리포트 생성
```

### 6.3 로그 데이터 구조
```cpp
// 확장 가능한 로그 시스템
USTRUCT(BlueprintType)
struct FPlayerActionLog
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;
    
    UPROPERTY(BlueprintReadWrite)
    FString ActionType; // "Build", "Combat", "Item_Use"
    
    UPROPERTY(BlueprintReadWrite)
    FVector Location;
    
    UPROPERTY(BlueprintReadWrite)
    float TimeStamp;
    
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, FString> AdditionalData; // 확장 가능한 메타데이터
};

// 분석 기능 예시
UFUNCTION(BlueprintCallable)
float GetPlayerCombatAccuracy(const FString& PlayerName)
{
    int32 Shots = GetActionCount(PlayerName, "Combat_Shoot");
    int32 Hits = GetActionCount(PlayerName, "Combat_Hit");
    return Shots > 0 ? (float)Hits / Shots : 0.0f;
}
```

### 6.4 단계적 구현 계획
```
로그 시스템 로드맵:
Phase 1 (현재) → Phase 2 (다음) → Phase 3 (향후) → Phase 4 (장기)
      ↓              ↓              ↓              ↓
   구조 설계      실시간 수집     파일 저장     외부 연동
```

![로그 시스템 설계](./images/sprint13/log_system_design.jpg)
*포괄적인 게임플레이 데이터 수집 시스템*:

1. **1단계 (현재)**: 데이터 구조 설계 및 기본 수집 로직
2. **2단계**: 게임 내 실시간 로그 수집 구현
3. **3단계**: 로컬 파일 저장 및 세션 관리
4. **4단계**: 외부 분석 도구 연동

## 7. 소규모 개선사항

### 7.1 크로스헤어 시스템 구현

전투 시스템의 사용성 향상을 위한 기본 크로스헤어를 구현했습니다:

```cpp
// CrosshairWidget.cpp - 기본 크로스헤어
void UCrosshairWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 화면 중앙 고정
    SetAnchorsInViewport(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
}

void UCrosshairWidget::UpdateCrosshairState(bool bIsAiming)
{
    if (bIsAiming)
    {
        // 조준 시 정밀 크로스헤어
        CrosshairImage->SetBrushSize(FVector2D(20.0f, 20.0f));
        CrosshairImage->SetColorAndOpacity(FLinearColor::Red);
    }
    else
    {
        // 기본 크로스헤어
        CrosshairImage->SetBrushSize(FVector2D(32.0f, 32.0f));
        CrosshairImage->SetColorAndOpacity(FLinearColor::White);
    }
}
```

### 7.2 원형 프로그레스 바 기초 작업

건설 진행도 표시를 위한 원형 프로그레스 바 기초를 작업했습니다:

```cpp
// CircularProgressBar.h - 기본 구조
UCLASS()
class BRIDGERUN_API UCircularProgressBar : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable)
    void SetProgress(float NewProgress); // 0.0f ~ 1.0f
    
    UFUNCTION(BlueprintCallable)
    void SetProgressColor(FLinearColor NewColor);
    
protected:
    UPROPERTY(meta = (BindWidget))
    class UImage* ProgressImage;
    
    UPROPERTY(EditAnywhere, Category = "Progress")
    UMaterialInterface* ProgressMaterial;
    
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicProgressMaterial;
};
```

## 8. 다음 스프린트 계획

### 8.1 공모전 데모 준비 (스프린트 14)
- 2분 데모에 최적화된 게임 플로우 구현
- 명확한 승부 판정 시스템
- 시각적 완성도 향상

### 8.2 동적 팀 UI 시스템
- 활성화된 팀 수에 따른 가변적 UI
- 실시간 점수 표시 시스템
- 팀별 색상 구분 강화

### 8.3 게임 종료 시스템
- 경기 결과 화면 구현
- 팀별 순위 및 개인 성과 표시
- 재경기 및 로비 복귀 기능

## 9. 회고 및 학습 포인트

### 9.1 성취사항

이번 스프린트에서 가장 큰 성과는 **시스템 안정화**였습니다:

- **코드 품질 향상**: SOLID 원칙 적용으로 유지보수성 크게 개선
- **물리 시스템 완성도**: 아이템 스폰의 자연스러운 동작 구현
- **확장 가능한 구조**: 팀 시스템과 로그 시스템의 견고한 기반 구축

### 9.2 기술적 깊이

- **네트워크 동기화**: 리플리케이션 패턴의 체계적 적용
- **템플릿 프로그래밍**: C++ 템플릿을 활용한 코드 중복 제거
- **시스템 설계**: 미래 확장을 고려한 아키텍처 설계

### 9.3 다음 스프린트 준비

공모전 데모를 위한 준비가 잘 갖춰졌습니다:
- 안정적인 코어 시스템
- 명확한 게임플레이 흐름을 위한 기반
- 빠른 기능 추가가 가능한 구조

스프린트 13을 통해 "기술적 부채"를 상당히 해결하고, 다음 단계의 빠른 개발을 위한 탄탄한 기반을 마련했습니다.