# 브릿지런 리팩토링 분석: SOLID 원칙 적용 사례

## 1. 개요

이 문서는 브릿지런 게임의 `BuildingComponent` 클래스에 적용된 리팩토링을 분석합니다. 특히 SOLID 원칙과 객체지향 디자인 패턴을 적용하여 코드의 품질을 개선한 방법에 초점을 맞추고 있습니다.

## 2. 단일 책임 원칙 (Single Responsibility Principle) 적용

### 2.1 리팩토링 전: 여러 책임이 혼합된 함수

```cpp
// 리팩토링 전: 모든 책임이 한 함수에 혼재
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

### 2.2 리팩토링 후: 명확한 책임 분리

```cpp
// 리팩토링 후: 명확한 책임 분리
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

### 2.3 개선 효과

1. **가독성 향상**: 각 함수가 하나의 명확한 책임만을 갖게 되어 코드 흐름을 이해하기 쉬워졌습니다.
2. **디버깅 용이성**: 문제 발생 시 관련된 특정 함수만 검사하면 되므로 디버깅이 간편해졌습니다.
3. **테스트 용이성**: 작은 기능 단위로 분리되어 단위 테스트가 용이해졌습니다.
4. **코드 재사용성**: 분리된 함수들은 다른 맥락에서도 재사용할 수 있게 되었습니다.
5. **유지보수성 향상**: 특정 기능 수정 시 해당 함수만 변경하면 되므로 유지보수가 쉬워졌습니다.

## 3. 템플릿 메서드 패턴 적용

### 3.1 리팩토링 전: 중복된 코드 블록

```cpp
// 리팩토링 전: 중복 코드
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
    // 거의 동일한 코드가 텐트 처리를 위해 반복됨
    // ...
}
```

### 3.2 리팩토링 후: 템플릿 함수 적용

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
            // 물리/충돌 설정을 별도 함수로 분리
            ConfigureBuildingItemPhysics(SpawnedItem->MeshComponent, Location, Rotation);
        }

        // 인벤토리 상태 체크를 별도 함수로 분리
        CheckInventoryAfterBuilding(SpawnedItem);

        return SpawnedItem;
    }

    return nullptr;
}

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

### 3.3 개선 효과

1. **코드 중복 제거**: 거의 동일하게 반복되던 코드가 하나의 템플릿 함수로 통합되었습니다.
2. **타입 안전성**: 템플릿을 사용함으로써 타입 안전한 코드가 되었습니다.
3. **유지보수성 향상**: 기능 변경 시 한 곳만 수정하면 되므로 유지보수가 쉬워졌습니다.
4. **확장성 증가**: 새로운 건설 아이템 추가 시 기존 코드 수정 없이 템플릿 함수 재사용이 가능합니다.
5. **코드 길이 감소**: 반복 코드 제거로 전체 코드 길이가 줄어들었습니다.

## 4. 개방-폐쇄 원칙 (Open-Closed Principle) 적용

### 4.1 리팩토링 전: 변경에 닫혀있지 않은 구조

원래 코드에서는 새로운 건설 아이템 타입이 추가될 때마다 `AttemptBuild_Implementation` 함수를 직접 수정해야 했습니다. 각 아이템 타입별로 거의 동일한 코드 블록이 중복되어 있어 확장 시 여러 부분을 수정해야 했습니다.

```cpp
// 리팩토링 전: if-else 조건문으로 타입별 처리
if (PlankClass && CurrentBuildingItem == EInventorySlot::Plank)
{
    // 플랭크 처리 로직...
}
else if (TentClass && CurrentBuildingItem == EInventorySlot::Tent)
{
    // 텐트 처리 로직...
}
// 새로운 아이템 추가 시 여기에 else if 블록을 추가해야 함
```

### 4.2 리팩토링 후: 변경에는 열려있고 수정에는 닫혀있는 구조

리팩토링 후에는 템플릿 함수와 타입 추상화를 통해 새로운 건설 아이템 추가 시 기존 코드를 수정하지 않고도 확장할 수 있게 되었습니다.

