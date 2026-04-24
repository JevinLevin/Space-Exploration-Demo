// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "APIWidget.generated.h"

/**
 * 
 */
UCLASS()
class APIPROJECT_API UAPIWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void DisplayPlanetName(const FString& PlanetName);

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void DisplayPlanetList(const TArray<FString>& PlanetNames);

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void UpdatePlanetList(const int& Index);

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void UpdateTimer(const float& Time);

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void FinishGame(const float& FinalTime);

	UFUNCTION(BlueprintImplementableEvent,  Category = "Widget")
	void StartGame();
};
