# 브릿지런 개발일지 (스프린트 11)

## 📅 개발 기간
2025년 4월 07일 ~ 2025년 4월 20일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표
스프린트 11에서는 로비 시스템 통합 및 개선에 집중했습니다:

* 마켓플레이스에서 구매한 SimpleLobbySystem 플러그인 통합
* 브릿지런의 기존 시스템과 연동 (BP_Citizen 캐릭터 등)
* 팀 선택 및 컬러 시스템 구현
* 방 생성 및 참가 관련 버그 수정
* 데디케이티드 서버 환경으로 전환하여 세션 관리 안정화

## 2. 로비 시스템 통합 배경 및 필요성

### 2.1 기능의 필요성
브릿지런은 팀 기반 전략 액션 게임으로, 여러 플레이어가 멀티플레이어로 참여하여 팀을 이루어 경쟁하는 게임입니다. 이에 필요한 핵심 기능은 다음과 같습니다:

* 플레이어들이 모이는 로비 시스템
* 방 생성 및 참가 기능
* 팀 선택 및 게임 세션 관리
* 게임 시작 및 맵 전환 메커니즘

기존에는 이러한 기능이 부재하여 테스트 목적으로만 멀티플레이어를 구현했지만, 이제는 실제 게임으로 발전시키기 위해 완전한 로비 시스템이 필요했습니다.

### 2.2 구매 및 통합 결정
로비 시스템을 처음부터 직접 개발하려면 많은 시간과 노력이 필요하다고 판단했습니다. 따라서 마켓플레이스에서 SimpleLobbySystem 플러그인을 구매하여 이를 기반으로 브릿지런에 맞게 수정하기로 결정했습니다. 이를 통해:

* 개발 시간 단축
* 기본적인 네트워킹 및 세션 관리 기능 활용
* 안정적인 기반 위에 브릿지런만의 기능 추가

라는 이점을 얻을 수 있었습니다.

## 3. SimpleLobbySystem 분석

SimpleLobbySystem 플러그인을 구매한 후, 첫 단계로 시스템 구조와 핵심 컴포넌트를 분석했습니다.

### 3.1 플러그인 구조
플러그인은 다음과 같은 주요 컴포넌트로 구성되어 있었습니다:

* **게임 모드 클래스**:
  - GM_Lobby: 로비 화면 관리
  - GM_Menu: 메인 메뉴 관리
  - GM_Game: 실제 게임플레이 관리

* **플레이어 관련 클래스**:
  - BP_Player: 기본 플레이어 블루프린트
  - BP_DisplayPlayer: 플레이어 시각적 표현
  - PC_Lobby/PC_Game/PC_Menu: 각 모드별 플레이어 컨트롤러

* **UI 위젯**:
  - UMG_Menu: 메인 메뉴 인터페이스
  - UMG_Lobby: 로비 화면 및 방 목록
  - W_PlayerItem/W_PlayerName: 플레이어 정보 표시
  - W_ChatMessage: 채팅 시스템
  - W_RoomSession: 방 세션 관리

* **게임 상태 클래스**:
  - GS_Lobby: 로비 상태 관리

### 3.2 초기 문제점 식별
SimpleLobbySystem을 브릿지런에 통합하여 테스트하면서 다음과 같은 문제점을 발견했습니다:

1. **맵 전환 이슈**: 방에 참가할 때 클라이언트들이 자동으로 게임 맵으로 전환되는 문제
2. **캐릭터 호환성**: 기본 BP_Player와 브릿지런의 BP_Citizen 간 차이로 인한 호환성 문제
3. **세션 동기화**: 방장이 아닌 클라이언트가 방을 생성한 경우 세션 동기화 문제
4. **UI 제한**: 팀 색상 선택 및 팀 밸런스 검증 기능 부재

## 4. 통합 과정 및 문제 해결

### 4.1 맵 전환 이슈 해결
가장 먼저 맵 전환 이슈를 발견했습니다. 서버+클라이언트 구성에서 방에 입장할 때, 다른 클라이언트들이 자동으로 디폴트 맵으로 전환되는 심각한 문제였습니다.

#### 원인 분석
문제의 원인을 분석한 결과, 이 문제는 "리스닝 서버(Listen Server)" 환경에서만 발생하는 현상임을 확인했습니다:

