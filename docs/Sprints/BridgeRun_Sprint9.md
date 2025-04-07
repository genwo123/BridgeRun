# 브릿지런 개발일지 (스프린트 9)

## 📅 개발 기간
2025년 3월 17일 ~ 2025년 3월 30일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표
스프린트 9에서는 코드 품질 개선과 유지보수성 향상을 위한 SOLID 및 OOP 원칙 기반 리팩토링에 집중했습니다:

* SOLID 원칙(단일 책임, 개방-폐쇄, 리스코프 치환, 인터페이스 분리, 의존성 역전)에 따른 구조 개선
* 모듈화 및 코드 재사용성 향상
* 명확한 책임 분리와 함수 분리로 디버깅 용이성 증대
* 확장성 있는 구조로 변경하여 추후 개발 효율성 향상

## 2. 리팩토링 배경 및 필요성

### 2.1 외부 피드백
최근 현업 개발자와의 대화 중에 받은 피드백을 통해 코드 구조의 개선 필요성을 인식했습니다:

>"코드가 깔끔하지 않아서 아쉽다. C++의 장점을 더 살릴 수 있을 것 같다."

이러한 피드백을 바탕으로 코드를 검토한 결과, 다음과 같은 문제점들을 발견했습니다:

* 한 함수가 여러 책임을 담당하여 가독성과 유지보수성 저하
* 모듈화가 부족하여 작은 변경도 연쇄적인 수정 필요
* 코드의 중복이 많고 재사용성이 낮음
* 일관된 명명 규칙과 구조의 부재

### 2.2 기존 코드의 문제점
특히 `BuildingComponent` 클래스에서 다음과 같은 구체적인 문제를 발견했습니다:

**함수의 과도한 책임**:
```cpp
// 건설 시도, 아이템 사용, 물리 설정 등 여러 책임이 한 함수에 혼재
void UBuildingComponent::AttemptBuild_Implementation()
{
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
        return;

    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 2.0f, false);

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AItem_Plank* SpawnedPlank = GetWorld()->SpawnActor<AItem_Plank>(
                PlankClass,
                Location,
                Rotation,
                SpawnParams
            );

            // ... 이하 길고 중복된 코드 생략 ...
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        // ... 위와 매우 유사하지만 중복된 코드 ...
    }

    // ... 이하 생략 ...
}
```

**긴 함수와 코드 중복**:
* 판자와 텐트 생성 로직이 거의 동일하지만 중복 구현
* 건설 후 인벤토리 처리 로직이 여러 위치에 분산

**불명확한 상태 관리**:
* 상태 변수들(`bCanBuildNow`, `bIsBuilding` 등)의 조작이 여러 함수에 분산
* 상태 변경 시 관련 효과(시각적, 물리적)가 일관되게 적용되지 않음

## 3. 리팩토링 접근 방식

### 3.1 SOLID 원칙 적용

#### 단일 책임 원칙 (Single Responsibility Principle)
각 함수가 하나의 책임만 가지도록 기존 함수를 분리했습니다:

```cpp
// 리팩토링 후: 명확한 책임 분리
void UBuildingComponent::AttemptBuild_Implementation()
{
    if (!ValidateBuildAttempt())
        return;

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // 아이템 타입에 따라 다른 처리 (템플릿 함수로 중복 제거)
    if (CurrentBuildingItem == EInventorySlot::Plank && OwnerCitizen->UseItem(EInventorySlot::Plank))
    {
        SpawnBuildingItem<AItem_Plank>(PlankClass, Location, Rotation);
    }
    else if (CurrentBuildingItem == EInventorySlot::Tent && OwnerCitizen->UseItem(EInventorySlot::Tent))
    {
        SpawnBuildingItem<AItem_Tent>(TentClass, Location, Rotation);
    }

    UpdateBuildState();
}

// 건설 시도 유효성 검사를 별도 함수로 분리
bool UBuildingComponent::ValidateBuildAttempt() const
{
    return BuildPreviewMesh && OwnerCitizen && bCanBuildNow && 
           bIsValidPlacement && !bIsBuilding && GetOwner()->HasAuthority();
}

// 건설 후 상태 업데이트를 별도 함수로 분리
void UBuildingComponent::UpdateBuildState()
{
    bIsBuilding = false;
    bCanBuildNow = true;
    MulticastOnBuildComplete();

    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }

    if (BuildPreviewMesh)
    {
        BuildPreviewMesh->MarkRenderStateDirty();
    }
}
```

#### 템플릿 메서드 패턴 적용
반복되는 코드를 템플릿 함수로 통합하여 코드 중복을 제거했습니다:

