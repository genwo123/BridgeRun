# 브릿지런
## 🎮 게임 개요
**장르**: 팀 기반 전략 액션 게임
**대상 사용자**:
- 15세 이상, 전략적 팀 플레이를 즐기는 게이머
- 빠른 액션과 협력 플레이에 관심이 있는 사용자
  
**주요 경험**:
- 치열한 팀 대결과 전략적 사고를 요구하는 경험
- 다양한 직업을 활용한 다채로운 전술
- 도시의 영웅이 되어 명예와 부를 쟁취하는 몰입감
  
**플랫폼**: PC, 콘솔
## 💫 게임의 차별점
- 미래 도시의 다리를 배경으로 한 독특한 전장
- 직업 간의 시너지와 전략적 직업 변경 시스템
- 다리 및 천막 설치 등 다양한 건설 및 방어 요소를 통한 창의적 전투
- 트로피 타워 점령을 통한 명예와 자원의 획득



## 💻 시스템 구현 현황
| 시스템 | 설명 | 진행 상황 | 중요도 | 구현 계획 | 관련 스프린트 |
|---|---|---|---|---|---|
| 캐릭터 시스템 | 컴포넌트 기반의 확장 가능한 캐릭터 구조 | ✅ 완료 | 🔥 높음 | 직업 시스템으로 확장 예정 | Sprint 1 |
| 인벤토리 시스템 | 아이템 관리 및 실시간 UI 업데이트 | 🔄 진행중 | 🔥 높음 | Slate UI로 마이그레이션 예정 | Sprint 1, 3 (UI 개선) |
| 건설 시스템 | 스플라인 기반 다리 건설 메커닉 | ✅ 완료 | 🔥 높음 | 추가 건설 패턴 구현 예정 | Sprint 2 |
| 점수 시스템 | 트로피 기반 점수 획득 로직 | ✅ 완료 | 🔥 높음 | 팀별 점수 시스템 확장 예정 | Sprint 2 |
| 전투 시스템 | 기본 전투 메커닉 | ✅ 완료 | 🔥 높음 | 무기 데미지 시스템 구현 예정 | Sprint 2-3 |
| 팀 시스템 | 팀 기반 게임플레이 로직 | ⏳ 계획됨 | 🔥 높음 | Sprint 4에서 구현 예정 | Sprint 6 |
| 네트워크 시스템 | 멀티플레이어 지원 | ⏳ 계획됨 | 🔥 높음 | Sprint 4 이후 구현 | 미정 |




