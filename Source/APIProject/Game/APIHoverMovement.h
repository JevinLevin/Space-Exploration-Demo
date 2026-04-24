// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "APIHoverMovement.generated.h"

#define PRINT(x, ...) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT(x), ##__VA_ARGS__), false)

class AAPIPlayer;
class UInputAction;
class UInputMappingContext;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APIPROJECT_API UAPIHoverMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAPIHoverMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<AAPIPlayer> Player;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* StartHoverInput;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* StopHoverInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoverPower = 0.075f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHoverPower = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoverSpeedMultiplier = 1.5f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void StartHover(const FInputActionValue& Value);
	void StopHover(const FInputActionValue& Value);
	void Hover(float DeltaTime);

	bool IsHovering;
	
	float CurrentHoverPower;
	float PreviousHoverPower;
	float CurrentHoverSpeedMultiplier;
	float HoverPowerInitialBoost;

		
};
