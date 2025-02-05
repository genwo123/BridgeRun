# 브릿지런 개발일지 (스프린트 5)

## 📅 개발 기간
2025년 1월 13일 ~ 2025년 2월 03일

## 👨‍💻 작성자
김건우

## 1. 주요 개발 목표
스프린트 5에서는 실시간 멀티플레이어 게임을 위한 네트워크 시스템 안정화와 버그 수정에 집중했습니다:

* 판자 설치 시스템의 물리/충돌 네트워크 동기화 개선
* 프리뷰 시스템의 클라이언트-서버 동기화 문제 해결
* 망원경 아이템 중복 배급 버그 수정

## 2. 물리/충돌 시스템 구현

### 2.1 판자 물리 동기화 문제
현재까지 다음과 같은 물리/충돌 관련 문제가 있었습니다:

* 서버에서는 판자 위에 올라갈 수 있지만, 클라이언트에서는 충돌은 있으나 올라가지지 않음
* 클라이언트에서 판자가 설치될 때 공중에 떠있는 현상 발생
* 클라이언트 간 물리 동작의 불일치 문제

### 2.2 해결 방법
코드를 다음과 같이 수정하여 물리/충돌 시스템을 개선했습니다:

```cpp
void AItem_Plank::BeginPlay()
{
    Super::BeginPlay();
    if (MeshComponent)
    {
        // 물리/충돌 설정 강화
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);    
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;             
        MeshComponent->SetMobility(EComponentMobility::Movable);              
        MeshComponent->SetSimulatePhysics(true);                             
        MeshComponent->SetEnableGravity(true);                               
        MeshComponent->SetGenerateOverlapEvents(true);                       
    }
}
```

설치 시 네트워크 동기화 코드도 강화했습니다:

```cpp
void AItem_Plank::OnPlaced_Implementation()
{
    if (!HasAuthority()) return;
    
    bIsBuiltPlank = true;
    
    if (MeshComponent)
    {
        // 네트워크/물리 설정 강화
        MeshComponent->SetMobility(EComponentMobility::Movable);
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->bReplicatePhysicsToAutonomousProxy = true;           
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);        
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComponent->SetEnableGravity(true);                             
        MeshComponent->SetGenerateOverlapEvents(true);                     
        
        // 위치 동기화 강화
        MeshComponent->UpdateComponentToWorld();                           
        
        MulticastSetMobility(EComponentMobility::Movable);
    }
    
    ForceNetUpdate();
}
```

### 2.3 주요 변경점
* 충돌 응답 변경
  * ECR_Overlap → ECR_Block (ECC_Pawn 채널)
  * 모든 채널에 대한 Block 응답 설정

* 물리 설정 강화
  * bReplicatePhysicsToAutonomousProxy 추가
  * SetEnableGravity 추가
  * SetGenerateOverlapEvents 추가

* 네트워크 동기화 강화
  * UpdateComponentToWorld 추가
  * Transform 업데이트 강제화

## 3. 프리뷰 시스템 동기화

### 3.1 프리뷰 네트워크 문제
프리뷰 시스템에서 다음과 같은 네트워크 관련 문제가 있었습니다:

* 리플리케이션 순서 문제
  * bIsValidPlacement가 복제될 때 타이밍이 적절하지 않음
  * 서버에서 상태 업데이트와 클라이언트로의 전파 사이에 지연 발생

* 머티리얼 업데이트 로직 불일치
  * UpdateBuildPreview에서 로컬 컨트롤 여부에 따라 다른 값 사용
  * 서버와 클라이언트간 시각적 상태 불일치 발생

### 3.2 구현 코드
프리뷰 메시의 네트워크 동기화를 위해 다음과 같이 코드를 수정했습니다:

```cpp
void UBuildingComponent::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !OwnerCitizen)
        return;

    // 서버에서만 위치/회전 업데이트 수행
    if (GetOwner()->HasAuthority())
    {
        BuildPreviewMesh->SetWorldLocation(PreviewLocation);
        BuildPreviewMesh->SetWorldRotation(PreviewRotation);
        GetOwner()->ForceNetUpdate();
    }

    // 머티리얼은 모든 클라이언트에서 업데이트
    BuildPreviewMesh->SetMaterial(0, bIsValidPlacement ? 
        ValidPlacementMaterial : InvalidPlacementMaterial);
    BuildPreviewMesh->SetVisibility(true);
}
```

