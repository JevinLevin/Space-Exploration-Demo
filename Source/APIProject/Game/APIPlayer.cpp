// Fill out your copyright notice in the Description page of Project Settings.


#include "APIPlayer.h"

#include "APIBoostMovement.h"
#include "APIHoverMovement.h"
#include "APIPlanet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values
AAPIPlayer::AAPIPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Creates components
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Collider = CreateDefaultSubobject<UCapsuleComponent>("Collider");
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	BoostParticle = CreateDefaultSubobject<UNiagaraComponent>("Boost Particle");
	HoverParticle = CreateDefaultSubobject<UNiagaraComponent>("Hover Particle");


	BoostMovement = CreateDefaultSubobject<UAPIBoostMovement>("Boost Movement");
	HoverMovement = CreateDefaultSubobject<UAPIHoverMovement>("Hover Movement");

	// Attaches components
	Collider->SetupAttachment(RootComponent);
	Mesh->SetupAttachment(Collider);
	SpringArm->SetupAttachment(Mesh);
	Camera->SetupAttachment(SpringArm);
	BoostParticle->SetupAttachment(Mesh);
	HoverParticle->SetupAttachment(Mesh);


	//BoostMovement->SetupAttachment(RootComponent);

}

void AAPIPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Bind inputs
	APlayerController* PC = Cast<APlayerController>(GetController());

	// Enhanced input binding (this is all stupid)
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	Subsystem->AddMappingContext(MappingContext, 0);
	UEnhancedInputComponent* PawnInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	PawnInputComp->BindAction(MoveForwardInput, ETriggerEvent::Triggered, this, &AAPIPlayer::MoveForward);
	PawnInputComp->BindAction(MoveRightInput, ETriggerEvent::Triggered, this, &AAPIPlayer::MoveRight);
	PawnInputComp->BindAction(YawCameraInput, ETriggerEvent::Triggered, this, &AAPIPlayer::YawCamera);
	PawnInputComp->BindAction(PitchCameraInput, ETriggerEvent::Triggered, this, &AAPIPlayer::PitchCamera);
	PawnInputComp->BindAction(JumpInput, ETriggerEvent::Triggered, this, &AAPIPlayer::StartJump);

	// This stops the player from falling if they spawn in the air
	OnGround = true;

	// Scales the values so you can use low values in editor
	MoveSpeed *= 100;
	JumpPower *= 100;
	//Gravity *= 100;

	// Stores original rotation so it can revert back to it easily
	DefaultSpringArmRotation = SpringArm->GetRelativeRotation().Pitch;

	// Stores all planets in array
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAPIPlanet::StaticClass(), Planets);

	// Grabs initial planet and gravity
	LocatePlanet();
	if(CurrentPlanet != nullptr)
		GravityDirection = CalculateGravityDirection();
	else
		GravityDirection = FVector::Zero();


}

void AAPIPlayer::Tick(float DeltaTime)
{
	
	Super::Tick(DeltaTime);
	
	if(OnPlanet)
	{
		RotateAroundPlanet();
		OrbitWithPlanet();
		// If on planet, calculates direction of gravity, rather than just down
		GravityDirection = CalculateGravityDirection();
	}
	else
	{
		GravityDirection = FVector::Zero();
	}

	if(CheckingPlanet) 
		LocatePlanet();

	if(CheckGround() && !OnGround)
	{
		OnGround = true;
		OnPlanetLandEvent.Broadcast();
	}
	if(!CheckGround() && OnGround)
	{
		OnGround = false;
	}


	// Sets rotation around planet
	const FQuat TargetQuat = FQuat::FindBetween(GetActorUpVector(),-GravityDirection) * GetActorRotation().Quaternion();
	SetActorRotation(FQuat::Slerp(GetActorRotation().Quaternion(), TargetQuat, DeltaTime*1.5f));
	
	// Grab direction vector based off input
	FVector Direction = GetInputDirection();

	// If in space, cannot use WASD movement
	if(!OnPlanet)
		Direction = FVector::Zero();
	
	// Horizontal motion
	MovePlayerHor(Direction);
	
	// Jump system based off buffer variable
	if(JumpBuffer > 0)
	{
		if(OnGround)
		{
			Jump();
		}
		JumpBuffer -= DeltaTime;
	}
	


	// Vertical motion
	MovePlayerVer(DeltaTime);

	// Rotates camera based off player input
	if (OnPlanet)
		RotateCamera();
	else
		RotatePlayer(DeltaTime);

	// Stores last planets gravity
	if(CurrentPlanet)Gravity = CurrentPlanet->GetGravity();
	// Default gravity in space for hovering
	if(!CurrentPlanet) Gravity = -5000;
	
	// Applies gravity
	if(ShouldApplyGravity()) ZVelocity += Gravity * DeltaTime;

	// If in space, gravity cant be negative and can only go up
	if(!OnPlanet) ZVelocity = FMath::Max(ZVelocity, 0);

	// Applies constant gravity if on ground to ensure no floating
	if(OnGround) ZVelocity = -300.0f;

	// Resets for next frame
	MovementInput = FVector::Zero();
	RotationInput = FVector::Zero();
	ExternalHorVelocity = FVector::Zero();
	ExternalVerVelocity = 0.0f;
	CurrentMoveSpeed = MoveSpeed;

}

