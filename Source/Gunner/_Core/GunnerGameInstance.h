// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GunnerGameInstance.generated.h"



/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	void OnSeamlessTravelStart(UWorld* World, const FString& MapName);
	void OnPreLoadMap(const FString& String);
	void PostLoadMapWithWorld(UWorld* InLoadedWorld);
	void PlayLoadingScreen();
	void StopLoadingScreen();

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreenWidget;
};
