// Fill out your copyright notice in the Description page of Project Settings.


#include "APIBoostMovement.h"
#include "APIPlayer.h"
#include "Components/ShapeComponent.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UAPIBoostMovement::UAPIBoostMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UAPIBoostMovement::BeginPlay()
{
	Super::BeginPlay();

	// Gets player from component
	Player = Cast<AAPIPlayer>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(Player->GetController());

	// Enhanced input binding (this is all stupid)
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	Subsystem->AddMappingContext(MappingContext, 0);
	UEnhancedInputComponent* PawnInputComp = Cast<UEnhancedInputComponent>(Player->InputComponent);
	PawnInputComp->BindAction(StartBoostInput, ETriggerEvent::Triggered, this, &UAPIBoostMovement::StartBoost);
	PawnInputComp->BindAction(StopBoostInput, ETriggerEvent::Triggered, this, &UAPIBoostMovement::StopBoost);

	// Initialises for game
	BoostState = BoostNone;
	CurrentBoostPower = Player->GetActorForwardVector();
	
	
}

void UAPIBoostMovement::PostLoad()
{
	Super::PostLoad();

}

void UAPIBoostMovement::StartBoost(const FInputActionValue& Value)
{
	BoostState = BoostStart;
	BoostDuration = 0.0f;
	StartBoostPower = CurrentBoostPower;
	Player->ActivateBoostParticle(true);
}

void UAPIBoostMovement::StopBoost(const FInputActionValue& Value)
{
	BoostState = BoostStop;
	BoostDuration = 0.0f;
	StartBoostPower = CurrentBoostPower;

	Player->ActivateBoostParticle(false);

}


// Called every frame
void UAPIBoostMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Initialises for frame
	FVector TargetBoost = FVector::Zero();
	float BoostTime = 0;
	
	// Acceleration
	if(BoostState == BoostStart)
	{
		BoostDuration += DeltaTime;
		
		TargetBoost = Player->GetActorForwardVector() * BoostPower * 100 *
			// If in space apply additional boost to hover
			(FMath::Lerp(1,10.0f,Player->GetPlanetRadiusDistance()));

		BoostTime = BoostDuration / BoostAccelerate;

	}
	// Deceleration
	if(BoostState == BoostStop)
	{
		BoostDuration += DeltaTime;
		
		TargetBoost = FVector::Zero();

		BoostTime = BoostDuration / BoostDecelerate;

	}
	
	CurrentBoostPower = FMath::Lerp(StartBoostPower, TargetBoost, FMath::Clamp(BoostTime,0,1));

	// State to indicate no boost motion is being applied
	if(BoostState == BoostStop && BoostTime <= 0)
		BoostState = BoostNone;

	// Applies boost
	if(Player != nullptr && BoostState != BoostNone && CurrentBoostPower != FVector::Zero())
		Player->AddExternalHorVelocity(CurrentBoostPower);
	
}


