// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InputActionValue.h"
#include "APIGamemodeBase.generated.h"

class AAPIPlanet;
class UAPIWidget;
class AAPIPlayer;

class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class APIPROJECT_API AAPIGamemodeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAPIGamemodeBase();
	
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditAnywhere, Category="Widgets")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category="Widgets")
	UInputAction* StartGameInput;

	
	UPROPERTY(EditAnywhere, Category="Widgets")
	TSubclassOf<class UUserWidget> GameWidgetClass;

	UPROPERTY()
	UAPIWidget* GameWidget;

	UFUNCTION()
	void ChangePlanetText();

	UFUNCTION()
	void NewPlanet();


private:
	UFUNCTION()
	void StartGame(const FInputActionValue& Value);
	void ResetGame();
	void FinishGame();
	
	void GenerateArray();
	void ShuffleArray(TArray<AActor*>& Array);
	void NextPlanet();
	
	TObjectPtr<AAPIPlayer> Player;
	
	TObjectPtr<AAPIPlanet> CurrentPlanet;
	int CurrentPlanetIndex;

	UPROPERTY()
	TArray<AActor*> PlanetActors;
	TArray<AAPIPlanet*> Planets;
	TArray<FString> PlanetNames;

	float GameTimer = 0.0f;

	bool GameActive;
	bool CanStartGame = true;

	FTimerHandle RestartGameTimer;

};