1. 언리얼 엔진에서 Join Session 함수는 성공 시 자동으로 ClientTravel 함수를 호출
2. 리스닝 서버 환경에서는 서버와 클라이언트가 한 프로세스에 있어 자동 맵 전환 발생
3. 이로 인해 일부 클라이언트만 맵 전환되어 게임 세션 불일치 발생

#### 해결 방법
여러 해결책을 검토한 결과, 데디케이티드 서버(Dedicated Server) 방식으로 전환하는 것이 최선의 방법임을 확인했습니다:

```bash
# 데디케이티드 서버 실행 명령어
UE4Editor.exe BridgeRun ThirdPersonExampleMap -server -log
```

데디케이티드 서버 환경에서는:
- 서버에 PlayerController가 없으므로 ClientTravel 호출 대상이 없음
- 모든 클라이언트가 서버의 상태에 따라 올바르게 맵 전환됨
- 더 안정적인 멀티플레이어 환경 제공

이 변경으로 맵 전환 이슈를 근본적으로 해결했습니다.

### 4.2 BP_Citizen 통합
다음으로 SimpleLobbySystem의 기본 캐릭터(BP_Player)를 브릿지런의 BP_Citizen으로 대체하는 작업을 진행했습니다.

#### BP_Citizen 수정
BP_Citizen에 필요한 변수와 위젯 컴포넌트를 추가했습니다:

```cpp
// BP_Citizen에 추가된 변수
UPROPERTY(Replicated, BlueprintReadWrite)
FString PlayerName;

UPROPERTY(Replicated, BlueprintReadWrite)
int32 SkinIndex;

UPROPERTY(Replicated, BlueprintReadWrite)
FString PlayerID;

UPROPERTY(Replicated, BlueprintReadWrite)
int32 TeamID;

// 위젯 컴포넌트 추가
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
class UWidgetComponent* NameTagWidget;
```

#### 플레이어 스폰 로직 수정
GM_Lobby와 GM_Game에서 플레이어 스폰 로직을 수정하여 BP_Player 대신 BP_Citizen을 스폰하도록 변경했습니다:

```cpp
// 기존 코드
ACharacter* SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(BP_Player, ...);

// 수정된 코드
ACharacter* SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(BP_Citizen, ...);
```

블루프린트에서도 GetActorOfClass 노드의 클래스 참조를 BP_Citizen으로 변경했습니다.

#### 위젯 시스템 연동
BP_Player의 위젯 시스템을 BP_Citizen에 통합했습니다:

1. BP_Citizen에 위젯 컴포넌트 추가
2. BeginPlay 이벤트에서 위젯 초기화 로직 복제
3. 플레이어 이름 표시 기능 구현

![BP_Citizen 위젯 구현](./docs/Sprints/images/sprint11/bp_citizen_widget.png)

### 4.3 팀 시스템 구현
브릿지런의 핵심 기능인 팀 시스템을 구현했습니다.

#### 팀 색상 정의
4개의 팀을 위한 색상을 정의했습니다:

```cpp
// 팀 색상 정의
TeamColors.Add(FLinearColor(1.0f, 0.2f, 0.2f)); // 빨강
TeamColors.Add(FLinearColor(0.2f, 0.4f, 1.0f)); // 파랑
TeamColors.Add(FLinearColor(1.0f, 1.0f, 0.2f)); // 노랑
TeamColors.Add(FLinearColor(0.2f, 0.8f, 0.2f)); // 초록
```

#### 팀 선택 UI
방 내부 UI에 팀 선택 기능을 추가했습니다:

1. W_RoomSession 위젯에 팀 선택 패널 추가
2. 각 팀 버튼 UI 구현 및 이벤트 연결
3. 선택된 팀 시각화 및 플레이어 목록 구성

#### 팀 밸런스 검증
게임 시작 전 팀 밸런스를 검증하는 로직을 추가했습니다:

