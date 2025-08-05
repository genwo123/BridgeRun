# 브릿지런 개발일지 (스프린트 17)

## 📅 개발 기간
2025년 8월 5일 ~ 2025년 8월 18일 (2주)

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표

스프린트 17에서는 **Steam API 기반 업적 시스템 구축**에 집중합니다:
- **Steam API 래퍼 구현** - Steamworks 문서 기반 기본 구조
- **로컬 통계 추적 시스템** - 플레이어 행동 데이터 수집
- **업적 조건 체크 및 해제** - Steam API 연동 자동화
- **테스트 환경 구축** - 480 AppID로 검증 가능한 시스템

## 2. Steam API 래퍼 시스템 구현

### 2.1 Steamworks 문서 기반 구조

Steamworks 공식 문서의 예제 코드를 기반으로 안정적인 Steam API 래퍼를 구현합니다.

```cpp
// Steam 업적 데이터 구조체
struct Achievement_t {
    int m_eAchievementID;
    const char *m_pchAchievementID;
    char m_rgchName[128];
    char m_rgchDescription[256];
    bool m_bAchieved;
    int m_iIconImage;
};

// Steam API 래퍼 클래스
class CSteamAchievements {
private:
    Achievement_t *m_pAchievements;
    int m_iNumAchievements;
    bool m_bInitialized;
    
public:
    CSteamAchievements(Achievement_t *Achievements, int NumAchievements);
    bool RequestStats();
    bool SetAchievement(const char* ID);
    
    // Steam 콜백 처리
    STEAM_CALLBACK(CSteamAchievements, OnUserStatsReceived, UserStatsReceived_t);
    STEAM_CALLBACK(CSteamAchievements, OnUserStatsStored, UserStatsStored_t);
    STEAM_CALLBACK(CSteamAchievements, OnAchievementStored, UserAchievementStored_t);
};
```

### 2.2 업적 정의 및 관리

```cpp
// 게임별 업적 열거형
enum EAchievements {
    ACH_PLANK_NOVICE = 0,      // 판자 100개 설치
    ACH_PLANK_EXPERT = 1,      // 판자 1,000개 설치  
    ACH_PLANK_MASTER = 2,      // 판자 10,000개 설치
    ACH_FIRST_WIN = 3,         // 첫 승리
    ACH_VETERAN = 4,           // 100승 달성
    ACH_ROOKIE = 5,            // 10게임 플레이
    ACH_PLAYER = 6             // 100게임 플레이
};

// 업적 배열 정의
Achievement_t g_Achievements[] = {
    { ACH_PLANK_NOVICE, "ACH_PLANK_NOVICE", "판자 초보자", "", false, 0 },
    { ACH_PLANK_EXPERT, "ACH_PLANK_EXPERT", "판자 전문가", "", false, 0 },
    { ACH_PLANK_MASTER, "ACH_PLANK_MASTER", "판자의 달인", "", false, 0 },
    { ACH_FIRST_WIN, "ACH_FIRST_WIN", "첫 승리", "", false, 0 },
    { ACH_VETERAN, "ACH_VETERAN", "베테랑", "", false, 0 },
    { ACH_ROOKIE, "ACH_ROOKIE", "루키", "", false, 0 },
    { ACH_PLAYER, "ACH_PLAYER", "플레이어", "", false, 0 }
};
```

## 3. 로컬 통계 추적 시스템

### 3.1 플레이어 통계 구조

```cpp
// 플레이어 통계 데이터
USTRUCT(BlueprintType)
struct FPlayerStatistics
{
    GENERATED_BODY()
    
    // 건설 관련 통계
    UPROPERTY(BlueprintReadWrite, Category = "Building")
    int32 PlanksInstalled = 0;
    
    // 게임 관련 통계  
    UPROPERTY(BlueprintReadWrite, Category = "Games")
    int32 GamesWon = 0;
    
    UPROPERTY(BlueprintReadWrite, Category = "Games")
    int32 TotalGamesPlayed = 0;
    
    // 시간 관련 통계
    UPROPERTY(BlueprintReadWrite, Category = "Time")
    float TotalPlayTimeMinutes = 0.0f;
    
    // 향후 확장용
    UPROPERTY(BlueprintReadWrite, Category = "Extended")
    TMap<FString, int32> CustomStats;
};
```

