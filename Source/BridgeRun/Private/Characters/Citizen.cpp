// Private/Characters/Citizen.cpp
#include "Characters/Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Item.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Item/Item_Telescope.h"  
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "Modes/InvenComponent.h"
#include "Modes/PlayerModeComponent.h"
#include "Modes/BuildingComponent.h"
#include "Item/Item_Trophy.h"
#include "Modes/CombatComponent.h"
#include "Item/Item_Plank.h"
#include "GameFramework/PlayerStart.h"
#include "Item/Item_Tent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Core/BridgeRunGameState.h"
#include "Core/BridgeRunPlayerState.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    InvenComponent = CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));
    PlayerModeComponent = CreateDefaultSubobject<UPlayerModeComponent>(TEXT("PlayerModeComponent"));
    BuildingComponent = CreateDefaultSubobject<UBuildingComponent>(TEXT("BuildingComponent"));
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void ACitizen::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // 팀 머티리얼 초기화
        if (GetController() && GetController()->PlayerState)
        {
            ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(GetController()->PlayerState);
            if (BridgeRunPS)
            {
                int32 CurrentTeamID = BridgeRunPS->GetTeamID();
                if (CurrentTeamID >= 0)
                {
                    SetTeamMaterial(CurrentTeamID);
                }
            }
        }

        // 초기 망원경 지급
        FItemData* TelescopeData = InvenComponent ? InvenComponent->GetItemData(EInventorySlot::Telescope) : nullptr;
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }

    // UI 위젯 생성 (로컬 플레이어만)
    if (IsLocallyControlled() && InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }

    // 플레이어 모드 변경 이벤트 바인딩
    if (PlayerModeComponent)
    {
        PlayerModeComponent->OnPlayerModeChanged.AddDynamic(this, &ACitizen::OnPlayerModeChanged);
    }
}

void ACitizen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACitizen, TeamID);
    DOREPLIFETIME(ACitizen, HeldTrophy);
    DOREPLIFETIME(ACitizen, bIsDead);
}

void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACitizen::OnRep_IsDead()
{
    if (bIsDead)
    {
        // 죽었을 때 처리
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        // 들고 있는 트로피 드롭
        if (HeldTrophy)
        {
            HeldTrophy->Drop();
            HeldTrophy = nullptr;
        }
    }
    else
    {
        // 리스폰 됐을 때 처리
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void ACitizen::MulticastHandleDeath_Implementation()
{
    bIsDead = true;

    // 움직임 정지
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    // 입력 비활성화
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
}

void ACitizen::MulticastHandleRespawn_Implementation()
{
    bIsDead = false;

    // 이동 컴포넌트 초기화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    // 입력 재활성화 - 로컬 플레이어만 처리
    if (IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            EnableInput(PC);
        }
    }
}


void ACitizen::ServerRespawn_Implementation(const FVector& RespawnLocation)
{
    if (!HasAuthority()) return;

    // 위치 리셋
    SetActorLocation(RespawnLocation);
    bIsDead = false;

    // 이동 컴포넌트 초기화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->GravityScale = 1.0f;
    }

    // 모드 초기화
    if (PlayerModeComponent)
    {
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
    }

    // 건설 모드 비활성화
    if (BuildingComponent)
    {
        BuildingComponent->DeactivateBuildMode();
    }

    // 전투 아이템 초기화
    if (CombatComponent)
    {
        // 접근자 메서드 사용
        if (CombatComponent->GetEquippedGun())
        {
            CombatComponent->OnGunUnequipped();
        }
        if (CombatComponent->GetEquippedTelescope())
        {
            CombatComponent->OnTelescopeUnequipped();
        }
    }

    // 인벤토리 초기화
    if (InvenComponent)
    {
        // 소모품 제거
        InvenComponent->UpdateItemCount(EInventorySlot::Plank, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Tent, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Gun, 0);

        // 망원경 재지급
        FItemData* TelescopeData = InvenComponent->GetItemData(EInventorySlot::Telescope);
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }

    // 들고 있던 트로피 드롭
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
    }

    // 입력 재활성화 (서버측)
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        EnableInput(PC);
    }

    // 클라이언트에 리스폰 처리 알림
    MulticastHandleRespawn();
}