```cpp
// 리팩토링 후: 템플릿 함수를 통한 추상화
if (CurrentBuildingItem == EInventorySlot::Plank && OwnerCitizen->UseItem(EInventorySlot::Plank))
{
    SpawnBuildingItem<AItem_Plank>(PlankClass, Location, Rotation);
}
else if (CurrentBuildingItem == EInventorySlot::Tent && OwnerCitizen->UseItem(EInventorySlot::Tent))
{
    SpawnBuildingItem<AItem_Tent>(TentClass, Location, Rotation);
}
// 새로운 아이템 추가 시 같은 패턴으로 확장 가능
```

또한 물리/충돌 설정과 같은 공통 기능을 별도 함수로 분리하여 중앙 집중화했습니다.

```cpp
// 공통 설정을 별도 함수로 분리하여 중앙 집중화
void UBuildingComponent::ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, const FVector& Location, const FRotator& Rotation)
{
    // 공통 물리 설정 로직
}
```

### 4.3 개선 효과

1. **확장성 향상**: 새로운 건설 아이템 추가 시 기존 코드 수정 없이 확장 가능합니다.
2. **중앙 집중화된 설정**: 공통 설정 로직이 한 곳에 집중되어 있어 변경 시 한 곳만 수정하면 됩니다.
3. **일관된 동작 보장**: 모든 아이템이 동일한 설정 함수를 통해 처리되므로 일관성이 높아졌습니다.
4. **버그 발생 가능성 감소**: 중복 코드 제거로 실수로 인한 버그 가능성이 줄어들었습니다.
5. **코드 재사용성**: 분리된 함수는 다른 컨텍스트에서도 재사용 가능합니다.

## 5. 함수 분할 및 구조화

### 5.1 리팩토링 전: 큰 규모의 복잡한 함수

```cpp
void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    if (!OwnerCitizen)
        return;

    // BuildPreviewMesh 초기화
    if (!BuildPreviewMesh)
    {
        BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
        if (!BuildPreviewMesh)
            return;

        BuildPreviewMesh->SetupAttachment(OwnerCitizen->GetRootComponent());
        BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BuildPreviewMesh->SetVisibility(false);
        BuildPreviewMesh->SetIsReplicated(true);
        BuildPreviewMesh->bOnlyOwnerSee = false;
        BuildPreviewMesh->SetEnableGravity(false);
        BuildPreviewMesh->RegisterComponent();
    }

    // 아이템 설정 (100줄 이상의 코드...)
    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();

        switch (CurrentBuildingItem)
        {
        case EInventorySlot::Plank:
            // 플랭크 설정 코드...
            break;

        case EInventorySlot::Tent:
            // 텐트 설정 코드...
            break;
        }
    }

    // 상태 및 시각화 설정 코드...
}
```

### 5.2 리팩토링 후: 작고 명확한 목적을 가진 함수들

```cpp
void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    if (!OwnerCitizen)
        return;

    // 프리뷰 메시 초기화를 별도 함수로 분리
    InitializeBuildPreviewMesh();

    // 현재 선택된 아이템 가져오기
    if (UInvenComponent* InvenComp = OwnerCitizen->GetInvenComponent())
    {
        CurrentBuildingItem = InvenComp->GetCurrentSelectedSlot();
        
        // 현재 아이템에 따른 메시 설정을 별도 함수로 분리
        SetupPreviewMeshForCurrentItem();
    }

    // 상태 업데이트 및 네트워크 동기화
    if (BuildPreviewMesh)
    {
        bIsValidPlacement = false;
        UpdatePreviewVisuals(false);
    }

    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }
}

// 프리뷰 메시 초기화 전용 함수
void UBuildingComponent::InitializeBuildPreviewMesh()
{
    // 초기화 코드...
}

// 아이템 타입별 설정 함수
void UBuildingComponent::SetupPreviewMeshForCurrentItem()
{
    // 타입별 설정 코드...
}

// 플랭크 전용 설정 함수
void UBuildingComponent::SetupPlankPreviewMesh()
{
    // 플랭크 설정 코드...
}

// 텐트 전용 설정 함수
void UBuildingComponent::SetupTentPreviewMesh()
{
    // 텐트 설정 코드...
}
```

### 5.3 개선 효과