### 3.2 통계 관리자 클래스

```cpp
UCLASS(BlueprintType)
class BRIDGERUN_API UStatisticsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    // 통계 업데이트
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void IncrementStat(const FString& StatName, int32 Amount = 1);
    
    UFUNCTION(BlueprintCallable, Category = "Statistics")  
    int32 GetStat(const FString& StatName) const;
    
    // 업적 체크
    UFUNCTION(BlueprintCallable, Category = "Achievements")
    void CheckAchievements(const FString& StatName);
    
    // 데이터 저장/로드
    UFUNCTION(BlueprintCallable, Category = "Data")
    void SaveStatistics();
    
    UFUNCTION(BlueprintCallable, Category = "Data")
    void LoadStatistics();
    
protected:
    UPROPERTY()
    FPlayerStatistics PlayerStats;
    
    void UnlockAchievement(const FString& AchievementID);
    bool IsAchievementUnlocked(const FString& AchievementID);
    
    // Steam 연동
    CSteamAchievements* SteamAchievements;
};
```

## 4. 업적 조건 체크 시스템

### 4.1 자동 업적 검증

```cpp
void UStatisticsManager::CheckAchievements(const FString& StatName)
{
    // 판자 설치 관련 업적
    if (StatName == "PlanksInstalled")
    {
        int32 Count = PlayerStats.PlanksInstalled;
        
        if (Count >= 100 && !IsAchievementUnlocked("ACH_PLANK_NOVICE"))
            UnlockAchievement("ACH_PLANK_NOVICE");
            
        if (Count >= 1000 && !IsAchievementUnlocked("ACH_PLANK_EXPERT"))
            UnlockAchievement("ACH_PLANK_EXPERT");
            
        if (Count >= 10000 && !IsAchievementUnlocked("ACH_PLANK_MASTER"))
            UnlockAchievement("ACH_PLANK_MASTER");
    }
    
    // 승리 관련 업적
    else if (StatName == "GamesWon")
    {
        int32 Wins = PlayerStats.GamesWon;
        
        if (Wins >= 1 && !IsAchievementUnlocked("ACH_FIRST_WIN"))
            UnlockAchievement("ACH_FIRST_WIN");
            
        if (Wins >= 100 && !IsAchievementUnlocked("ACH_VETERAN"))
            UnlockAchievement("ACH_VETERAN");
    }
    
    // 게임 플레이 관련 업적
    else if (StatName == "TotalGamesPlayed")
    {
        int32 Games = PlayerStats.TotalGamesPlayed;
        
        if (Games >= 10 && !IsAchievementUnlocked("ACH_ROOKIE"))
            UnlockAchievement("ACH_ROOKIE");
            
        if (Games >= 100 && !IsAchievementUnlocked("ACH_PLAYER"))
            UnlockAchievement("ACH_PLAYER");
    }
}

void UStatisticsManager::UnlockAchievement(const FString& AchievementID)
{
    // Steam API 호출
    if (SteamAchievements)
    {
        SteamAchievements->SetAchievement(TCHAR_TO_ANSI(*AchievementID));
    }
    
    // 로컬 저장
    // ... (향후 구현)
    
    // 알림 표시
    UE_LOG(LogTemp, Warning, TEXT("업적 해제: %s"), *AchievementID);
}
```

## 5. 게임플레이 통계 연동

### 5.1 기존 게임 코드에 통계 업데이트 추가