void ACitizen::ServerSelectInventorySlot_Implementation(EInventorySlot Slot)
{
    if (!InvenComponent || !PlayerModeComponent) return;

    // 항상 이전 장비 해제
    if (CombatComponent)
    {
        if (CombatComponent->GetEquippedTelescope())
        {
            CombatComponent->OnTelescopeUnequipped();
        }
        if (CombatComponent->GetEquippedGun())
        {
            CombatComponent->OnGunUnequipped();
        }
    }

    // 같은 슬롯 다시 선택 시 해제 처리
    if (InvenComponent->GetCurrentSelectedSlot() == Slot)
    {
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        return;
    }

    // 아이템 소유 여부 확인
    FItemData* ItemData = InvenComponent->GetItemData(Slot);
    if (!ItemData || ItemData->Count <= 0)
    {
        // 아이템이 없으면 슬롯을 해제하고 일반 모드로 전환
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        return;
    }

    // 새 슬롯 선택
    InvenComponent->SetCurrentSelectedSlot(Slot);

    // 슬롯 유형에 따른 처리
    switch (Slot)
    {
    case EInventorySlot::Plank:
    case EInventorySlot::Tent:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Build);
        // 클라이언트 모드 변경 처리
        if (!HasAuthority() && IsLocallyControlled())
        {
            OnPlayerModeChanged(EPlayerMode::Build, PlayerModeComponent->GetCurrentMode());
            if (BuildingComponent)
            {
                BuildingComponent->OnBuildModeEntered();
            }
        }
        break;

    case EInventorySlot::Gun:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Combat);
        if (CombatComponent && CombatComponent->GetGunClass())
        {
            // 총 생성 및 장착
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            AItem_Gun* NewGun = GetWorld()->SpawnActor<AItem_Gun>(CombatComponent->GetGunClass(),
                GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
            if (NewGun)
            {
                CombatComponent->OnGunEquipped(NewGun);
            }
        }
        break;

    case EInventorySlot::Telescope:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Combat);
        if (CombatComponent && CombatComponent->GetTelescopeClass())
        {
            // 망원경 생성 및 장착
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            AItem_Telescope* NewTelescope = GetWorld()->SpawnActor<AItem_Telescope>(
                CombatComponent->GetTelescopeClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
            if (NewTelescope)
            {
                CombatComponent->OnTelescopeEquipped(NewTelescope);
            }
        }
        break;

    case EInventorySlot::Trophy:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        break;
    }
}


bool ACitizen::ServerRespawn_Validate(const FVector& RespawnLocation)
{
    return true;
}

void ACitizen::OnPlayerModeChanged(EPlayerMode NewMode, EPlayerMode OldMode)
{
    switch (NewMode)
    {
    case EPlayerMode::Build:
        if (BuildingComponent)
        {
            // 서버와 클라이언트 모두에서 처리
            if (HasAuthority() || IsLocallyControlled())
            {
                BuildingComponent->OnBuildModeEntered();
            }
        }
        break;

    case EPlayerMode::Combat:
        if (CombatComponent)
        {
            CombatComponent->OnCombatModeEntered();
        }
        break;

    case EPlayerMode::Normal:
        if (BuildingComponent)
        {
            BuildingComponent->DeactivateBuildMode();
        }
        if (CombatComponent)
        {
            CombatComponent->OnCombatModeExited();
        }
        GetCharacterMovement()->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
        break;
    }
}

void ACitizen::SelectInventorySlot(EInventorySlot Slot)
{
    // 로컬에서 먼저 처리
    if (IsLocallyControlled())
    {
        FItemData* ItemData = InvenComponent->GetItemData(Slot);
        if (ItemData && ItemData->Count > 0)
        {
            if (Slot == EInventorySlot::Plank || Slot == EInventorySlot::Tent)
            {
                OnPlayerModeChanged(EPlayerMode::Build, PlayerModeComponent->GetCurrentMode());
            }
        }
    }

    // 서버에 전달
    if (HasAuthority())
    {
        ServerSelectInventorySlot_Implementation(Slot);
    }
    else
    {
        ServerSelectInventorySlot(Slot);
    }
}

bool ACitizen::ServerSelectInventorySlot_Validate(EInventorySlot Slot)
{
    return true;
}


void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 이동 입력
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // 점프
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);

    // 인벤토리 슬롯
    PlayerInputComponent->BindAction("Slot1", IE_Pressed, this, &ACitizen::SelectSlot1);
    PlayerInputComponent->BindAction("Slot2", IE_Pressed, this, &ACitizen::SelectSlot2);
    PlayerInputComponent->BindAction("Slot3", IE_Pressed, this, &ACitizen::SelectSlot3);
    PlayerInputComponent->BindAction("Slot4", IE_Pressed, this, &ACitizen::SelectSlot4);
    PlayerInputComponent->BindAction("Slot5", IE_Pressed, this, &ACitizen::SelectSlot5);

    // 상호작용
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);

    // 건설 관련 입력
    if (BuildingComponent)
    {
        PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, BuildingComponent, &UBuildingComponent::RotateBuildPreview);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, BuildingComponent, &UBuildingComponent::AttemptBuild);
    }

    // 전투 관련 입력
    if (CombatComponent)
    {
        PlayerInputComponent->BindAction("Zoom", IE_Pressed, CombatComponent, &UCombatComponent::HandleAim);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, CombatComponent, &UCombatComponent::HandleShoot);
        PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, CombatComponent, &UCombatComponent::DropCurrentWeapon);
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
    AddControllerYawInput(Value);
}

void ACitizen::LookUp(float Value)
{
    AddControllerPitchInput(Value);
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
            switch (Slot)
            {
            case EInventorySlot::Plank:
            case EInventorySlot::Tent:
                InvenComponent->UpdateItemCount(Slot, -Amount);
                if (ItemData->Count == 0 && BuildingComponent)
                {
                    BuildingComponent->DeactivateBuildMode();
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;

            case EInventorySlot::Telescope:
            case EInventorySlot::Gun:
                break;

            case EInventorySlot::Trophy:
                InvenComponent->UpdateItemCount(Slot, -Amount);
                if (ItemData->Count == 0)
                {
                    InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
                }
                break;
            }
            return true;
        }
    }
    return false;
}

