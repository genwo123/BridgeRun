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
* 건설 시도, 아이템 사용, 물리 설정 등 여러 책임이 한 함수에 혼재
* 하나의 변경이 여러 기능에 영향을 미치는 구조로 버그 발생 위험 높음

**긴 함수와 코드 중복**:
* 판자와 텐트 생성 로직이 거의 동일하지만 중복 구현
* 건설 후 인벤토리 처리 로직이 여러 위치에 분산

**불명확한 상태 관리**:
* 상태 변수들(`bCanBuildNow`, `bIsBuilding` 등)의 조작이 여러 함수에 분산
* 상태 변경 시 관련 효과(시각적, 물리적)가 일관되게 적용되지 않음

### 2.3 AI 조력자를 활용한 체계적 접근
리팩토링을 위해 AI 조력자(클로드)를 활용하여 체계적인 접근 방식을 취했습니다:

1. 기존 코드베이스 분석 및 문제점 식별
2. SOLID 원칙 및 디자인 패턴에 기반한 리팩토링 방안 도출
3. 중복 코드 제거 및 템플릿 패턴 적용 지점 식별
4. 함수 분할 및 책임 할당 방식 결정

이후 AI의 제안을 바탕으로 직접 코드를 수정하고 테스트하며 검증하는 과정을 거쳤습니다. 이러한 접근 방식을 통해 코드의 품질을 체계적으로 향상시킬 수 있었습니다.

## 3. 리팩토링 접근 방식

### 3.1 SOLID 원칙 적용

#### 단일 책임 원칙 (Single Responsibility Principle)
각 함수가 하나의 책임만 가지도록 기존 함수를 분리했습니다:

**리팩토링 전**:
```cpp
// 기존: 복잡하고 중복된 코드가 많은 함수
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 여러 조건 체크가 한꺼번에 진행됨
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
        return;

    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 2.0f, false);

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // 플랭크와 텐트 처리 코드가 거의 동일하지만 중복됨
    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            // 플랭크 생성 코드 (약 40줄)
            FActorSpawnParameters SpawnParams;
            // ... 생략 ...
            
            // 물리 및 충돌 설정 (텐트와 중복)
            SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
            SpawnedPlank->MeshComponent->SetEnableGravity(false);
            // ... 생략 ...
            
            // 인벤토리 체크 (텐트와 중복)
            if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
            {
                // ... 생략 ...
            }
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        // 텐트에 대한 거의 동일한 코드 블록이 반복됨
        // ... 생략 ...
    }

    // 상태 업데이트와 네트워크 동기화가 섞여 있음
    if (bIsValidPlacement)
    {
        // ... 생략 ...
    }
}
```

**리팩토링 후**:
```cpp
// 개선: 템플릿 패턴 적용으로 간결하고 명확해진 함수
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 유효성 검사를 별도 함수로 분리
    if (!ValidateBuildAttempt())
        return;

    // 기본 정보 가져오기
    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // 템플릿 함수로 아이템 타입에 관계없이 일관된 처리 (중복 제거)
    if (CurrentBuildingItem == EInventorySlot::Plank && OwnerCitizen->UseItem(EInventorySlot::Plank))
    {
        SpawnBuildingItem<AItem_Plank>(PlankClass, Location, Rotation);
    }
    else if (CurrentBuildingItem == EInventorySlot::Tent && OwnerCitizen->UseItem(EInventorySlot::Tent))
    {
        SpawnBuildingItem<AItem_Tent>(TentClass, Location, Rotation);
    }

    // 상태 업데이트를 별도 함수로 분리
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

**리팩토링 전**:
```cpp
// 중복 코드 - 플랭크 생성
if (OwnerCitizen->UseItem(EInventorySlot::Plank))
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCitizen;
    // ... 약 40줄의 유사한 코드 ...
}

