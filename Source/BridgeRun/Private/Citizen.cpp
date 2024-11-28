// Citizen.cpp
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "InvenComponent.h"
#include "PlayerModeComponent.h"
#include "BuildingComponent.h"
#include "CombatComponent.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 회전 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터 이동 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // 컴포넌트 생성
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    // 기능별 컴포넌트 생성
    InvenComponent = CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));
    PlayerModeComponent = CreateDefaultSubobject<UPlayerModeComponent>(TEXT("PlayerModeComponent"));
    BuildingComponent = CreateDefaultSubobject<UBuildingComponent>(TEXT("BuildingComponent"));
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void ACitizen::BeginPlay()
{
    Super::BeginPlay();

    // UI 생성
    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }

    // 모드 변경 이벤트 바인딩
    if (PlayerModeComponent)
    {
        PlayerModeComponent->OnPlayerModeChanged.AddDynamic(this, &ACitizen::OnPlayerModeChanged);
    }
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACitizen::OnPlayerModeChanged(EPlayerMode NewMode, EPlayerMode OldMode)
{
    // 이전 모드와 새로운 모드 로그 출력
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("Mode Changed: %s -> %s"),
            *UEnum::GetValueAsString(OldMode),
            *UEnum::GetValueAsString(NewMode)));



    switch (NewMode)
    {
    case EPlayerMode::Build:
        GetCharacterMovement()->bOrientRotationToMovement = false;
        if (BuildingComponent)
        {
            BuildingComponent->OnBuildModeEntered();
        }
        break;

    case EPlayerMode::Combat:
        GetCharacterMovement()->bOrientRotationToMovement = true;
        if (CombatComponent)
        {
            CombatComponent->OnCombatModeEntered();
        }
        break;

    case EPlayerMode::Normal:
        GetCharacterMovement()->bOrientRotationToMovement = true;
        if (BuildingComponent)
        {
            BuildingComponent->DeactivateBuildMode();
        }
        break;
    }
}

void ACitizen::SelectInventorySlot(EInventorySlot Slot)
{
    if (!InvenComponent || !PlayerModeComponent) return;

    FItemData* ItemData = InvenComponent->GetItemData(Slot);
    if (!ItemData || ItemData->Count <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            FString::Printf(TEXT("Cannot select slot: No items available in slot %d"), static_cast<int32>(Slot)));
        return;
    }

    if (InvenComponent->GetCurrentSelectedSlot() == Slot)
    {
        // 같은 슬롯 다시 선택 시 해제
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange,
            TEXT("Deselected current slot - Returning to Normal mode"));
    }
    else
    {
        InvenComponent->SetCurrentSelectedSlot(Slot);

        // 슬롯에 따른 모드 설정
        switch (Slot)
        {
        case EInventorySlot::Plank:
        case EInventorySlot::Tent:
            PlayerModeComponent->SetPlayerMode(EPlayerMode::Build);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                FString::Printf(TEXT("Selected Building Item: %s"),
                    Slot == EInventorySlot::Plank ? TEXT("Plank") : TEXT("Tent")));
            break;

        case EInventorySlot::Gun:
            PlayerModeComponent->SetPlayerMode(EPlayerMode::Combat);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                TEXT("Selected Combat Item: Gun"));
            break;

        case EInventorySlot::Telescope:
        case EInventorySlot::Trophy:
            PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
                FString::Printf(TEXT("Selected Normal Item: %s"),
                    Slot == EInventorySlot::Telescope ? TEXT("Telescope") : TEXT("Trophy")));
            break;
        }
    }
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

    // 상호작용
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);

    // 건설 관련 입력은 BuildingComponent에서 처리하도록 변경
    if (BuildingComponent)
    {
        PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, BuildingComponent, &UBuildingComponent::RotateBuildPreview);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, BuildingComponent, &UBuildingComponent::AttemptBuild);  // Build를 PrimaryAction으로 변경
    }
}

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
            // 아이템 타입에 따른 처리
            switch (Slot)
            {
            case EInventorySlot::Plank:
            case EInventorySlot::Tent:
                // 소모성 아이템
                ItemData->Count -= Amount;
                InvenComponent->UpdateItemCount(Slot, -Amount);
                // 갯수가 0이 되면 모드 해제
                if (ItemData->Count == 0 && BuildingComponent)
                {
                    BuildingComponent->DeactivateBuildMode();
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;

            case EInventorySlot::Telescope:
            case EInventorySlot::Gun:
                // 비소모성 아이템은 갯수 변경 없음
                break;

            case EInventorySlot::Trophy:
                // 트로피는 별도 처리
                ItemData->Count -= Amount;
                InvenComponent->UpdateItemCount(Slot, -Amount);
                if (ItemData->Count == 0)
                {
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;

            default:
                break;
            }
            return true;
        }
    }
    return false;
}

void ACitizen::Interact()
{
    // 레이캐스트 시작점과 끝점 체크
    FVector Start = CameraComponent->GetComponentLocation();
    FVector Forward = CameraComponent->GetForwardVector();
    FVector End = Start + (Forward * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (AItem* Item = Cast<AItem>(HitResult.GetActor()))
        {
            // 아이템 획득
            AddItem(Item->ItemType, Item->Amount);
            Item->Destroy();
        }
    }
}