void ACitizen::Interact()
{
    if (HasAuthority())
    {
        ServerInteract_Implementation();
    }
    else
    {
        ServerInteract();
    }
}

bool ACitizen::ServerInteract_Validate()
{
    return true;
}

void ACitizen::ServerInteract_Implementation()
{
    // 트로피 처리
    if (HandleHeldTrophyInteraction())
        return;

    // 주변 아이템과 상호작용
    InteractWithNearbyItems();
}

bool ACitizen::HandleHeldTrophyInteraction()
{
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
        return true;
    }
    return false;
}

void ACitizen::InteractWithNearbyItems()
{
    TArray<AActor*> OverlappedActors;

    // 상호작용 범위 시각화
    VisualizeInteractionRadius();

    // 가장 가까운 아이템 찾기
    AActor* ClosestItem = FindClosestInteractableItem(OverlappedActors);

    if (ClosestItem)
    {
        // 아이템 상호작용 처리
        ProcessItemInteraction(ClosestItem);
    }
}

void ACitizen::VisualizeInteractionRadius()
{
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        InteractionRange,
        32,
        FColor::Green,
        false,
        0.1f,
        0,
        1.0f
    );
}

AActor* ACitizen::FindClosestInteractableItem(TArray<AActor*>& OutOverlappedActors)
{
    AActor* ClosestItem = nullptr;
    float ClosestDistance = InteractionRange;

    if (UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        InteractionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AItem::StaticClass(),
        TArray<AActor*>(),
        OutOverlappedActors))
    {
        for (AActor* Actor : OutOverlappedActors)
        {
            float Distance = FVector::Distance(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                VisualizeItemConnection(Actor, false);
                ClosestDistance = Distance;
                ClosestItem = Actor;
            }
        }

        if (ClosestItem)
        {
            VisualizeItemConnection(ClosestItem, true);
        }
    }

    return ClosestItem;
}

void ACitizen::VisualizeItemConnection(AActor* Item, bool IsClosest)
{
    DrawDebugLine(
        GetWorld(),
        GetActorLocation(),
        Item->GetActorLocation(),
        IsClosest ? FColor::Red : FColor::Yellow,
        false,
        0.1f,
        0,
        IsClosest ? 2.0f : 1.0f
    );
}

void ACitizen::ProcessItemInteraction(AActor* Item)
{
    if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(Item))
    {
        Trophy->PickUp(this);
        HeldTrophy = Trophy;
    }
    else if (AItem_Gun* Gun = Cast<AItem_Gun>(Item))
    {
        FItemData* GunData = InvenComponent->GetItemData(EInventorySlot::Gun);
        if (!GunData || GunData->Count == 0)
        {
            AddItem(EInventorySlot::Gun, 1);
            Gun->Destroy();
        }
    }
    else if (AItem* GenericItem = Cast<AItem>(Item))
    {
        AddItem(GenericItem->ItemType, GenericItem->Amount);
        GenericItem->Destroy();
    }
}

void ACitizen::OnRep_TeamID()
{
    SetTeamMaterial(TeamID);
}

void ACitizen::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (GetPlayerState())
    {
        ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(GetPlayerState());
        if (BridgeRunPS)
        {
            int32 CurrentTeamID = BridgeRunPS->GetTeamID();
            if (CurrentTeamID >= 0)
            {
                TeamID = CurrentTeamID;
                SetTeamMaterial(CurrentTeamID);
            }
        }
    }
}

void ACitizen::MulticastSetTeamMaterial_Implementation(int32 InTeamID)
{
    SetTeamMaterial(InTeamID);
}

void ACitizen::SetTeamMaterial(int32 InTeamID)
{
    USkeletalMeshComponent* MeshComponent = GetMesh();
    if (!MeshComponent) return;

    // 팀 ID에 맞는 머티리얼 선택
    UMaterialInterface* TeamMaterial = nullptr;
    switch (InTeamID)
    {
    case 0: TeamMaterial = M_Team_Red; break;
    case 1: TeamMaterial = M_Team_Blue; break;
    case 2: TeamMaterial = M_Team_Yellow; break;
    case 3: TeamMaterial = M_Team_Green; break;
    default: return;
    }

    if (!TeamMaterial) return;

    // 머티리얼 인스턴스 생성 및 적용
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(TeamMaterial, this);
    if (DynamicMaterial)
    {
        // 슬롯 0에 적용
        MeshComponent->SetMaterial(0, DynamicMaterial);

        // 슬롯 1에도 적용
        if (MeshComponent->GetNumMaterials() > 1)
        {
            UMaterialInstanceDynamic* DynamicMaterial2 = UMaterialInstanceDynamic::Create(TeamMaterial, this);
            MeshComponent->SetMaterial(1, DynamicMaterial2);
        }
    }
}