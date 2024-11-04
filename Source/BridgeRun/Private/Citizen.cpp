// Fill out your copyright notice in the Description page of Project Settings.
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;
    // 캐릭터 자체는 컨트롤러 회전을 따르지 않도록 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    // 캐릭터가 이동 방향으로 회전하지 않도록
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // 스프링암 생성
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    // 카메라 생성
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

        // 디버깅을 위해 현재 회전값 출력
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

        // 디버깅을 위해 현재 회전값 출력
        FRotator CurrentRotation = GetControlRotation();
        UE_LOG(LogTemp, Warning, TEXT("Current Rotation: %s"), *CurrentRotation.ToString());
    }
}

// SetupPlayerInputComponent 수정

void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 디버깅을 위한 로그
    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Called"));

    // 이동
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);

    // 마우스 입력에 대한 바인딩 직전 로그
    UE_LOG(LogTemp, Warning, TEXT("About to bind mouse input"));

    // 마우스 회전
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // 점프
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);
}

void ACitizen::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // 카메라가 보는 방향을 기준으로 앞/뒤 이동
        const FVector Direction = GetActorForwardVector();
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // 카메라가 보는 방향을 기준으로 좌/우 이동
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

