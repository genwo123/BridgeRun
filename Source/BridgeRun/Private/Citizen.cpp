// Fill out your copyright notice in the Description page of Project Settings.
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;
    // ĳ���� ��ü�� ��Ʈ�ѷ� ȸ���� ������ �ʵ��� ����
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    // ĳ���Ͱ� �̵� �������� ȸ������ �ʵ���
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // �������� ����
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    // ī�޶� ����
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ACitizen::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ACitizen::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACitizen::Turn(float Value)
{
    if (Value != 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Turn Value: %f"), Value);
        AddControllerYawInput(Value);

        // ������� ���� ���� ȸ���� ���
        FRotator CurrentRotation = GetControlRotation();
        UE_LOG(LogTemp, Warning, TEXT("Current Rotation: %s"), *CurrentRotation.ToString());
    }
}

void ACitizen::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("LookUp Value: %f"), Value);
        AddControllerPitchInput(Value);

        // ������� ���� ���� ȸ���� ���
        FRotator CurrentRotation = GetControlRotation();
        UE_LOG(LogTemp, Warning, TEXT("Current Rotation: %s"), *CurrentRotation.ToString());
    }
}

// SetupPlayerInputComponent ����

void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // ������� ���� �α�
    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Called"));

    // �̵�
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);

    // ���콺 �Է¿� ���� ���ε� ���� �α�
    UE_LOG(LogTemp, Warning, TEXT("About to bind mouse input"));

    // ���콺 ȸ��
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // ����
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);
}

void ACitizen::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // ī�޶� ���� ������ �������� ��/�� �̵�
        const FVector Direction = GetActorForwardVector();
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // ī�޶� ���� ������ �������� ��/�� �̵�
        const FVector Direction = GetActorRightVector();
        AddMovementInput(Direction, Value);
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