TObjectPtr<UShapeComponent> AAPIPlayer::GetCollider() const
{
	return Collider;
}

// Keeps player in same position of rotating planet
void AAPIPlayer::RotateAroundPlanet()
{
	// Amount rotated this frame
	FQuat DeltaRotation = CurrentPlanet->GetDeltaSpinRotation().Quaternion();

	// Get relative location from the nearest planet
	FVector NewLocation = GetActorLocation() - CurrentPlanet->GetActorLocation();
	// Rotate it the same amount as the planet rotated the last frame
	NewLocation = DeltaRotation.RotateVector(NewLocation);

	SetActorLocation(NewLocation + CurrentPlanet->GetActorLocation());

	// Add planet rotation to player rotation
	SetActorRotation(DeltaRotation * GetActorQuat());
	
}

// Keeps player in same position of orbiting planet
void AAPIPlayer::OrbitWithPlanet()
{	
	// Moves player same distance that closest planet did last frame
	SetActorRelativeLocation( GetActorLocation() + CurrentPlanet->GetDeltaOrbitLocation());

}


// Checks whether gravity should be calculated and applied or not
bool AAPIPlayer::ShouldApplyGravity() const
{
	if(OnGround) return false;

	// If no conditions are met, then gravity should be applied
	return true;
}


// Finds direction between current position and current gravity pivot
FVector AAPIPlayer::CalculateGravityDirection()
{
	GravityPivot = CurrentPlanet->GetActorLocation();
	return UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), GravityPivot);
}

void AAPIPlayer::LocatePlanet()
{
	AAPIPlanet* NewPlanet = CurrentPlanet;

	// Sets to largest float value for comparison
	float ShortestDistance = std::numeric_limits<float>::max();

	// Checks if there are any valid planets found
	bool ValidPlanet = false;
	
	// Loops through all planets
	for(int i = 0; i < Planets.Num(); i++)
	{
		// Casts planet actor now to avoid doing it twice
		AAPIPlanet* TempPlanet = Cast<AAPIPlanet>(Planets[i]);

		// Finds distance between player and planet
		const float NewDistance = FVector::Distance(GetActorLocation(), TempPlanet->GetActorLocation());

		// If the planet is within the max range
		if(NewDistance < TempPlanet->GetAtmosphereRadiusMax())
		{
			// This value determines the distance from the radius to the max radius, from 0-1
			PlanetRadiusDistance = FMath::Max(UKismetMathLibrary::NormalizeToRange(NewDistance, TempPlanet->GetAtmosphereRadius()*0.5f, TempPlanet->GetAtmosphereRadiusMax()),0);
		}
		
		// If the distance is not within the atmosphere size, skip planet
		// If its checking the current planet, then use the exit radius rather than the normal radius
		// The exit radius ensures that the exit radius is further out than the entrance radius, ensuring players won't get stuck in the middle
		if(NewDistance > (TempPlanet != CurrentPlanet ? TempPlanet->GetAtmosphereRadius() : TempPlanet->GetAtmosphereRadiusExit()))
			continue;

		ValidPlanet = true;

		if(!TempPlanet->HasPlanetAtmosphere())
			continue;

		// If the distance is shorter than the other planets
		if(NewDistance < ShortestDistance)
		{
			// Set this distance as the current lowest distance
			ShortestDistance = NewDistance;
			// Change the current planet to this planet
			NewPlanet = TempPlanet;
		}
	}

	// If the player isn't in range of any planets
	if(!ValidPlanet)
	{
		CurrentPlanet = nullptr;
		NewPlanet = nullptr;
		
		OnPlanet = false;

		GravityDirection = -GetActorUpVector();
	}
	if(NewPlanet != CurrentPlanet)
	{
		// If switching planet to planet
		if(OnPlanet)
			ZVelocity = -ZVelocity / 2;
		// If switching from space to planet
		else
			ZVelocity = -10;

		// Delay to stop from switching atmospheres too quickly
		CheckingPlanet = false;
		GetWorld()->GetTimerManager().SetTimer(GravitySwitchTimer, this, &AAPIPlayer::StartCheckingGravityPivot, 1.0f, false);

		CurrentPlanet = NewPlanet;

		OnPlanet = true;

		// Event to signify planet change
		OnPlanetEnterEvent.Broadcast();
	}
}

// Moves player in passed direction
void AAPIPlayer::MovePlayerHor(const FVector& Direction) const
{
	Collider->SetAllPhysicsLinearVelocity( (Direction * CurrentMoveSpeed) + ExternalHorVelocity); 
}

// Returns the current move speed
void AAPIPlayer::ChangeMoveSpeed(float Multiplier)
{
	CurrentMoveSpeed += MoveSpeed * Multiplier;
}


// Applys any vertical velocity and checks jump state
void AAPIPlayer::MovePlayerVer(float DeltaTime)
{

	ZVelocity += ExternalVerVelocity;
	AddActorWorldOffset( GravityDirection*(-(ZVelocity)*DeltaTime), true);
}

