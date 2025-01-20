// Public/Characters/BridgeRunCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BridgeRunCharacter.generated.h"

UCLASS(config = Game)
class ABridgeRunCharacter : public ACharacter
{
    GENERATED_BODY()

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

public:
    ABridgeRunCharacter();

    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float BaseTurnRate;

    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float BaseLookUpRate;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Resets HMD orientation in VR. */
    void OnResetVR();

    /** Called for forwards/backward input */
    void MoveForward(float Value);

    /** Called for side to side input */
    void MoveRight(float Value);

    /** Called via input to turn at a given rate. */
    void TurnAtRate(float Rate);

    /** Called via input to turn look up/down at a given rate. */
    void LookUpAtRate(float Rate);

    /** Handler for when a touch input begins. */
    void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

    /** Handler for when a touch input stops. */
    void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};