```cpp
bool ULobbyGameInstance::ValidateTeamBalance() const
{
    // 팀별 플레이어 수 계산
    TArray<int32> TeamCounts;
    TeamCounts.SetNum(4);
    
    for (const FPlayerInfo& Player : CurrentRoom.Players)
    {
        if (Player.TeamID >= 0 && Player.TeamID < 4)
        {
            TeamCounts[Player.TeamID]++;
        }
    }
    
    // 최대 인원 차이 계산
    int32 MinCount = INT_MAX;
    int32 MaxCount = 0;
    
    for (int32 Count : TeamCounts)
    {
        if (Count > 0) // 비어있지 않은 팀만 고려
        {
            MinCount = FMath::Min(MinCount, Count);
            MaxCount = FMath::Max(MaxCount, Count);
        }
    }
    
    // 팀 간 인원 차이가 1명 이하여야 밸런스가 잡힌 것으로 간주
    return (MaxCount - MinCount) <= 1;
}
```

### 4.4 방 관리 시스템 개선
방 생성 및 참가 관련 기능을 개선했습니다.

#### 방장 권한 강화
방장에게만 특정 기능을 허용하도록 권한 체계를 강화했습니다:

```cpp
bool IsHost = (CurrentRoom.HostID == LocalPlayerID);
if (IsHost)
{
    // 방장 전용 기능 활성화
    StartGameButton->SetVisibility(ESlateVisibility::Visible);
    MapSelectionComboBox->SetIsEnabled(true);
}
else
{
    // 일반 플레이어는 방장 기능 비활성화
    StartGameButton->SetVisibility(ESlateVisibility::Hidden);
    MapSelectionComboBox->SetIsEnabled(false);
}
```

#### 맵 선택 시스템
방장이 게임 맵을 선택할 수 있는 기능을 추가했습니다:

1. ThirdPersonExampleMap을 기본 맵으로 설정
2. 추후 더 많은 맵을 추가할 수 있는 구조 구현

```cpp
// 맵 목록 초기화
AvailableMaps.Add(FMapInfo{ "ThirdPersonExampleMap", "기본 맵", nullptr });
// 추후 더 많은 맵 추가 예정
```

#### 카운트다운 시스템
게임 시작 시 카운트다운 기능을 구현했습니다:

```cpp
void UGameStartCountdown::StartCountdown(int32 Seconds)
{
    RemainingSeconds = Seconds;
    GetWorld()->GetTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &UGameStartCountdown::UpdateCountdown,
        1.0f,
        true
    );
    
    // 카운트다운 UI 활성화
    CountdownText->SetVisibility(ESlateVisibility::Visible);
    UpdateCountdownDisplay();
}

void UGameStartCountdown::UpdateCountdown()
{
    RemainingSeconds--;
    
    if (RemainingSeconds <= 0)
    {
        // 카운트다운 완료, 게임 시작
        GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
        OnCountdownFinished.Broadcast();
    }
    else
    {
        // 카운트다운 UI 업데이트
        UpdateCountdownDisplay();
    }
}
```

## 5. 데디케이티드 서버 도입

### 5.1 데디케이티드 서버로 전환
맵 전환 이슈 해결을 위해 데디케이티드 서버 방식으로 전환했습니다. 이 과정에서 여러 변경이 필요했습니다:

1. 서버 실행 스크립트 생성
   ```batch
   @echo off
   start UE4Editor.exe BridgeRun ThirdPersonExampleMap -server -log
   ```

2. 클라이언트 접속 로직 수정
   ```cpp
   // 서버 IP 주소 설정
   ServerIPAddress = "127.0.0.1";
   
   // 서버에 직접 접속
   APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
   if (PC)
   {
       PC->ClientTravel(ServerIPAddress, TRAVEL_Absolute);
   }
   ```

3. 세션 관리 로직 조정
   데디케이티드 서버에 맞게 세션 관리 로직을 조정했습니다. 특히 권한 체크를 강화했습니다:
   ```cpp
   if (GetWorld()->GetNetMode() == NM_DedicatedServer)
   {
       // 서버 전용 로직
   }
   else
   {
       // 클라이언트 전용 로직
   }
   ```

### 5.2 권한 관리 개선
데디케이티드 서버 환경에서는 권한 관리가 더욱 중요해졌습니다. 이를 위해 다음과 같은 개선을 진행했습니다:

1. RPC 함수에 권한 검사 추가
   ```cpp
   UFUNCTION(Server, Reliable, WithValidation)
   void ServerStartGame();
   
   bool ServerStartGame_Validate()
   {
       // 방장만 게임 시작 가능
       return IsHost();
   }
   
   void ServerStartGame_Implementation()
   {
       // 게임 시작 로직
   }
   ```

