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
	void OnSeamlessTravelStart(UWorld* World, const FString& MapName);
	void PostLoadMapWithWorld(UWorld* InLoadedWorld);
	void PlayLoadingScreen();
	void StopLoadingScreen();
	
	
	virtual void Shutdown() override; 
	UCurveTable* GetDamageCurveTable() const;
	UDataTable* GetWeaponDataTable() const;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveTable> DamageCurveTable;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> WeaponDataTable;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	
	
};