초기화 코드도 보강했습니다:

```cpp
void UBuildingComponent::OnBuildModeEntered_Implementation()
{
    if (!BuildPreviewMesh)
    {
        BuildPreviewMesh = NewObject<UStaticMeshComponent>(OwnerCitizen, TEXT("BuildPreviewMesh"));
        if (BuildPreviewMesh)
        {
            BuildPreviewMesh->SetIsReplicated(true);  // 네트워크 복제 활성화
            BuildPreviewMesh->bOnlyOwnerSee = false;  // 모든 클라이언트가 볼 수 있도록 설정
            BuildPreviewMesh->SetEnableGravity(false);  // 물리 영향 받지 않도록 설정
            BuildPreviewMesh->RegisterComponent();
        }
    }
}
```

## 4. 망원경 시스템 개선

### 4.1 망원경 위치 설정
망원경의 위치를 더 안정적으로 설정하도록 수정했습니다:

```cpp
void AItem_Telescope::OnRep_HeldState()
{
    if (bIsHeld && GetOwner())
    {
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepWorld,
            true
        );
        AttachToActor(GetOwner(), AttachRules);
        
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            SetActorRelativeLocation(FVector(80.0f, 30.0f, 50.0f));
            SetActorRotation(Character->GetActorRotation());
        }
    }
}
```

### 4.2 중복 지급 문제 해결
서버에서만 아이템을 지급하도록 수정했습니다:

```cpp
void ACitizen::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())  // 서버에서만 실행
    {
        // 망원경은 플레이어당 1개만 지급
        FItemData* TelescopeData = InvenComponent ? 
            InvenComponent->GetItemData(EInventorySlot::Telescope) : nullptr;
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }
}
```

## 5. 현재까지의 구현 결과
현재까지 다음과 같은 시스템들이 개선되었습니다:

* 판자 설치 시스템
  * 물리/충돌 시스템 안정화
  * 네트워크 동기화 개선
  * 클라이언트 권한 관리 강화

* 프리뷰 시스템
  * 서버-클라이언트 간 동기화 개선
  * 머티리얼 업데이트 최적화
  * 가시성 문제 해결

* 아이템 시스템
  * 망원경 중복 지급 버그 수정
  * 위치 동기화 안정화
  * 네트워크 권한 관리 개선

## 6. 다음 스프린트 계획

### 6.1 네트워크 시스템 안정화
* 프리뷰 시스템 완성
* 리플리케이션 순서 최적화
* 클라이언트 권한 검증 강화

### 6.2 게임플레이 시스템 확장
* 팀 시스템 구현
* 점수 시스템 확장
* 게임 모드 다양화

### 6.3 성능 최적화
* 네트워크 트래픽 최적화
* 물리 연산 최적화
* 메모리 사용량 개선

## 7. 회고 및 느낀점
이번 스프린트에서는 멀티플레이어 게임의 핵심인 네트워크 동기화 시스템 안정화에 집중했습니다. 특히 판자 설치 시스템의 물리/충돌 네트워크 동기화 개선과 프리뷰 시스템의 클라이언트-서버 동기화 문제 해결에서 많은 진전이 있었습니다.

명절을 껴서 3주의 기간을 가졌음에도 불구하고, 실제로는 핵심적인 네트워크 이슈 해결에만 시간을 투자했습니다. 특히 판자 설치 프리뷰의 동기화 문제는 여전히 완벽하게 해결하지 못한 상태이지만, 현재는 기능적으로 작동하는 수준까지는 구현되었습니다.

이번 스프린트를 통해 언리얼 엔진의 네트워크 시스템, 특히 물리 동기화와 권한 관리에 대한 이해도가 크게 향상되었습니다. 다음 스프린트에서는 남은 네트워크 이슈들을 해결하고 게임의 핵심 시스템들을 확장해 나갈 계획입니다.
