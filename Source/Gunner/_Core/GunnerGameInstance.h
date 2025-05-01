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
    	UCurveTable* GetDamageCurveTable() const;
    	UDataTable* GetWeaponDataTable() const;
private:
	void OnSeamlessTravelStart(UWorld* World, const FString& MapName);
	void PostLoadMapWithWorld(UWorld* InLoadedWorld);
	void OnShowDebugInfo(AHUD* Ahud, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg);
	void PlayLoadingScreen();
	void StopLoadingScreen();
	void OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController);
	
	
	

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveTable> DamageCurveTable;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> WeaponDataTable;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	
};
