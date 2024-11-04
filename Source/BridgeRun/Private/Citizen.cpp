// Fill out your copyright notice in the Description page of Project Settings.
#include "Citizen.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// 생성자 부분 수정
ACitizen::ACitizen()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 회전 설정 수정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터가 이동 방향으로 자연스럽게 회전하도록 설정
    GetCharacterMovement()->bOrientRotationToMovement = true; // 이 값을 true로 변경
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // 회전 속도 설정
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

// 이동 함수 수정
void ACitizen::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // 컨트롤러의 회전값을 가져옵니다.
        const FRotator Rotation = Controller->GetControlRotation();
        // 회전값에서 Pitch(X)와 Roll(Z)은 무시하고 Yaw(Y)만 사용합니다.
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        // 회전값으로부터 전방 벡터를 구합니다.
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        // 구한 방향으로 이동을 적용합니다.
        AddMovementInput(Direction, Value);
    }
}

void ACitizen::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // 컨트롤러의 회전값을 가져옵니다.
        const FRotator Rotation = Controller->GetControlRotation();
        // 회전값에서 Pitch(X)와 Roll(Z)은 무시하고 Yaw(Y)만 사용합니다.
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        // 회전값으로부터 우측 벡터를 구합니다.
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        // 구한 방향으로 이동을 적용합니다.
        AddMovementInput(Direction, Value);
    }
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
        //UE_LOG(LogTemp, Warning, TEXT("Turn Value: %f"), Value);
        AddControllerYawInput(Value);

        // 디버깅을 위해 현재 회전값 출력
        //FRotator CurrentRotation = GetControlRotation();
       // UE_LOG(LogTemp, Warning, TEXT("Current Rotation: %s"), *CurrentRotation.ToString());
    }
}

void ACitizen::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        //UE_LOG(LogTemp, Warning, TEXT("LookUp Value: %f"), Value);
        AddControllerPitchInput(Value);

        // 디버깅을 위해 현재 회전값 출력
        // FRotator CurrentRotation = GetControlRotation();
        //UE_LOG(LogTemp, Warning, TEXT("Current Rotation: %s"), *CurrentRotation.ToString());
    }
}

// SetupPlayerInputComponent 수정

void ACitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 디버깅을 위한 로그
    //UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Called"));

    // 이동
    PlayerInputComponent->BindAxis("MoveForward", this, &ACitizen::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACitizen::MoveRight);

    // 마우스 입력에 대한 바인딩 직전 로그
    //UE_LOG(LogTemp, Warning, TEXT("About to bind mouse input"));

    // 마우스 회전
    PlayerInputComponent->BindAxis("Turn", this, &ACitizen::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACitizen::LookUp);

    // 점프
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACitizen::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACitizen::StopJump);
}

void ACitizen::StartJump()
{
    bPressedJump = true;
}

void ACitizen::StopJump()
{
    bPressedJump = false;
}

