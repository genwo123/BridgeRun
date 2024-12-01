# 브릿지런 (BridgeRun) 개발 진행 상황
## 1-2주차 (2024.11.04 ~ 2024.11.17)

### 🎯 주요 개발 목표
- 기본 캐릭터 시스템 구축
- 인벤토리 및 UI 시스템 개발
- 모듈화된 프로젝트 구조 설계

### 💻 주요 구현 내용

#### 1. UI 스택 시스템 구현
![PlayerStack 구조](./images/sprint1/ui_player_stack_structure.png)
*Common UI를 활용한 UI 스택 관리 시스템*

![HUD 연동](./images/sprint1/ui_hud_gamemode_setup.png)
*게임모드와 HUD 시스템 연동*

#### 2. 인벤토리 시스템
![아이템 슬롯 구조](./images/sprint1/ui_inventory_slot_layout.png)
*기본 아이템 슬롯 UI 구조*

![실시간 카운트](./images/sprint1/ui_item_count_binding.png)
*아이템 카운트 실시간 업데이트 시스템*

![아이템 카운트 디자인](./images/sprint1/ui_item_count_buleprint.png)
*아이템 카운트 UI 디자인 및 바인딩*

#### 3. 입력 시스템
![입력 매핑](./images/sprint1/input_system_mapping.png)
*기본 입력 시스템 구성*

#### 4. 게임플레이 구현
![인게임 구현](./images/sprint1/ingame_inventory_ui.png)
*실제 게임에서 구현된 UI 시스템*

### 🛠 기술 스택
- 언리얼 엔진 4
- C++ (게임플레이 프레임워크)
- Common UI 플러그인

### 🎮 구현된 기능
- 기본 캐릭터 이동 및 점프
- 아이템 획득 및 관리
- 실시간 UI 업데이트
- 모드 전환 시스템 (일반/건설/전투)

### 📝 다음 개발 계획
- 직업 시스템 확장
- 건설/전투 시스템 상세 구현
- UI 커스터마이징 기능 추가

### 📖 상세 정보
더 자세한 개발 내용은 프로젝트의 개발일지 PDF를 참고해 주세요.

### 👨‍💻 개발 환경
- IDE: Visual Studio 2022
- 엔진: Unreal Engine 4.27
- 플러그인: Common UI
