// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "APIBoostMovement.generated.h"

#define PRINT(x, ...) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT(x), ##__VA_ARGS__), false)




class AAPIPlayer;
class UInputAction;
class UInputMappingContext;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APIPROJECT_API UAPIBoostMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAPIBoostMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<AAPIPlayer> Player;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* StartBoostInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* StopBoostInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostPower = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostAccelerate = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostDecelerate = 1.0f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostLoad() override;

private:

	void StartBoost(const FInputActionValue& Value);
	void StopBoost(const FInputActionValue& Value);

	enum BoostStates{ BoostNone, BoostStart, BoostStop};
	BoostStates BoostState = BoostNone;

	
	FVector CurrentBoostPower;
	FVector StartBoostPower;
	float BoostDuration;
	bool OnPlanet;


		
};
