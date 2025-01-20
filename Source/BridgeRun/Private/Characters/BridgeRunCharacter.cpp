// Private/Characters/BridgeRunCharacter.cpp
#include "Characters/BridgeRunCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

ABridgeRunCharacter::ABridgeRunCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // set our turn rates for input
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    // Network setup
    bReplicates = true;

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Create a camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ABridgeRunCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABridgeRunCharacter, BaseTurnRate);
    DOREPLIFETIME(ABridgeRunCharacter, BaseLookUpRate);
}

void ABridgeRunCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Set up gameplay key bindings
    check(PlayerInputComponent);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAxis("MoveForward", this, &ABridgeRunCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABridgeRunCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &ABridgeRunCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ABridgeRunCharacter::LookUpAtRate);

    PlayerInputComponent->BindTouch(IE_Pressed, this, &ABridgeRunCharacter::TouchStarted);
    PlayerInputComponent->BindTouch(IE_Released, this, &ABridgeRunCharacter::TouchStopped);

    PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABridgeRunCharacter::OnResetVR);
}

void ABridgeRunCharacter::OnResetVR()
{
    UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ABridgeRunCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
    Jump();
}

void ABridgeRunCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
    StopJumping();
}

void ABridgeRunCharacter::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABridgeRunCharacter::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABridgeRunCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ABridgeRunCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}