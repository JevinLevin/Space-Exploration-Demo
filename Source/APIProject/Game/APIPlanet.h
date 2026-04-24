// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APIPlanet.generated.h"

class USphereComponent;
class UNiagaraComponent;


UCLASS()
class APIPROJECT_API AAPIPlanet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAPIPlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<USphereComponent> Collider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlanetName;

// How many seconds it will take the planet to complete a full spin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpinSpeed = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Gravity = -75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasAtmosphere = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RandomStartOrbit = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FRotator GetDeltaSpinRotation() const;
	FVector GetDeltaOrbitLocation() const;
	float GetAtmosphereRadius() const;
	float GetAtmosphereRadiusExit() const;
	float GetAtmosphereRadiusMax() const;
	float GetGravity() const;
	FString GetPlanetName() const;
	bool HasPlanetAtmosphere() const;
 
private:

	void Spin(float DeltaTime);
	float SpinDuration;
	FRotator DeltaSpinRotation;

	void Orbit(float DeltaTime);
	void SetRandomOrbit();
	FVector SetOrbit(float Rotation) const;
	FVector LastPosition;
	FVector DeltaOrbitLocation;
	// Calcuated based on mass and distance from sun
	float OrbitSpeed;

	float AtmosphereRadius;
	float AtmosphereRadiusExit;
	float AtmosphereRadiusMax;

	
};
