// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

UCLASS()
class GUNNER_API ASpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnManager();
	virtual void PostInitializeComponents() override;
	void OnPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController);
	

private:
	
};