// 중복 코드 - 텐트 생성
if (OwnerCitizen->UseItem(EInventorySlot::Tent))
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCitizen;
    // ... 약 40줄의 유사한 코드 ...
}
```

**리팩토링 후**:
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

이번 스프린트에서 가장 큰 변화가 있었던 부분은 `BuildingComponent` 클래스입니다. 이 클래스는 건설 시스템의 핵심 로직을 담당하는 컴포넌트로, 텐트와 판자 설치 등 게임의 중요한 메커니즘을 처리합니다. 기존 코드는 기능적으로는 작동했으나, 많은 중복 코드와 긴 함수들로 인해 유지보수와 확장이 어려운 상태였습니다.

### 4.1 BuildingComponent 클래스
가장 광범위한 리팩토링이 이루어진 영역입니다. 특히 건설 시도 함수(AttemptBuild)의 변화가 두드러집니다:

#### 건설 시도 함수 (AttemptBuild) 비교

**리팩토링 전**:
```cpp
// 기존: 복잡하고 중복된 코드가 많은 함수
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 여러 조건 체크가 한꺼번에 진행됨
    if (!BuildPreviewMesh || !OwnerCitizen || !bCanBuildNow || !bIsValidPlacement || bIsBuilding || !GetOwner()->HasAuthority())
        return;

    bCanBuildNow = false;
    GetWorld()->GetTimerManager().SetTimer(BuildDelayTimerHandle, this, &UBuildingComponent::ResetBuildDelay, 2.0f, false);

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // 플랭크와 텐트 처리 코드가 거의 동일하지만 중복됨
    if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
    {
        if (OwnerCitizen->UseItem(EInventorySlot::Plank))
        {
            // 플랭크 생성 코드 (약 40줄)
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCitizen;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AItem_Plank* SpawnedPlank = GetWorld()->SpawnActor<AItem_Plank>(
                PlankClass,
                Location,
                Rotation,
                SpawnParams
            );

            if (SpawnedPlank)
            {
                SpawnedPlank->SetReplicates(true);
                SpawnedPlank->SetReplicateMovement(true);
                SpawnedPlank->bIsBuiltItem = true;

                if (SpawnedPlank->MeshComponent)
                {
                    // BPАЗ ±вє» Е©±вїН јіБ¤А» °ЎБ®їИ
                    AItem_Plank* DefaultPlank = Cast<AItem_Plank>(PlankClass.GetDefaultObject());
                    if (DefaultPlank && DefaultPlank->MeshComponent)
                    {
                        SpawnedPlank->MeshComponent->SetStaticMesh(DefaultPlank->MeshComponent->GetStaticMesh());
                        SpawnedPlank->MeshComponent->SetWorldScale3D(DefaultPlank->MeshComponent->GetRelativeScale3D());
                        FTransform NewTransform = DefaultPlank->MeshComponent->GetRelativeTransform();
                        NewTransform.SetLocation(Location);
                        NewTransform.SetRotation(Rotation.Quaternion());
                        SpawnedPlank->SetActorTransform(NewTransform);
                    }

                    // №°ё®/Гжµ№ јіБ¤
                    SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
                    SpawnedPlank->MeshComponent->SetEnableGravity(false);
                    SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
                    SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

                    // №°ё® »уЕВ є№Б¦ јіБ¤
                    SpawnedPlank->MeshComponent->SetIsReplicated(true);
                    SpawnedPlank->MeshComponent->SetMobility(EComponentMobility::Movable);
                    SpawnedPlank->MeshComponent->bReplicatePhysicsToAutonomousProxy = true;

                    // А§ДЎ °нБ¤А» А§ЗС јіБ¤
                    SpawnedPlank->MeshComponent->SetWorldLocation(Location);
                    SpawnedPlank->MeshComponent->SetWorldRotation(Rotation);

                    SpawnedPlank->ForceNetUpdate();
                }

                if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
                {
                    FItemData* ItemData = InvenComp->GetItemData(EInventorySlot::Plank);
                    if (ItemData && ItemData->Count <= 0)
                    {
                        if (UPlayerModeComponent* ModeComp = OwnerCitizen->GetPlayerModeComponent())
                        {
                            DeactivateBuildMode();
                            ModeComp->SetPlayerMode(EPlayerMode::Normal);
                        }
                    }
                }
            }
        }
    }
    else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
    {
        // 텐트에 대한 거의 동일한 코드 블록이 반복됨
        // 코드가 약 40줄 정도 중복됨
    }

    // 상태 업데이트와 네트워크 동기화가 섞여 있음
    if (bIsValidPlacement)
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
}
```

**리팩토링 후**:
```cpp
// 개선: 템플릿 패턴 적용으로 간결하고 명확해진 함수
void UBuildingComponent::AttemptBuild_Implementation()
{
    // 유효성 검사를 별도 함수로 분리
    if (!ValidateBuildAttempt())
        return;

    // 기본 정보 가져오기
    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    // 템플릿 함수로 아이템 타입에 관계없이 일관된 처리 (중복 제거)
    if (CurrentBuildingItem == EInventorySlot::Plank && OwnerCitizen->UseItem(EInventorySlot::Plank))
    {
        SpawnBuildingItem<AItem_Plank>(PlankClass, Location, Rotation);
    }
    else if (CurrentBuildingItem == EInventorySlot::Tent && OwnerCitizen->UseItem(EInventorySlot::Tent))
    {
        SpawnBuildingItem<AItem_Tent>(TentClass, Location, Rotation);
    }

    // 상태 업데이트를 별도 함수로 분리
    UpdateBuildState();
}
```

리팩토링 후 코드는 훨씬 간결해졌으며, 책임이 명확하게 분리되었습니다. 코드를 검토하며 특히 인상 깊었던 점은 템플릿 패턴의 적용을 통해 중복 코드를 효과적으로 제거한 것입니다.

### 4.2 템플릿 패턴 적용
중복 코드를 제거하기 위해 템플릿 함수를 도입한 것이 가장 큰 개선점입니다:

```cpp
// 템플릿 함수로 아이템 생성 로직 통합 (중복 제거)
template<class T>
T* UBuildingComponent::SpawnBuildingItem(TSubclassOf<T> ItemClass, const FVector& Location, const FRotator& Rotation)
{
    // 아이템 생성
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
            // 물리/충돌 설정을 별도 함수로 분리
            ConfigureBuildingItemPhysics(SpawnedItem->MeshComponent, Location, Rotation);
        }

        // 인벤토리 상태 체크를 별도 함수로 분리
        CheckInventoryAfterBuilding(SpawnedItem);

        return SpawnedItem;
    }

    return nullptr;
}
```

AI 조력자의 제안을 검토하면서, 템플릿 함수를 사용하는 이 패턴이 코드 중복을 획기적으로 줄이면서도 타입 안전성을 유지하는 좋은 방법임을 확인했습니다. 이는 C++의 강력한 템플릿 기능을 활용한 좋은 사례입니다.

### 4.3 물리 설정 함수 분리
`ConfigureBuildingItemPhysics` 함수를 별도로 분리하여 코드의 재사용성과 가독성을 높였습니다:

**리팩토링 전**:
```cpp
// 리팩토링 전: 각 아이템 생성 코드마다 물리 설정 중복
SpawnedPlank->MeshComponent->SetSimulatePhysics(false);
SpawnedPlank->MeshComponent->SetEnableGravity(false);
SpawnedPlank->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
SpawnedPlank->MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
SpawnedPlank->MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
SpawnedPlank->MeshComponent->SetIsReplicated(true);
SpawnedPlank->MeshComponent->SetMobility(EComponentMobility::Movable);
SpawnedPlank->MeshComponent->bReplicatePhysicsToAutonomousProxy = true;
SpawnedPlank->MeshComponent->SetWorldLocation(Location);
SpawnedPlank->MeshComponent->SetWorldRotation(Rotation);