```cpp
// 판자 설치 시 (기존 BuildingComponent 등에서)
void ABridgeRunCharacter::OnPlankInstalled()
{
    if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
    {
        StatsMgr->IncrementStat("PlanksInstalled");
    }
}

// 게임 승리 시 (GameMode에서)
void ABridgeRunGameMode::OnGameWon(int32 WinningTeamID)
{
    // 승리한 팀의 플레이어들에게 통계 업데이트
    for (auto& PlayerController : WinningTeamPlayers)
    {
        if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
        {
            StatsMgr->IncrementStat("GamesWon");
        }
    }
}

// 게임 완료 시 (모든 플레이어)
void ABridgeRunGameMode::OnGameCompleted()
{
    if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
    {
        StatsMgr->IncrementStat("TotalGamesPlayed");
    }
}
```

### 5.2 GameInstance 초기화 수정

```cpp
// BridgeRunGameInstance::Init() 수정
void UBridgeRunGameInstance::Init()
{
    Super::Init();
    
    // 기존 초기화...
    LoadLocalSettings();
    
    // Steam 업적 시스템 초기화
    InitializeSteamAchievements();
}

void UBridgeRunGameInstance::InitializeSteamAchievements()
{
    if (SteamAPI_Init())
    {
        // Steam 업적 시스템 생성
        g_SteamAchievements = new CSteamAchievements(g_Achievements, ACHIEVEMENT_COUNT);
        
        UE_LOG(LogTemp, Warning, TEXT("Steam 업적 시스템 초기화 성공"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API 초기화 실패"));
    }
}

// 게임 루프에 콜백 처리 추가
void UBridgeRunGameInstance::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Steam 콜백 처리
    SteamAPI_RunCallbacks();
}
```

## 6. 테스트 및 디버깅 시스템

### 6.1 디버그 명령어 추가

```cpp
// 통계 확인용 명령어
UFUNCTION(Exec, BlueprintCallable)
void ShowStats()
{
    if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
    {
        UE_LOG(LogTemp, Warning, TEXT("=== 플레이어 통계 ==="));
        UE_LOG(LogTemp, Warning, TEXT("설치한 판자: %d"), StatsMgr->GetStat("PlanksInstalled"));
        UE_LOG(LogTemp, Warning, TEXT("승리 횟수: %d"), StatsMgr->GetStat("GamesWon"));
        UE_LOG(LogTemp, Warning, TEXT("플레이 게임 수: %d"), StatsMgr->GetStat("TotalGamesPlayed"));
    }
}

// 업적 강제 해제 (테스트용)
UFUNCTION(Exec, BlueprintCallable)
void ForceUnlockAchievement(const FString& AchievementID)
{
    if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
    {
        StatsMgr->UnlockAchievement(AchievementID);
    }
}

// 통계 강제 설정 (테스트용)
UFUNCTION(Exec, BlueprintCallable)
void SetStatForTesting(const FString& StatName, int32 Value)
{
    if (UStatisticsManager* StatsMgr = GetGameInstance()->GetSubsystem<UStatisticsManager>())
    {
        StatsMgr->SetStat(StatName, Value);
        StatsMgr->CheckAchievements(StatName);
    }
}
```

### 6.2 Steam 콘솔 테스트

Steam 클라이언트 콘솔에서 테스트 가능한 명령어들:
```
steam.exe -console 실행 후:

achievement_clear 480 ACH_PLANK_NOVICE    // 업적 초기화
reset_all_stats 480                       // 모든 통계 리셋
```

## 7. 구현 일정 및 우선순위

### 7.1 Week 1 (8/5 ~ 8/11)

**Day 1-2: Steam API 기초 구조**
- [ ] SteamAchievements.h/cpp 구현
- [ ] 기본 Steam API 초기화 테스트
- [ ] Steam SDK 상태 최종 확인

**Day 3-4: 통계 시스템 구축**  
- [ ] PlayerStatistics 구조체 설계
- [ ] UStatisticsManager 클래스 구현
- [ ] 기본 통계 추적 테스트