```cpp
// 템플릿 함수로 아이템 생성 로직 통합
template<class T>
T* UBuildingComponent::SpawnBuildingItem(TSubclassOf<T> ItemClass, const FVector& Location, const FRotator& Rotation)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCitizen;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    T* SpawnedItem = GetWorld()->SpawnActor<T>(
        ItemClass,
        Location,
        Rotation,
        SpawnParams
    );

    if (SpawnedItem)
    {
        // 공통 설정
        SpawnedItem->SetReplicates(true);
        SpawnedItem->SetReplicateMovement(true);
        SpawnedItem->bIsBuiltItem = true;

        if (SpawnedItem->MeshComponent)
        {
            // 물리 및 충돌 설정 별도 함수로 분리
            ConfigureBuildingItemPhysics(SpawnedItem->MeshComponent, Location, Rotation);
        }

        // 인벤토리 상태 체크 별도 함수로 분리
        CheckInventoryAfterBuilding(SpawnedItem);

        return SpawnedItem;
    }

    return nullptr;
}
```

#### 개방-폐쇄 원칙 (Open-Closed Principle)
코드를 확장에 열려있고 수정에 닫혀있도록 구조화했습니다:

```cpp
// 물리/충돌 설정을 별도 함수로 분리하여 수정 없이 확장 가능하게 함
void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, const FVector& Location, const FRotator& Rotation)
{
    if (!MeshComp) return;

    // 기본 물리 설정
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetEnableGravity(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // 네트워크 복제 설정
    MeshComp->SetIsReplicated(true);
    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->bReplicatePhysicsToAutonomousProxy = true;

    // 위치 고정
    MeshComp->SetWorldLocation(Location);
    MeshComp->SetWorldRotation(Rotation);
    MeshComp->UpdateComponentToWorld();
}
```

### 3.2 함수 분할 및 구조화
큰 함수들을 작고 명확한 목적을 가진 함수들로 분할했습니다:

**리팩토링 전**:
```cpp
void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // 100줄 이상의 복잡한 로직...
}
```

**리팩토링 후**:
```cpp
void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    AController* Controller = OwnerCitizen->GetController();
    if (!Controller) return;

    // 카메라 및 플레이어 위치 계산
    FVector CameraLocation, PlayerLocation;
    FRotator CameraRotation;
    CalculateViewPositions(Controller, CameraLocation, CameraRotation, PlayerLocation);

    // 프리뷰 위치 및 회전 결정
    FVector PreviewLocation;
    FRotator PreviewRotation;
    CalculatePreviewTransform(CameraLocation, CameraRotation, PlayerLocation, PreviewLocation, PreviewRotation);

    // 배치 유효성 검사
    bool NewValidPlacement = DetermineValidPlacement(PreviewLocation, PreviewRotation);

    // 시각적 업데이트
    UpdatePreviewVisuals(NewValidPlacement);
}
```

### 3.3 명명 규칙 및 가독성 개선
* 함수와 변수의 이름을 더 명확하고 일관되게 수정
* 주석 추가 및 코드 구조화로 가독성 향상
* 단위 작업별로 논리적 그룹화

## 4. 주요 리팩토링 영역

### 4.1 BuildingComponent 클래스
가장 광범위한 리팩토링이 이루어진 영역입니다:

1. **함수 세분화**: 거대한 함수들을 의미 있는 작은 함수들로 분리
2. **템플릿 패턴 적용**: 중복 코드를 제거하기 위한 템플릿 함수 도입
3. **책임 분리**: 물리 설정, 시각화, 상태 관리 등 영역별 분리 
4. **상태 관리 개선**: 상태 변경과 시각화 연동을 명확히 분리

리팩토링 전후 코드:

```cpp
// 리팩토링 전: 모든 기능이 한 함수에 혼합됨
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 200줄 이상의 복잡한, 중첩된 로직
}

// 리팩토링 후: 명확히 세분화된 책임
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 30줄 이하의 명확한 고수준 로직
}

// 분리된 헬퍼 함수들
bool UBuildingComponent::ValidateBuildAttempt() const { /* ... */ }
void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, /* ... */) { /* ... */ }
void UBuildingComponent::CheckInventoryAfterBuilding(AItem* BuiltItem) { /* ... */ }
void UBuildingComponent::UpdateBuildState() { /* ... */ }
```

### 4.2 네트워크 복제 및 상태 관리
네트워크 상태 관리와 복제 관련 로직을 개선했습니다:

