// Fill out your copyright notice in the Description page of Project Settings.


#include "APIGamemodeBase.h"

#include "APIPlanet.h"
#include "APIPlayer.h"
#include "APIWidget.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

AAPIGamemodeBase::AAPIGamemodeBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}


void AAPIGamemodeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If game is inactive then no need to run code
	if(!GameActive) return;
	
	GameTimer += DeltaTime;
	GameWidget->UpdateTimer(GameTimer);
}

void AAPIGamemodeBase::BeginPlay()
{
	
	if(GameWidgetClass)
	{
		GameWidget = Cast<UAPIWidget>(CreateWidget(GetWorld(), GameWidgetClass));

		if(GameWidget)
		{
			GameWidget->AddToViewport();

		}
	}

}

void AAPIGamemodeBase::GenerateArray()
{
	PlanetActors.Empty();
	Planets.Empty();
	PlanetNames.Empty();
	
	// Stores all planets in array
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAPIPlanet::StaticClass(), PlanetActors);

	// Randomises the planets array
	ShuffleArray(PlanetActors);

	// Casts list into AAPIPlanets
	for (AActor* Actor : PlanetActors)
	{
		AAPIPlanet* Planet = Cast<AAPIPlanet>(Actor);

		// Removes planet if it has no atmosphere
		if(!Planet->HasPlanetAtmosphere())
		{
			continue;
		}
    
		if (Planet)
		{
			Planets.Add(Planet);
		}
	}

	for(int i = 0; i < Planets.Num(); i++)
	{
		// Store planet
		AAPIPlanet* TempPlanet = Planets[i];

		PlanetNames.Add(TempPlanet->GetPlanetName());
		
	}
	

	GameWidget->DisplayPlanetList(PlanetNames);
	GameWidget->UpdatePlanetList(0);
}

// Algorithm taken from https://forums.unrealengine.com/t/using-c-shuffle/133069/7
void AAPIGamemodeBase::ShuffleArray(TArray<AActor*>& Array)
{
	if (Array.Num() > 0)
	{
		int32 LastIndex = Array.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				Array.Swap(i, Index);
			}
		}
	}
}

void AAPIGamemodeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	Player = Cast<AAPIPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));

	if (Player)
	{
		// Bind events to the delegate
		Player->OnPlanetEnterEvent.AddDynamic(this, &AAPIGamemodeBase::ChangePlanetText);
		Player->OnPlanetLandEvent.AddDynamic(this, &AAPIGamemodeBase::NewPlanet);
	}

	// Gets player controller from component
	APlayerController* PC = Cast<APlayerController>(Player->GetController());

	// Enhanced input binding
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	Subsystem->AddMappingContext(MappingContext, 0);
	UEnhancedInputComponent* PawnInputComp = Cast<UEnhancedInputComponent>(Player->InputComponent);
	PawnInputComp->BindAction(StartGameInput, ETriggerEvent::Triggered, this, &AAPIGamemodeBase::StartGame);
}

void AAPIGamemodeBase::NextPlanet()
{
	CurrentPlanetIndex++;
	if(CurrentPlanetIndex < Planets.Num())
	{
		CurrentPlanet = Planets[CurrentPlanetIndex];
	}
	else
	{
		FinishGame();
	}
	GameWidget->UpdatePlanetList(CurrentPlanetIndex);


}

void AAPIGamemodeBase::ChangePlanetText()
{
	GameWidget->DisplayPlanetName(Player->GetPlanetName());
}

void AAPIGamemodeBase::NewPlanet()
{
	if(!GameActive) return;

	// Checks if the planet is the current planet in the list
	if(Player->GetPlanetName() == CurrentPlanet->GetPlanetName()) NextPlanet();
}

void AAPIGamemodeBase::StartGame(const FInputActionValue& Value)
{
	if(!CanStartGame) return;
	CanStartGame = false;
	
	GenerateArray();
	
	GameActive = true;
	GameTimer = 0.0f;
	CurrentPlanet = Planets[0];
	CurrentPlanetIndex = 0;
	
	GameWidget->StartGame();
}

void AAPIGamemodeBase::ResetGame()
{
	CanStartGame = true;
}

void AAPIGamemodeBase::FinishGame()
{
	GameWidget->FinishGame(GameTimer);

	CurrentPlanet = nullptr;

	GameActive = false;
		
	GetWorld()->GetTimerManager().SetTimer(RestartGameTimer, this, &AAPIGamemodeBase::ResetGame, 10.5f, false);
}