// 텐트 생성 코드에서도 동일한 코드 반복
SpawnedTent->MeshComponent->SetSimulatePhysics(false);
// ... 동일한 코드 반복 ...
```

**리팩토링 후**:
```cpp
// 물리/충돌 설정을 별도 함수로 분리
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

### 4.4 네트워크 복제 및 상태 관리 개선
네트워크 상태 관리와 복제 관련 로직도 개선했습니다:

**리팩토링 전**:
```cpp
// 리팩토링 전: 상태 변경 후 네트워크 업데이트가 일관되지 않음
void UBuildingComponent::OnRep_BuildState()
{
    // 상태에 따른 시각적 처리가 복잡하게 얽혀있음
    if (BuildPreviewMesh)
    {
        if (bIsBuilding)
        {
            BuildPreviewMesh->SetVisibility(false);
            if (BuildPreviewMesh->GetStaticMesh())
            {
                BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                BuildPreviewMesh->SetSimulatePhysics(false);
            }
        }
        else if (bCanBuildNow)
        {
            UpdateBuildPreview();
            BuildPreviewMesh->SetVisibility(true);
            BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial);
        }
        else
        {
            BuildPreviewMesh->SetVisibility(false);
        }
    }

    if (OwnerCitizen)
    {
        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(bIsBuilding ? MOVE_None : MOVE_Walking);
        }
    }
}
```