```cpp
// 리팩토링 전: 상태 변경 후 네트워크 업데이트가 일관되지 않음
void UBuildingComponent::OnRep_BuildState()
{
    // 상태에 따른 시각적 처리가 복잡하게 얽혀있음
}

// 리팩토링 후: 상태별 명확한 처리
void UBuildingComponent::OnRep_BuildState()
{
    UpdateOwnerBuildState();
    UpdatePreviewVisuals(bIsValidPlacement);
}

void UBuildingComponent::UpdateOwnerBuildState()
{
    // 캐릭터 상태 업데이트 로직 
}

void UBuildingComponent::UpdatePreviewVisuals(bool bValid)
{
    // 시각적 피드백 업데이트 로직
}
```

### 4.3 AI 조력자(클로드)를 활용한 체계적 리팩토링
리팩토링 과정에서 AI 조력자를 활용하여 객체지향 원칙에 맞는 코드 구조화를 진행했습니다:

1. 기존 코드 분석 및 SOLID 원칙 위반 식별
2. 함수 분리와 책임 할당에 대한 체계적 접근
3. 템플릿 및 디자인 패턴 적용 지점 식별
4. 점진적 개선과 단위별 테스트

이러한 방식을 통해 코드 길이는 크게 증가하지 않으면서도 가독성과 유지보수성이 크게 향상되었습니다.

## 5. 리팩토링 결과 및 성과

### 5.1 코드 품질 향상
* 중복 코드 제거로 버그 발생 가능성 감소
* 책임 분리로 각 함수의 가독성 및 테스트 용이성 증가
* 명확한 함수명과 구조로 코드 이해도 향상

### 5.2 유지보수성 개선
* 작은 함수 단위로 나뉘어 변경 영향 범위 최소화
* 함수 재사용성 증가로 추가 개발 시간 단축
* 확장에 용이한 구조로 새로운 기능 추가가 간편해짐

### 5.3 성능 최적화
* 필요한 시점에만 상태 업데이트 수행으로 불필요한 연산 감소
* 네트워크 동기화 최적화로 대역폭 사용 감소
* 성능 병목 지점 식별 및 해결

### 5.4 확장성 향상
* 새로운 건설 아이템 추가가 용이해짐
* 게임 메커니즘 변경에 따른 코드 수정 범위 최소화
* 팀 개발 시 역할 분담이 명확해짐

## 6. 다음 스프린트 계획

### 6.1 커스텀 UI 위젯 개발
* 브릿지런 전용 UI 플러그인 개발
* 팀 기반 UI 템플릿 제작
* 객체지향 방식의 UI 클래스 설계

### 6.2 UI 컴포넌트 아키텍처 구축
* 재사용 가능한 UI 컴포넌트 라이브러리 개발
* 플러그인 형태의 모듈화된 구조 설계
* SOLID 원칙에 따른 UI 시스템 구현

### 6.3 플레이어 피드백 시스템 개선
* 직관적인 건설 관련 UI/UX 개선
* 팀 시스템과 통합된 HUD 개발
* 게임 상태 피드백 메커니즘 개선

## 7. 회고 및 느낀점
이번 스프린트에서 SOLID 원칙과 객체지향 설계에 기반한 코드 리팩토링을 진행하면서 많은 것을 배웠습니다. 처음에는 단순히 '작동하는 코드'를 만드는 데 중점을 두었지만, 현업 개발자의 피드백을 통해 '좋은 코드'가 무엇인지를 고민하게 되었습니다.

특히 C++의 강력한 템플릿 기능과 객체지향 패턴을 제대로 활용하지 못했던 점을 깨닫고, 이번 기회에 체계적으로 개선했습니다. 재사용성과 확장성을 고려한 설계가 장기적으로 얼마나 큰 이점을 가져오는지 직접 경험할 수 있었습니다.

AI 조력자(클로드)를 활용하여 코드 분석 및 리팩토링을 진행한 방식도 매우 효율적이었습니다. 각 함수가 하나의 책임만 담당하게 하고, 코드 중복을 제거하는 과정에서 버그를 발견하고 수정할 수 있었습니다.

코드 라인 수는 크게 증가하지 않았지만, 전체 코드베이스의 가독성과 유지보수성이 눈에 띄게 향상되었습니다. 특히 BuildingComponent와 같은 복잡한 클래스의 경우, 새로운 기능을 추가하거나 기존 기능을 수정하는 데 필요한 시간이 크게 줄어들었습니다.

이번 리팩토링을 통해 깨끗하고 확장 가능한 코드를 작성하는 것이 단순히 '좋은 관행'이 아니라, 개발 효율성과 품질에 직접적인 영향을 미치는 중요한 요소임을 체감했습니다. 앞으로의 개발에서도 이러한 원칙과 패턴을 꾸준히 적용해 나가겠습니다.