1. **높은 가독성**: 각 함수가 명확한 이름과 목적을 가져 코드 이해가 쉬워졌습니다.
2. **관심사 분리**: 각 함수가 하나의 작업에만 집중하여 코드의 복잡성이 감소했습니다.
3. **유지보수성 향상**: 특정 기능만 수정할 때 관련 함수만 찾아 변경하면 되므로 유지보수가 쉬워졌습니다.
4. **디버깅 용이성**: 작은 함수 단위로 분할되어 문제 발생 시 디버깅이 용이해졌습니다.
5. **코드 재사용성**: 작은 기능 단위로 분리된 함수를 다른 맥락에서도 재사용할 수 있게 되었습니다.

## 6. 명명 규칙 및 가독성 개선

### 6.1 리팩토링 전: 불명확한 함수명과 주석 부족

```cpp
bool UBuildingComponent::CheckLocation(const FVector& Location)
{
    // 코드...
}

void UBuildingComponent::UpdateMaterialState()
{
    // 코드...
}
```

### 6.2 리팩토링 후: 명확한 함수명과 개선된 주석

```cpp
// 건설 위치의 유효성을 검증하는 함수
bool UBuildingComponent::ValidateBuildLocation(const FVector& Location)
{
    // 코드...
}

// 프리뷰 메시의 시각적 상태를 업데이트하는 함수
void UBuildingComponent::UpdatePreviewVisuals(bool bValid)
{
    // 코드...
}
```

### 6.3 개선 효과

1. **자기 문서화 코드**: 명확한 함수명으로 코드 자체가 문서 역할을 합니다.
2. **이해도 향상**: 함수의 목적과 책임이 이름만으로도 명확하게 드러납니다.
3. **코드 탐색 용이성**: 의미 있는 이름으로 필요한 함수를 쉽게 찾을 수 있습니다.
4. **유지보수성 향상**: 코드의 의도가 명확해져 유지보수가 쉬워졌습니다.
5. **협업 효율성**: 다른 개발자도 코드의 목적을 빠르게 이해할 수 있습니다.

## 7. 종합 평가

### 7.1 성과

1. **코드 품질 향상**:
   - 중복 코드 제거로 버그 발생 가능성 감소
   - 책임 분리로 각 함수의 가독성 및 테스트 용이성 증가
   - 명확한 함수명과 구조로 코드 이해도 향상

2. **유지보수성 개선**:
   - 작은 함수 단위로 나뉘어 변경 영향 범위 최소화
   - 함수 재사용성 증가로 추가 개발 시간 단축
   - 확장에 용이한 구조로 새로운 기능 추가가 간편해짐

3. **성능 최적화**:
   - 필요한 시점에만 상태 업데이트 수행으로 불필요한 연산 감소
   - 네트워크 동기화 최적화로 대역폭 사용 감소
   - 성능 병목 지점 식별 및 해결

4. **확장성 향상**:
   - 새로운 건설 아이템 추가가 용이해짐
   - 게임 메커니즘 변경에 따른 코드 수정 범위 최소화
   - 팀 개발 시 역할 분담이 명확해짐

### 7.2 주요 개선 패턴

1. **단일 책임 원칙 적용**:
   - 각 함수가 하나의 책임만 가지도록 설계
   - 관심사 분리를 통한 복잡성 감소

2. **템플릿 메서드 패턴 적용**:
   - 중복 코드를 템플릿 함수로 통합
   - 타입 안전성 보장 및 코드 재사용성 향상

3. **개방-폐쇄 원칙 적용**:
   - 확장에 열려있고 수정에 닫혀있는 구조
   - 새로운 기능 추가 시 기존 코드 수정 최소화

4. **함수 분할 및 구조화**:
   - 큰 함수를 작고 명확한 목적을 가진 함수들로 분할
   - 코드의 가독성 및 유지보수성 향상

5. **명명 규칙 및 가독성 개선**:
   - 명확한 함수명과 변수명 사용
   - 주석 추가 및 코드 구조화

### 7.3 결론

SOLID 원칙과 객체지향 디자인 패턴을 적용한 리팩토링을 통해 브릿지런 게임의 코드 품질이 크게 향상되었습니다. 특히 템플릿 함수 도입을 통한 코드 중복 제거와 단일 책임 원칙에 따른 함수 분리가 가장 큰 개선점으로 평가됩니다.

이러한 개선으로 코드의 가독성, 유지보수성, 확장성이 크게 향상되었으며, 향후 개발에서도 이러한 원칙을 지속적으로 적용함으로써 더 깔끔하고 효율적인 코드베이스를 유지할 수 있을 것입니다.
