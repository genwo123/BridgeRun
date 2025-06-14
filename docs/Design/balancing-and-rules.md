# 브릿지런 밸런스 전략

> **"협력과 경쟁의 균형점을 찾아서"**

---

## 📊 핵심 밸런스 철학

### 🎯 밸런스 목표
- **직업간 상성**: 각 직업의 고유 역할과 카운터 관계
- **아이템 희소성**: 스폰 주기와 최대 보유량의 균형
- **위험 vs 보상**: 중립 토템존의 위험도와 점수 배율
- **시간 관리**: 라운드 시간과 각종 쿨다운의 조화

---

## ⚖️ 직업 간 밸런스

### 시즌 0 직업 밸런스표

| 직업명 | 특화도 | 범용성 | 생존력 | 팀 기여도 | 밸런스 점수 |
|--------|--------|--------|--------|-----------|-------------|
| **시민** | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | **13/20** |
| **건설가** | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | **18/20** |
| **직물사** | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | **16/20** |
| **저격수** | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ | ⭐⭐⭐ | **14/20** |
| **러너** | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ | **15/20** |

### 🔄 직업별 카운터 관계

```
건설가 → 직물사 → 저격수 → 러너 → 시민 → 건설가
   ↑                                            ↓
   └────────────── 밸런스 사이클 ──────────────────┘
```

#### 상성 관계 상세

| 직업 A | vs | 직업 B | 상성 | 이유 |
|--------|----|---------|----|------|
| **건설가** | ⚔️ | **저격수** | 🟢 유리 | 다리 건설로 접근 차단 |
| **직물사** | ⚔️ | **러너** | 🟢 유리 | 텐트로 기동성 제한 |
| **저격수** | ⚔️ | **시민** | 🟢 유리 | 원거리 우위 |
| **러너** | ⚔️ | **건설가** | 🟢 유리 | 빠른 기동으로 건설 방해 |
| **시민** | ⚔️ | **직물사** | 🟢 유리 | 범용성으로 대응 |

---

## 🎒 아이템 밸런스 시스템

### 아이템 희소성 설계

| 아이템 | 초기 수량 | 스폰 주기 | 최대 보유 | 낙사 보존율 | 전략적 가치 |
|--------|-----------|-----------|-----------|-------------|-------------|
| **판자** | 10개/팀 | 20초/개 | 15개/팀 | 50% (건설가 100%) | ⭐⭐⭐⭐⭐ |
| **텐트** | 1개/팀 | 120초/개 | 2개/팀 | 50% (직물사 100%) | ⭐⭐⭐⭐ |
| **총기** | 맵 배치 | 상황별 | 1개/인 | 팀 베이스 귀환 | ⭐⭐⭐ |
| **망원경** | 기본 지급 | - | 1개/인 | 100% | ⭐⭐ |

### 📦 보급 상자 밸런스

| 보급 타입 | 내용물 | 스폰 위치 | 쟁탈 난이도 | 전략적 의미 |
|-----------|--------|-----------|-------------|-------------|
| **판자 상자** | 판자 x10 | 중간 지점 | 🟡 보통 | 건설 촉진 |
| **텐트 상자** | 텐트 x2 | 위험 지역 | 🔴 어려움 | 방어 강화 |
| **무기 상자** | 총 x1 | 전략 거점 | 🟠 높음 | 전투력 증대 |

---

## 🏆 점수 시스템 밸런스

### 토템 가치 설계

| 토템 종류 | 기본 가치 | 등장 라운드 | 생성 주기 | 위험도 | 점수 효율성 |
|-----------|-----------|-------------|-----------|--------|-------------|
| **일반 토템** | 10점 | 모든 라운드 | 45초마다 1-2개 | 🟢 낮음 | ⭐⭐⭐ |
| **골드 토템** | 20점 | 2라운드부터 | 2분마다 1개 | 🟡 보통 | ⭐⭐⭐⭐ |
| **다이아몬드 토템** | 30점 | 3라운드만 | 라운드당 1개 | 🔴 높음 | ⭐⭐⭐⭐⭐ |

### 🎯 토템존 배율 시스템

| 토템존 유형 | 위치 | 점수 배율 | 접근 난이도 | 전략적 가치 |
|-------------|------|-----------|-------------|-------------|
| **팀 토템존** | 본진 근처 | 1.0배 | 🟢 쉬움 | 안전한 기본 점수 |
| **중립 토템존** | 맵 중앙 | 1.5배 | 🟡 보통 | 중위험 중보상 |
| **위험 토템존** | 고립 지역 | 2.0배 | 🔴 어려움 | 고위험 고보상 |

---

## ⏱️ 시간 밸런스 설계

### 라운드별 시간 압박