**리팩토링 후**:
```cpp
// 리팩토링 후: 상태별 명확한 처리
void UBuildingComponent::OnRep_BuildState()
{
    UpdateOwnerBuildState();
    UpdatePreviewVisuals(bIsValidPlacement);
}

void UBuildingComponent::UpdateOwnerBuildState()
{
    // 캐릭터 상태 업데이트 로직
    if (OwnerCitizen)
    {
        if (UCharacterMovementComponent* MovementComp = OwnerCitizen->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(bIsBuilding ? MOVE_None : MOVE_Walking);
        }
    }
}

void UBuildingComponent::UpdatePreviewVisuals(bool bValid)
{
    // 시각적 피드백 업데이트 로직
    if (BuildPreviewMesh)
    {
        if (bIsBuilding)
        {
            BuildPreviewMesh->SetVisibility(false);
        }
        else if (bCanBuildNow)
        {
            BuildPreviewMesh->SetVisibility(true);
            BuildPreviewMesh->SetMaterial(0, bValid ? ValidPlacementMaterial : InvalidPlacementMaterial);
        }
        else
        {
            BuildPreviewMesh->SetVisibility(false);
        }
    }
}
```

### 4.5 AI 조력자(클로드)를 활용한 체계적 리팩토링
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

특히 C++의 강력한 템플릿 기능과 객체지향 패턴을 제대로 활용하지 못했던 점을 깨닫고, 이번 기회에 체계적으로 개선했습니다. AI 조력자(클로드)의 도움으로 SOLID 원칙에 따른 리팩토링 방향을 찾을 수 있었고, 그 제안을 바탕으로 직접 코드를 수정하고 테스트하면서 리팩토링의 효과를 체감했습니다.

특히 인상 깊었던 점은 단일 책임 원칙(SRP)에 따라 함수를 분리하고, 템플릿 메소드 패턴을 적용했을 때의 변화였습니다. 예를 들어, `BuildingComponent` 클래스의 리팩토링 후에는 하나의 기능을 수정할 때 그 기능을 담당하는 함수만 건드리면 되어 변경의 영향 범위가 크게 줄어들었습니다.

또한 템플릿 함수를 사용해 중복 코드를 제거한 것은 매우 효과적이었습니다. 기존에는 플랭크와 텐트 생성 코드가 거의 동일하게 반복되었지만, 템플릿 패턴 적용 후에는 코드의 양이 크게 줄어들고 확장성도 좋아졌습니다. 이제 새로운 건설 아이템을 추가하더라도 최소한의 코드만 작성하면 됩니다.

AI 조력자를 활용한 리팩토링 과정에서 제안된 내용을 단순히 적용하는 것이 아니라, 각 제안을 직접 검토하고 테스트하며 적용 여부를 판단했습니다. 이 과정에서 객체지향 설계의 원칙과 패턴에 대한 이해가 크게 향상되었습니다.

리팩토링의 결과로 얻은 코드는 가독성과 유지보수성이 크게 향상되었으며, 새로운 기능을 추가하거나 기존 기능을 수정할 때 훨씬 더 효율적인 작업이 가능해졌습니다. 앞으로의 개발에서도 이러한 원칙과 패턴을 꾸준히 적용하여 더 깔끔하고 확장 가능한 코드를 작성해 나가겠습니다.
