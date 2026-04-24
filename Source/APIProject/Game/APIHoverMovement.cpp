// Fill out your copyright notice in the Description page of Project Settings.


#include "APIHoverMovement.h"
#include "APIPlayer.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UAPIHoverMovement::UAPIHoverMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAPIHoverMovement::BeginPlay()
{
	Super::BeginPlay();

	// Gets player from component
	Player = Cast<AAPIPlayer>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(Player->GetController());

	// Enhanced input binding 
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	Subsystem->AddMappingContext(MappingContext, 0);
	UEnhancedInputComponent* PawnInputComp = Cast<UEnhancedInputComponent>(Player->InputComponent);
	PawnInputComp->BindAction(StartHoverInput, ETriggerEvent::Triggered, this, &UAPIHoverMovement::StartHover);
	PawnInputComp->BindAction(StopHoverInput, ETriggerEvent::Triggered, this, &UAPIHoverMovement::StopHover);

	// Scales the values so you can use low values in editor
	HoverPower *= 10000;
	MaxHoverPower *= 10000;
	
}


// Called every frame
void UAPIHoverMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If the player has landed stop hovering
	if(IsHovering && Player->GetOnGround())
		StopHover(true);
	
	// If holding space while in air
	
	if(IsHovering)// && Player->GetPlanetState())
	{
		Hover(DeltaTime);
	}
	else
	{
		// Decreases the hover speed over time
		CurrentHoverPower -= DeltaTime;

		CurrentHoverSpeedMultiplier = 1;
	}
}

void UAPIHoverMovement::Hover(float DeltaTime)
{
	// Increases the hover speed and ensures the hover velocity cant go past the max
	CurrentHoverPower = FMath::Clamp(CurrentHoverPower+=HoverPower, 0.0f, MaxHoverPower);

	// Gradually reduce the intial boost
	HoverPowerInitialBoost = FMath::FInterpTo(HoverPowerInitialBoost, 1, DeltaTime, 50);

	const float AddVelocity = CurrentHoverPower * HoverPowerInitialBoost * DeltaTime;
	// Adds hover velocity
	Player->AddExternalVerVelocity(AddVelocity);

	// Increases the movespeed while hovering
	Player->ChangeMoveSpeed(HoverSpeedMultiplier);
}

void UAPIHoverMovement::StartHover(const FInputActionValue& Value)
{
	
	if(Player->GetOnGround()) return;

	IsHovering = true;

	const float ZVelocity = Player->GetVerticalVelocity();
	// If the player is currently falling, give a little boost based on how quickly they are falling (the ZVelocity)
	HoverPowerInitialBoost = ZVelocity < 0 ?
		1 + -ZVelocity / 750 :
		1;

	Player->ActivateHoverParticle(true);

}

void UAPIHoverMovement::StopHover(const FInputActionValue& Value)
{
	IsHovering = false;

	Player->ActivateHoverParticle(false);
}