| 라운드 | 제한 시간 | 목표 점수 | 시간당 난이도 | 특별 규칙 |
|--------|-----------|-----------|---------------|-----------|
| **1라운드** | 4분 | 40점 | 🟢 여유 | 시민 고정, 기본 학습 |
| **2라운드** | 4분 | 45점 | 🟡 보통 | 직업 선택, 골드 토템 등장 |
| **3라운드** | 4분 | 50점 | 🔴 긴박 | 다이아몬드 토템, 골든타임 |

### ⚡ 쿨다운 밸런스표

| 시스템 | 쿨다운 | 밸런스 목적 | 조정 가능성 |
|--------|--------|-------------|-------------|
| **판자 생성** | 20초 | 무분별한 건설 방지 | 2라운드 40초로 증가 |
| **텐트 생성** | 120초 | 방어 수단 희소성 | 고정 |
| **직업 교체** | 라운드 시작시만 | 전략적 선택 강화 | 고정 |
| **리스폰** | 5초 | 게임 템포 유지 | 고정 |

---

## 🎮 플레이어 행동 유도 설계

### 💰 위험-보상 곡선

```
높은 보상 ↑
          │     ◆ 다이아몬드 토템 (위험지역)
          │   ◇ 골드 토템 (중앙)
          │ ○ 일반 토템 (안전지역)
          └─────────────────────→ 높은 위험
```

### 🚀 게임 템포 조절

| 게임 구간 | 템포 설계 | 플레이어 행동 | 밸런스 요소 |
|-----------|-----------|---------------|-------------|
| **초반 (0-1분)** | 🐌 느린 탐색 | 맵 파악, 역할 분담 | 충분한 자원 제공 |
| **중반 (1-3분)** | 🏃 활발한 경쟁 | 토템 쟁탈, 건설 | 자원 희소성 증가 |
| **후반 (3-4분)** | ⚡ 격렬한 대결 | 최종 스퍼트 | 골든타임 2배 점수 |

---

## 📈 지속 모니터링 지표

### 🔍 핵심 밸런스 KPI

| 지표 | 목표값 | 현재값 | 상태 | 조치 필요성 |
|------|--------|--------|------|-------------|
| **직업별 승률 편차** | ±5% 이내 | 측정 중 | 🟡 관찰 | 데이터 수집 중 |
| **라운드별 평균 점수** | 40-45-50점 | 측정 중 | 🟡 관찰 | 플레이테스트 필요 |
| **토템존 점유율** | 각 25-30% | 측정 중 | 🟡 관찰 | 위치 조정 검토 |
| **게임 길이** | 12-15분 | 측정 중 | 🟡 관찰 | 템포 조정 가능 |

### 🎯 밸런스 조정 우선순위

| 순위 | 조정 대상 | 이유 | 예상 영향도 |
|------|-----------|-----|-------------|
| **1순위** | 직업별 특화 능력 | 게임 재미의 핵심 | 🔴 매우 높음 |
| **2순위** | 토템 생성 주기 | 게임 템포 결정 | 🟠 높음 |
| **3순위** | 아이템 스폰 주기 | 전략 다양성 | 🟡 보통 |
| **4순위** | 맵 레이아웃 | 플레이 패턴 | 🟡 보통 |

---

## 🔄 시즌 1 대적자 밸런스 (차후 개발)

### 건설 vs 파괴 밸런스

| 건설 직업 | 파괴 직업 | 밸런스 포인트 | 설계 철학 |
|-----------|-----------|---------------|-----------|
| **건설가** | **철거반** | 건설 속도 vs 파괴 속도 | 창조와 파괴의 균형 |
| **직물사** | **방화범** | 방어력 vs 화력 | 보호와 공격의 대립 |
| **러너** | **트래퍼** | 기동성 vs 구속력 | 자유와 제약의 경쟁 |
| **저격수** | **연막병** | 정밀성 vs 혼란 | 질서와 무질서의 대결 |

### ⚔️ 대적자 시스템 특징

- **양손무기**: 2칸 차지, 높은 파괴력
- **토템 양립성**: 파괴 전문이지만 토템 운반 가능
- **카운터 플레이**: 기존 직업에 대한 명확한 카운터
- **밸런스 복잡성**: 9개 직업 간 상성 관계 관리

---

## 🏁 밸런스 검증 체크리스트

### ✅ 출시 전 필수 검증

- [ ] **직업별 승률**: 모든 직업 45-55% 승률 달성
- [ ] **게임 길이**: 95% 게임이 12-18분 내 종료
- [ ] **점수 분포**: 팀 간 점수 격차 30점 이내
- [ ] **아이템 활용도**: 모든 아이템 80% 이상 사용률
- [ ] **토템존 활용**: 모든 토템존 70% 이상 활용률

### 🔄 지속 업데이트 계획

- **월간 데이터 리뷰**: 밸런스 지표 분석
- **분기별 대형 패치**: 주요 밸런스 조정
- **시즌별 리워크**: 직업/시스템 전면 개편
- **커뮤니티 피드백**: 플레이어 의견 적극 반영

---

*이 문서는 플레이테스트 결과에 따라 지속적으로 업데이트됩니다.*