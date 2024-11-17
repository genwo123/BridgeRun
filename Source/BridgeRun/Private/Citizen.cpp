// Citizen.cpp
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "InvenComponent.h"
#include "BuildableZone.h" 

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 회전 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터가 이동 방향으로 자연스럽게 회전하도록 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // 스프링암 생성
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    // 카메라 생성
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    // 인벤토리 컴포넌트 생성
    InvenComponent = CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));

    // BuildPreviewMesh 초기화
    BuildPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildPreviewMesh"));
    BuildPreviewMesh->SetupAttachment(RootComponent);
    BuildPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BuildPreviewMesh->SetVisibility(false);

    // 기본값 초기화는 InvenComponent에서 처리
}

void ACitizen::BeginPlay()
{
    Super::BeginPlay();

    // 인벤토리 UI 생성
    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 건설 관련 아이템이 선택되었을 때만 프리뷰 업데이트
    if (InvenComponent &&
        (InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Plank ||
            InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Tent))
    {
        UpdateBuildPreview();
    }
}

// 이동 관련 함수들은 그대로 유지
void ACitizen::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::Turn(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value);
    }
}

void ACitizen::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerPitchInput(Value);
    }
}

void ACitizen::StartJump()
{
    bPressedJump = true;
}

void ACitizen::StopJump()
{
    bPressedJump = false;
}

void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 이동
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // 점프
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);

    // 인벤토리 슬롯 선택
    PlayerInputComponent->BindAction("Slot1", IE_Pressed, this, &ACitizen::SelectSlot1);
    PlayerInputComponent->BindAction("Slot2", IE_Pressed, this, &ACitizen::SelectSlot2);
    PlayerInputComponent->BindAction("Slot3", IE_Pressed, this, &ACitizen::SelectSlot3);
    PlayerInputComponent->BindAction("Slot4", IE_Pressed, this, &ACitizen::SelectSlot4);
    PlayerInputComponent->BindAction("Slot5", IE_Pressed, this, &ACitizen::SelectSlot5);

    // 건설 관련
    PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, this, &ACitizen::RotateBuildPreview);
    PlayerInputComponent->BindAction("Build", IE_Pressed, this, &ACitizen::AttemptBuild);

    //상호 작용
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);
}

void ACitizen::SelectInventorySlot(EInventorySlot Slot)
{
    if (InvenComponent)
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count > 0)
        {
            if (InvenComponent->GetCurrentSelectedSlot() == Slot)
            {
                InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                BuildPreviewMesh->SetVisibility(false);
            }
            else
            {
                InvenComponent->SetCurrentSelectedSlot(Slot);
                if (Slot == EInventorySlot::Plank || Slot == EInventorySlot::Tent)
                {
                    BuildPreviewMesh->SetVisibility(true);
                }
                else
                {
                    BuildPreviewMesh->SetVisibility(false);
                }
            }
        }
    }
}