2. 권한별 UI 제어 강화
   클라이언트 UI에서는 권한에 따라 버튼 활성화/비활성화를 명확하게 구분했습니다.

## 6. 테스트 및 최적화

### 6.1 멀티플레이어 테스트
통합된 로비 시스템을 다양한 시나리오에서 테스트했습니다:

1. 다중 클라이언트 접속 테스트
2. 방 생성/참가/나가기 테스트
3. 팀 선택 및 변경 테스트
4. 게임 시작 및 맵 전환 테스트

이를 통해 몇 가지 추가적인 문제점을 발견하고 해결했습니다:

- 방 나가기 후 재접속 시 UI 동기화 문제 수정
- 팀 변경 시 네트워크 복제 지연 이슈 수정
- 카운트다운 중 방장 퇴장 시 처리 로직 개선

### 6.2 성능 최적화
로비 시스템의 성능을 최적화했습니다:

1. 불필요한 네트워크 트래픽 감소
   ```cpp
   // 변경된 속성만 복제하도록 최적화
   DOREPLIFETIME_CONDITION(ABP_Citizen, TeamID, COND_SimulatedOnly);
   ```

2. UI 업데이트 최적화
   필요한 경우에만 UI를 업데이트하도록 로직을 개선했습니다.

3. 방 목록 갱신 빈도 조정
   방 목록 갱신 빈도를 조정하여 네트워크 부하를 줄였습니다.

## 7. 최종 통합 및 향후 계획

### 7.1 통합 완료된 기능
스프린트 11 종료 시점에 다음 기능들이 성공적으로 통합되었습니다:

- 데디케이티드 서버 기반 로비 시스템
- BP_Citizen 캐릭터 통합
- 팀 선택 및 색상 시스템
- 방 생성 및 참가 기능
- 게임 시작 카운트다운
- 기본 채팅 시스템

### 7.2 향후 개선 계획
다음 스프린트에서는 다음 기능들을 추가로 개발할 예정입니다:

1. **고급 팀 시스템**
   - 팀별 스탯과 특성 추가
   - 팀 밸런스 알고리즘 개선

2. **맵 선택 시스템 확장**
   - 다양한 맵 추가
   - 맵 프리뷰 기능 구현

3. **로비 UI 개선**
   - 브릿지런 테마에 맞는 UI 디자인 적용
   - 캐릭터 커스터마이징 UI 개선

4. **네트워크 안정성 강화**
   - 접속 끊김 감지 및 복구 기능
   - 세션 관리 최적화

## 8. 회고 및 느낀점

이번 스프린트에서 SimpleLobbySystem을 브릿지런에 통합하면서 많은 도전과 배움이 있었습니다. 처음에는 단순히 플러그인을 추가하고 몇 가지 설정만 변경하면 될 것이라 생각했지만, 실제로는 훨씬 복잡한 과정이었습니다.

특히 맵 전환 이슈는 예상치 못한 도전이었습니다. 이 문제를 해결하기 위해 언리얼 엔진의 세션 관리와 네트워크 아키텍처를 더 깊이 이해할 수 있었고, 데디케이티드 서버 방식으로 전환하면서 더 안정적인 멀티플레이어 환경을 구축할 수 있었습니다.

또한 BP_Citizen을 기존 로비 시스템에 통합하는 과정은 언리얼 엔진의 클래스 상속과 네트워크 복제에 대한 이해를 높이는 좋은 기회였습니다. 앞으로 이러한 경험을 바탕으로 더 확장성 있고 유지보수가 쉬운 코드를 작성할 수 있을 것입니다.

SimpleLobbySystem을 구매하여 사용한 것은 결과적으로 올바른 결정이었습니다. 처음부터 로비 시스템을 개발했다면 훨씬 더 많은 시간이 소요되었을 것입니다. 물론 플러그인을 브릿지런에 맞게 수정하는 과정에서 예상보다 많은 작업이 필요했지만, 그래도 기본적인 프레임워크를 활용할 수 있어 개발 속도를 크게 높일 수 있었습니다.

다음 스프린트에서는 이번에 통합한 로비 시스템을 기반으로 더 다양하고 풍부한 기능을 추가할 예정입니다. 특히 팀 시스템과 맵 선택 기능을 확장하여 브릿지런의 게임성을 더욱 향상시킬 것입니다.