**Day 5: 초기 연동 테스트**
- [ ] GameInstance에 시스템 통합
- [ ] 간단한 통계 업데이트 테스트

### 7.2 Week 2 (8/12 ~ 8/18)

**Day 6-7: 게임플레이 연동**
- [ ] 기존 게임 코드에 통계 업데이트 추가
- [ ] 업적 조건 체크 로직 구현  
- [ ] 자동 업적 해제 테스트

**Day 8-9: 테스트 및 검증**
- [ ] Steam 480 AppID로 업적 테스트
- [ ] 디버그 명령어 추가
- [ ] 전체 시스템 안정성 검증

**Day 10: 완성 및 정리**
- [ ] 최종 테스트 및 버그 수정
- [ ] 문서 작성 및 정리

## 8. 예상 결과물

### 8.1 구현 완료 시 달성 목표

- **7개 기본 업적** Steam API로 해제 가능
- **로컬 통계 추적** 완전 자동화
- **Steam 콘솔 검증** 가능한 테스트 환경
- **향후 Steam 정식 연동** 준비 완료

### 8.2 생성될 파일 목록

**C++ 클래스 파일:**
```
Source/BridgeRun/Public/Steam/
├── SteamAchievements.h          // Steam API 래퍼
├── PlayerStatistics.h           // 통계 구조체  
└── StatisticsManager.h          // 통계 관리자

Source/BridgeRun/Private/Steam/
├── SteamAchievements.cpp        
├── PlayerStatistics.cpp         
└── StatisticsManager.cpp        
```

**수정될 기존 파일:**
```
├── BridgeRunGameInstance.h/cpp  // Steam 초기화 추가
├── BuildingComponent.cpp        // 판자 설치 통계
├── GameMode.cpp                 // 게임 완료 통계
└── 기타 게임플레이 클래스들     // 통계 업데이트 호출
```

## 9. 성공 지표 및 검증 방법

### 9.1 스프린트 17 완료 기준

- [ ] Steam API 정상 초기화 및 콜백 처리
- [ ] 게임 내 행동 시 통계 자동 업데이트  
- [ ] 조건 달성 시 Steam 업적 자동 해제
- [ ] Steam 클라이언트에서 업적 확인 가능
- [ ] 디버그 명령어로 테스트 가능

### 9.2 향후 확장 준비도

- **Steam 정식 AppID 연동** 시 코드 변경 최소화
- **복잡한 업적 조건** 추가 시 확장 용이성
- **멀티플레이어 환경**에서의 통계 동기화 준비

이번 스프린트를 통해 BridgeRun은 **Steam 플랫폼과 완전히 통합된 업적 시스템**을 갖추게 되며, 향후 정식 Steam 출시를 위한 견고한 기반을 마련하게 됩니다.초기화
- [ ] OnlineSubsystem이 NULL이 아닌 STEAM으로 표시
- [ ] Steam 플레이어 이름 정상 획득
- [ ] 기본 친구 목록 조회 성공
- [ ] 독립 실행과 PIE 모두에서 안정적 동작

### 8.2 품질 기준

- Steam 클라이언트 미실행 시에도 게임 정상 동작
- 오류 발생 시 명확한 사용자 안내 메시지
- 모든 Steam 기능의 graceful fallback 구현

## 9. 회고 및 향후 계획

### 9.1 현재까지의 성과

- Steam SDK 통합 기술적 기반 완성
- 향후 확장 가능한 아키텍처 설계
- 체계적인 테스트 및 진단 도구 구축

### 9.2 장기 비전

스프린트 17은 **Steam 플랫폼 통합의 기초**를 완성하는 단계입니다. 
현재는 기본적인 API 연동과 플레이어 정보 획득에 집중하고, 
실제 업적 시스템과 고급 기능들은 게임이 더 완성된 후 단계별로 구현할 예정입니다.

**"기반은 지금, 활용은 나중에"**의 접근 방식으로 
안정적이고 확장 가능한 Steam 통합 시스템을 구축하겠습니다.