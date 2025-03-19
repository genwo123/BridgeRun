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
        // �� ��Ƽ���� �ʱ�ȭ
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

        // �ʱ� ������ ����
        FItemData* TelescopeData = InvenComponent ? InvenComponent->GetItemData(EInventorySlot::Telescope) : nullptr;
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }

    // UI ���� ���� (���� �÷��̾)
    if (IsLocallyControlled() && InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }

    // �÷��̾� ��� ���� �̺�Ʈ ���ε�
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
        // �׾��� �� ó��
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        // ��� �ִ� Ʈ���� ���
        if (HeldTrophy)
        {
            HeldTrophy->Drop();
            HeldTrophy = nullptr;
        }
    }
    else
    {
        // ������ ���� �� ó��
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void ACitizen::MulticastHandleDeath_Implementation()
{
    bIsDead = true;

    // ������ ����
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    // �Է� ��Ȱ��ȭ
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
}

void ACitizen::MulticastHandleRespawn_Implementation()
{
    bIsDead = false;

    // �̵� ������Ʈ �ʱ�ȭ
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    // �Է� ��Ȱ��ȭ - ���� �÷��̾ ó��
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

    // ��ġ ����
    SetActorLocation(RespawnLocation);
    bIsDead = false;

    // �̵� ������Ʈ �ʱ�ȭ
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->GravityScale = 1.0f;
    }

    // ��� �ʱ�ȭ
    if (PlayerModeComponent)
    {
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
    }

    // �Ǽ� ��� ��Ȱ��ȭ
    if (BuildingComponent)
    {
        BuildingComponent->DeactivateBuildMode();
    }

    // ���� ������ �ʱ�ȭ
    if (CombatComponent)
    {
        // ������ �޼��� ���
        if (CombatComponent->GetEquippedGun())
        {
            CombatComponent->OnGunUnequipped();
        }
        if (CombatComponent->GetEquippedTelescope())
        {
            CombatComponent->OnTelescopeUnequipped();
        }
    }

    // �κ��丮 �ʱ�ȭ
    if (InvenComponent)
    {
        // �Ҹ�ǰ ����
        InvenComponent->UpdateItemCount(EInventorySlot::Plank, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Tent, 0);
        InvenComponent->UpdateItemCount(EInventorySlot::Gun, 0);

        // ������ ������
        FItemData* TelescopeData = InvenComponent->GetItemData(EInventorySlot::Telescope);
        if (TelescopeData && TelescopeData->Count == 0)
        {
            AddItem(EInventorySlot::Telescope);
        }
    }

    // ��� �ִ� Ʈ���� ���
    if (HeldTrophy)
    {
        HeldTrophy->Drop();
        HeldTrophy = nullptr;
    }

    // �Է� ��Ȱ��ȭ (������)
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        EnableInput(PC);
    }

    // Ŭ���̾�Ʈ�� ������ ó�� �˸�
    MulticastHandleRespawn();
}


void ACitizen::ServerSelectInventorySlot_Implementation(EInventorySlot Slot)
{
    if (!InvenComponent || !PlayerModeComponent) return;

    // �׻� ���� ��� ����
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

    // ���� ���� �ٽ� ���� �� ���� ó��
    if (InvenComponent->GetCurrentSelectedSlot() == Slot)
    {
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        return;
    }

    // ������ ���� ���� Ȯ��
    FItemData* ItemData = InvenComponent->GetItemData(Slot);
    if (!ItemData || ItemData->Count <= 0)
    {
        // �������� ������ ������ �����ϰ� �Ϲ� ���� ��ȯ
        InvenComponent->SetCurrentSelectedSlot(EInventorySlot::None);
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Normal);
        return;
    }

    // �� ���� ����
    InvenComponent->SetCurrentSelectedSlot(Slot);

    // ���� ������ ���� ó��
    switch (Slot)
    {
    case EInventorySlot::Plank:
    case EInventorySlot::Tent:
        PlayerModeComponent->SetPlayerMode(EPlayerMode::Build);
        // Ŭ���̾�Ʈ ��� ���� ó��
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
            // �� ���� �� ����
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
            // ������ ���� �� ����
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
            // ������ Ŭ���̾�Ʈ ��ο��� ó��
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
    // ���ÿ��� ���� ó��
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

    // ������ ����
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

    // �̵� �Է�
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // ����
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);

    // �κ��丮 ����
    PlayerInputComponent->BindAction("Slot1", IE_Pressed, this, &ACitizen::SelectSlot1);
    PlayerInputComponent->BindAction("Slot2", IE_Pressed, this, &ACitizen::SelectSlot2);
    PlayerInputComponent->BindAction("Slot3", IE_Pressed, this, &ACitizen::SelectSlot3);
    PlayerInputComponent->BindAction("Slot4", IE_Pressed, this, &ACitizen::SelectSlot4);
    PlayerInputComponent->BindAction("Slot5", IE_Pressed, this, &ACitizen::SelectSlot5);

    // ��ȣ�ۿ�
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACitizen::Interact);

    // �Ǽ� ���� �Է�
    if (BuildingComponent)
    {
        PlayerInputComponent->BindAction("RotateBuild", IE_Pressed, BuildingComponent, &UBuildingComponent::RotateBuildPreview);
        PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, BuildingComponent, &UBuildingComponent::AttemptBuild);
    }

    // ���� ���� �Է�
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
    // Ʈ���� ó��
    if (HandleHeldTrophyInteraction())
        return;

    // �ֺ� �����۰� ��ȣ�ۿ�
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

    // ��ȣ�ۿ� ���� �ð�ȭ
    VisualizeInteractionRadius();

    // ���� ����� ������ ã��
    AActor* ClosestItem = FindClosestInteractableItem(OverlappedActors);

    if (ClosestItem)
    {
        // ������ ��ȣ�ۿ� ó��
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

    // �� ID�� �´� ��Ƽ���� ����
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

    // ��Ƽ���� �ν��Ͻ� ���� �� ����
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(TeamMaterial, this);
    if (DynamicMaterial)
    {
        // ���� 0�� ����
        MeshComponent->SetMaterial(0, DynamicMaterial);

        // ���� 1���� ����
        if (MeshComponent->GetNumMaterials() > 1)
        {
            UMaterialInstanceDynamic* DynamicMaterial2 = UMaterialInstanceDynamic::Create(TeamMaterial, this);
            MeshComponent->SetMaterial(1, DynamicMaterial2);
        }
    }
}