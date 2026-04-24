// Fill out your copyright notice in the Description page of Project Settings.


#include "APIPlanet.h"

#include "Components/SphereComponent.h"

// Sets default values
AAPIPlanet::AAPIPlanet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates components
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Collider = CreateDefaultSubobject<USphereComponent>("Collider");

	// Attaches components
	Collider->SetupAttachment(RootComponent);
	Mesh->SetupAttachment(Collider);

}

// Called when the game starts or when spawned
void AAPIPlanet::BeginPlay()
{
	Super::BeginPlay();

	// Calulates base off mass and distance from sun
	OrbitSpeed = ((GetActorScale().X*250) * FVector::Distance(GetActorLocation(),FVector(0,0,0)) / 5000000);


	// Calculates planet gravity based off size
	Gravity *= (GetActorScale().X/125)*100;

	// Intitialises variables
	DeltaSpinRotation = FRotator::ZeroRotator;
	DeltaOrbitLocation = FVector::Zero();
	LastPosition = GetActorLocation();

	if(RandomStartOrbit) SetRandomOrbit();
	
	AtmosphereRadius = 2500 * FMath::Pow(GetActorScale().X, 0.5);
	AtmosphereRadiusExit = AtmosphereRadius*1.1;
	AtmosphereRadiusMax = AtmosphereRadius*2.5;
	
	if(!HasAtmosphere)
	{
		AtmosphereRadius =0;
		AtmosphereRadiusMax = 0;
	}
}

// Called every frame
void AAPIPlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Spin(DeltaTime);
	Orbit(DeltaTime);

}

// Spins the planet on its own axis
void AAPIPlanet::Spin(float DeltaTime)
{
	// Calculates the rotation around the planet of the frame
	// Uses 360 / SpinSpeed to ensure that the planet spins 360 degrees in the time of SpinSpeed
	DeltaSpinRotation = FRotator(0,360 / SpinSpeed * DeltaTime,0);
	
	AddActorLocalRotation(DeltaSpinRotation);
	
}

// Spins planet around sun (positioned at 0,0,0)
void AAPIPlanet::Orbit(float DeltaTime)
{
	// Calculates the rotation around the sun of the frame
	// Uses 360 / OrbitSpeed to ensure that the planet spins 360 degrees in the time of OrbitSpeed
	const FRotator DeltaOrbitRotation = FRotator(0, 360 / OrbitSpeed * DeltaTime, 0);

	// Calculates Radian (angle wrapped around circle)
	const float DeltaOrbitRadian = FMath::DegreesToRadians(DeltaOrbitRotation.Yaw);
	
	// Rotate the relative location around the Z-axis  by the radian
	const FVector RotatedRelativeLocation = SetOrbit(DeltaOrbitRadian);

	SetActorLocation(RotatedRelativeLocation);

	// Difference in position from last frame
	DeltaOrbitLocation = GetActorLocation() - LastPosition;
	LastPosition = GetActorLocation();

}

// Places planet at random point of path
void AAPIPlanet::SetRandomOrbit()
{
	SetActorLocation(SetOrbit(FMath::RandRange(0,360)));
}

// Returns new position, rotated around Z axis by Rotation float (angle)
FVector AAPIPlanet::SetOrbit(float Rotation) const
{
	return FQuat(FVector(0, 0, 1), Rotation) * GetActorLocation();
}


FRotator AAPIPlanet::GetDeltaSpinRotation() const
{
	return DeltaSpinRotation;
}

FVector AAPIPlanet::GetDeltaOrbitLocation() const
{
	return DeltaOrbitLocation;
}

float AAPIPlanet::GetAtmosphereRadius() const
{
	return AtmosphereRadius;
}

float AAPIPlanet::GetAtmosphereRadiusExit() const
{
	return AtmosphereRadiusExit;
}

float AAPIPlanet::GetAtmosphereRadiusMax() const
{
	return AtmosphereRadiusMax;
}

float AAPIPlanet::GetGravity() const
{
	return Gravity;
}

FString AAPIPlanet::GetPlanetName() const
{
	return PlanetName;
}

bool AAPIPlanet::HasPlanetAtmosphere() const
{
	return HasAtmosphere;
}