void ACitizen::UpdateBuildPreview()
{
    if (!BuildPreviewMesh || !InvenComponent) return;

    EInventorySlot CurrentSlot = InvenComponent->GetCurrentSelectedSlot();
    if (CurrentSlot != EInventorySlot::Plank && CurrentSlot != EInventorySlot::Tent)
    {
        BuildPreviewMesh->SetVisibility(false);
        return;
    }
    BuildPreviewMesh->SetVisibility(true);

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    FVector Start, Dir;
    PC->DeprojectMousePositionToWorld(Start, Dir);
    FVector End = Start + (Dir * MaxBuildDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    bool bValidPlacement = false;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (ABuildableZone* Zone = Cast<ABuildableZone>(HitResult.GetActor()))
        {
            // 좌우 설치 포인트 계산
            FVector LeftPoint = HitResult.Location;
            FVector RightPoint = LeftPoint + FVector(0.0f, Zone->BridgeWidth, 0.0f);

            // 판자와 천막의 설치 가능 여부 체크
            if (CurrentSlot == EInventorySlot::Plank)  // CurrentSelectedSlot을 CurrentSlot으로 변경
            {
                bValidPlacement = Zone->IsPlankPlacementValid(LeftPoint, RightPoint);
            }
            else if (CurrentSlot == EInventorySlot::Tent)  // CurrentSelectedSlot을 CurrentSlot으로 변경
            {
                bValidPlacement = Zone->IsTentPlacementValid(LeftPoint, RightPoint);
            }

            // 프리뷰 위치 및 회전 설정
            BuildPreviewMesh->SetWorldLocation(LeftPoint);
            BuildPreviewMesh->SetWorldRotation(FRotator::ZeroRotator);

            // 디버그용 시각화 (개발 중에만 사용)
            DrawDebugLine(GetWorld(), LeftPoint, RightPoint,
                bValidPlacement ? FColor::Green : FColor::Red,
                false, -1.0f, 0, 2.0f);
        }
    }

    // 프리뷰 머티리얼 업데이트
    if (ValidPlacementMaterial && InvalidPlacementMaterial)
    {
        BuildPreviewMesh->SetMaterial(0, bValidPlacement ?
            ValidPlacementMaterial : InvalidPlacementMaterial);
    }

    // 디버그 메시지
    GEngine->AddOnScreenDebugMessage(-1, 0.1f,
        bValidPlacement ? FColor::Green : FColor::Red,
        FString::Printf(TEXT("Can Place: %s"),
            bValidPlacement ? TEXT("Yes") : TEXT("No")));
}

void ACitizen::RotateBuildPreview()
{
    if (InvenComponent && InvenComponent->GetCurrentSelectedSlot() == EInventorySlot::Plank)
    {
        FRotator NewRotation = BuildPreviewMesh->GetComponentRotation();
        NewRotation.Yaw += BuildRotationStep;
        BuildPreviewMesh->SetWorldRotation(NewRotation);
    }
}

void ACitizen::AttemptBuild()
{
    if (!InvenComponent) return;

    EInventorySlot CurrentSlot = InvenComponent->GetCurrentSelectedSlot();
    if (CurrentSlot != EInventorySlot::Plank && CurrentSlot != EInventorySlot::Tent) return;

    FVector Location = BuildPreviewMesh->GetComponentLocation();
    FRotator Rotation = BuildPreviewMesh->GetComponentRotation();

    if (IsValidBuildLocation(Location))
    {
        if (UseItem(CurrentSlot))
        {
            // TODO: 실제 건설 구현
        }
    }
}

void ACitizen::AddItem(EInventorySlot Slot, int32 Amount)
{
    if (InvenComponent)
    {
        InvenComponent->UpdateItemCount(Slot, Amount);
    }
}

bool ACitizen::UseItem(EInventorySlot Slot, int32 Amount)
{
    if (InvenComponent)
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count >= Amount)
        {
            ItemData->Count -= Amount;
            InvenComponent->UpdateItemCount(Slot, -Amount);
            return true;
        }
    }
    return false;
}

bool ACitizen::IsValidBuildLocation(const FVector& Location) const
{
    // TODO: BuildableZone 체크 구현
    return true;
}


void ACitizen::Interact()
{
    // 레이캐스트 시작점과 끝점 체크
    FVector Start = CameraComponent->GetComponentLocation();
    FVector Forward = CameraComponent->GetForwardVector();
    FVector End = Start + (Forward * InteractionRange);

    // 디버그 라인 그리기
    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Red,
        false,  // persistent lines
        5.0f,   // lifetime
        0,      // depth priority
        2.0f    // thickness
    );

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        // 히트 발생 확인
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName()));

        if (AItem* Item = Cast<AItem>(HitResult.GetActor()))
        {
            // 아이템 캐스팅 성공 확인
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
                TEXT("Found Item - Adding to Inventory"));

            // 아이템 획득
            AddItem(Item->ItemType, Item->Amount);
            Item->Destroy();

            // 아이템 제거 확인
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White,
                TEXT("Item Destroyed"));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                TEXT("Hit Actor is not an Item"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("No Hit Result"));
    }
}