void AAPIPlayer::StartCheckingGround()
{
	CheckingGround = true;
}

void AAPIPlayer::StartCheckingGravityPivot()
{
	CheckingPlanet = true;
}

// Returns if the player is currently on the ground or not
bool AAPIPlayer::CheckGround() const
{
	if(!CheckingGround) {return false;}
	
	FHitResult Hit;
	 
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = GetActorLocation() + -GetActorUpVector() * 60.0f;
	 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	 
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	 
	// If the planet it lands on isnt the current planet then something has gone wrong
	// (implemented to specifically stop crashes when colliding with sun)
	if(Hit.GetActor() != CurrentPlanet) return false;
	 
	return Hit.bBlockingHit && IsValid(Hit.GetActor());
}

// Stores players normalised X and Y input
void AAPIPlayer::MoveRight(const FInputActionValue& Value)
{
	MovementInput = MovementInput + (GetActorRightVector() * FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f));

}

void AAPIPlayer::MoveForward(const FInputActionValue& Value)
{
	MovementInput = MovementInput + (GetActorForwardVector() * FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f));
}

// Player camera mouse input
void AAPIPlayer::PitchCamera(const FInputActionValue& Value)
{
	RotationInput.Y = Value.Get<float>() * Sensitivity;
}
void AAPIPlayer::YawCamera(const FInputActionValue& Value)
{
	RotationInput.X = Value.Get<float>() * Sensitivity;
}


// Rotate camera based off player input
void AAPIPlayer::RotateCamera()
{
	// Yaw
	AddActorLocalRotation(FRotator(0.0f, RotationInput.X, 0.0f));
	
	// Pitch
	FRotator NewCameraRotation = SpringArm->GetRelativeRotation();
	NewCameraRotation.Pitch = FMath::Clamp(NewCameraRotation.Pitch + RotationInput.Y, MinCameraPitch, MaxCameraPitch);
	SpringArm->SetRelativeRotation(NewCameraRotation);
}

void AAPIPlayer::RotatePlayer(float DeltaTime)
{
	AddActorLocalRotation(FRotator(RotationInput.Y, RotationInput.X, 0.0f));

	FRotator TargetRotation = SpringArm->GetRelativeRotation();
	TargetRotation.Pitch = DefaultSpringArmRotation;
	SpringArm->SetRelativeRotation(FMath::RInterpTo(SpringArm->GetRelativeRotation(), TargetRotation, DeltaTime, 5));
}

// Sets jump buffer if player uses jump input
void AAPIPlayer::StartJump(const FInputActionValue& Value)
{
	if(OnGround)
	{
		JumpBuffer = JumpBufferMax;
		return;
	}
}

// If jumped and on ground
void AAPIPlayer::Jump()
{
	// Sets vertical velocity to send player up
	ZVelocity = JumpPower;

	// Forces player to press space again to jump
	JumpBuffer = 0;

	OnGround = false;

	CheckingGround = false;
	GetWorld()->GetTimerManager().SetTimer(CheckGroundTimer, this, &AAPIPlayer::StartCheckingGround, 0.25f, false);

}


// Returns direction vector based off input
FVector AAPIPlayer::GetInputDirection() const
{
	FVector Direction = FVector::Zero();
	if(!MovementInput.IsZero())
	{
		Direction = MovementInput.GetSafeNormal();
	}
	return Direction;
}

// Removes planet with no atmosphere from list
void AAPIPlayer::RemoveNoAtmosphere()
{
	for(int i = 0; i < Planets.Num(); i++)
	{
		// Cast planet
		AAPIPlanet* TempPlanet = Cast<AAPIPlanet>(Planets[i]);

		// Removes planet if it has no atmosphere
		if(!TempPlanet->HasPlanetAtmosphere())
		{
			Planets.Remove(TempPlanet);
		}
		
	}
}

bool AAPIPlayer::GetPlanetState() const
{
	return OnPlanet;
}

float AAPIPlayer::GetPlanetRadiusDistance() const
{
	return PlanetRadiusDistance;
}

FString AAPIPlayer::GetPlanetName() const
{
	return CurrentPlanet->GetPlanetName();
}

// Used for other components to add velocity to the player each frame
void AAPIPlayer::AddExternalHorVelocity(const FVector Velocity)
{
	ExternalHorVelocity += Velocity;
}

void AAPIPlayer::AddExternalVerVelocity(const float Velocity)
{
	ExternalVerVelocity += Velocity;
	//ZVelocity += Velocity;
}

bool AAPIPlayer::GetOnGround() const
{
	return OnGround;
}

float AAPIPlayer::GetVerticalVelocity() const
{
	return ZVelocity;
}

void AAPIPlayer::ActivateBoostParticle(bool Value)
{
	if(Value)
		BoostParticle->Activate();
	else
		BoostParticle->Deactivate();
}

void AAPIPlayer::ActivateHoverParticle(bool Value)
{
	if(Value)
		HoverParticle->Activate();
	else
		HoverParticle->Deactivate();
}