## 📝 개발 일지
| 회차 | 기간 | 핵심 내용 | 개발 일지 |
| --- | --- | --- | --- |
| 디자인문서 | 2024.10.21 - 2025.ing   | 게임 디자인 문서 | [디자인 문서](./docs/Sprints/Designdocument.md) |
| Sprint 00 | 2024.10.21 - 2024.11.03 | 게임 기획 및 초기 컨셉 설계 | [0회차 개발일지](./docs/Sprints/BridgeRun_Sprint0.md) |
| Sprint 01 | 2024.11.04 - 2024.11.17 | 기본 시스템 구축 및 컴포넌트 설계 | [1회차 개발일지](./docs/Sprints/BridgeRun_Sprint1.md) |
| Sprint 02 | 2024.11.18 - 2024.12.01 | 점수/건설 시스템 구현 및 최적화 | [2회차 개발일지](./docs/Sprints/BridgeRun_Sprint2.md) |
| Sprint 03 | 2024.12.02 - 2024.12.15 | 전투 시스템 구현 및 UI 개선 계획 | [3회차 개발일지](./docs/Sprints/BridgeRun_Sprint3.md) |
| Sprint -- | 2024.12.16 - 2024.12.29 | 건강 및 이사| 몸 건강 상태 및 이사 이슈로 인해 2주간 작업이 불가피했습니다. |
| Sprint 04 | 2024.12.30 - 2025.01.12 | UI 시스템 개선 및 네트워크 기반 구축 | [4회차 개발일지](./docs/Sprints/BridgeRun_Sprint4.md) |
| Sprint 05 | 2025.01.13 - 2025.02.02 | 물리/충돌 시스템 네트워크 동기화 및 버그 수정| [5회차 개발일지](./docs/Sprints/BridgeRun_Sprint5.md) |
| Sprint 06 | 2025.02.03 - 2025.02.16 | 낙사 시스템 및 리스폰 시스템 구현| [6회차 개발일지](./docs/Sprints/BridgeRun_Sprint6.md) |
| Sprint 07 | 2025.02.17 - 2025.03.02 | Score 시스템 구현 및 위젯 구성| [7회차 개발일지](./docs/Sprints/BridgeRun_Sprint7.md) |
| Sprint 08 | 2025.03.03 - 2025.03.16 | 팀 시스템 기반 구축 및 랜덤 배정 로직 | [8회차 개발일지](./docs/Sprints/BridgeRun_Sprint8.md) |
| Sprint 09 | 2025.03.17 - 2025.03.30 | SOLID 및 OOP 원칙 기반 코드 리팩토링 | [9회차 개발일지](./docs/Sprints/BridgeRun_Sprint9.md) |
| Sprint 10 | 2025.03.31 - 2025.04.06 | 커스텀 BR UI 플러그인 개발 | [10회차 개발일지](./docs/Sprints/BridgeRun_Sprint10.md) |
| Sprint 11 | 2025.04.07 - 2025.04.20 | 로비 시스템 구축 및 멀티플레이어 환경 변경 | [11회차 개발일지](./docs/Sprints/BridgeRun_Sprint11.md) |
| Sprint 12 | 2025.04.21 - 2025.05.04 | 로비 시스템 개선 및 팀 선택 기능 | [12회차 개발일지](./docs/Sprints/BridgeRun_Sprint12.md) |
| Sprint 13 | 2025.05.05 - 2025.05.18 |  | [13회차 개발일지](./docs/Sprints/BridgeRun_Sprint13.md) |
| Sprint 13 | 2025.05.19 - 2025.05.26 |  | [14회차 개발일지](./docs/Sprints/BridgeRun_Sprint14.md) |


### 개발 일지 내용
각 스프린트 기록에는 다음 내용이 포함됩니다:
- 📅 개발 기간
- 🎯 주요 목표 및 달성 사항
- 💻 구현된 시스템 및 기능
- 🔧 발생한 문제점과 해결 과정
- 📋 다음 스프린트 계획


## 🗂 프로젝트 구조
```
docs/
├── Sprints/
│   ├── BridgeRun_Sprint0.md   # 게임 기획 및 초기 컨셉
│   ├── BridgeRun_Sprint1.md   # 기본 시스템 구축
│   ├── BridgeRun_Sprint2.md   # 핵심 게임플레이 구현
│   ├── BridgeRun_Sprint3.md   # 전투 시스템 구현
│   ├── BridgeRun_Sprint4.md   # UI/네트워크 시스템 개선
│   ├── BridgeRun_Sprint5.md   # 물리/충돌 시스템 네트워크 동기화 및 버그 수정
│   ├── BridgeRun_Sprint6.md   # 낙사 및 리스폰 시스템 구현
│   ├── BridgeRun_Sprint7.md   # Score 시스템 구현 및 위젯 구성
│   ├── BridgeRun_Sprint8.md   # 팀 시스템 기반 구축 및 랜덤 배정 로직
│   ├── BridgeRun_Sprint9.md   # SOLID 및 OOP 원칙 기반 코드 리팩토링
│   ├── BridgeRun_Sprint10.md  # 커스텀 BR UI 플러그인 개발
│   └── images/                # 스프린트별 이미지
│       ├── sprint0/
│       ├── sprint1/
│       ├── sprint2/
│       ├── sprint3/.....
└── README.md                  # 프로젝트 메인 문서
```


## 🔧 설치 및 사용법
-현재는 설치 및 사용법이 없습니다. 빠른 시일 내에 만들어보겠습니다
