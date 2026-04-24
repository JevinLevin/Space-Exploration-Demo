// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"

#include "APIPlayer.generated.h"

#define PRINT(x, ...) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT(x), ##__VA_ARGS__), false)

class UNiagaraComponent;
class UAPIHoverMovement;
class UAPIBoostMovement;
class UCapsuleComponent;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AAPIPlanet;

class UInputAction;
class UInputMappingContext;

UCLASS()
class APIPROJECT_API AAPIPlayer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAPIPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveForwardInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveRightInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PitchCameraInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* YawCameraInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpInput;


	// Define Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UCapsuleComponent> Collider;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> BoostParticle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> HoverParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UAPIBoostMovement> BoostMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UAPIHoverMovement> HoverMovement;

	// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sensitivity = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinCameraPitch = -50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCameraPitch = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpBufferMax = 0.15f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float Gravity = -30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpPower = 25.0f;
	


public:
	virtual void Tick(float DeltaTime) override;
	


	TObjectPtr<UShapeComponent> GetCollider() const;

	bool GetPlanetState() const;
	float GetPlanetRadiusDistance() const;
	FString GetPlanetName() const;

	void AddExternalHorVelocity(const FVector Velocity);
	void AddExternalVerVelocity(const float Velocity);

	bool GetOnGround() const;

	void ChangeMoveSpeed(float Multiplier);

	float GetVerticalVelocity() const;
	
	void ActivateBoostParticle(bool Value);
	void ActivateHoverParticle(bool Value);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlanetEnter);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlanetEnter OnPlanetEnterEvent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlanetLand);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlanetLand OnPlanetLandEvent;


private:
	// Functions
	// Inputs
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void PitchCamera(const FInputActionValue& Value);
	void YawCamera(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	
	void Jump();
	

	void MovePlayerHor(const FVector& Direction) const;
	void MovePlayerVer(float DeltaTime);
	void RotateCamera();
	void RotatePlayer(float DeltaTime);

	void RotateAroundPlanet();
	void OrbitWithPlanet();

	void RemoveNoAtmosphere();
	
	void LocatePlanet();
	bool CheckingPlanet = true;
	FTimerHandle GravitySwitchTimer;
	UFUNCTION()
	void StartCheckingGravityPivot();



	bool CheckGround() const;
	bool CheckingGround = true;
	FTimerHandle CheckGroundTimer;
	UFUNCTION()
	void StartCheckingGround();

	bool ShouldApplyGravity() const;


	FVector CalculateGravityDirection();
	FVector GetInputDirection() const;

	// Stores inputs throughout frame
	FVector MovementInput;
	FVector RotationInput;

	// Planetary gravity
	FVector GravityPivot;
	FVector GravityDirection;
	float ZVelocity;
	
	bool OnGround;
	bool OnPlanet;
	float Gravity = -5000;
	float JumpBuffer;
	float DefaultSpringArmRotation;
	float PlanetRadiusDistance=1;

	float CurrentMoveSpeed;

	FVector ExternalHorVelocity;
	float ExternalVerVelocity;

	
	// All planets in scene
	UPROPERTY()
	TArray<AActor*> Planets;
	UPROPERTY()
	AAPIPlanet* CurrentPlanet;

	
};
