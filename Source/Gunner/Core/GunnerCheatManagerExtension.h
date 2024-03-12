// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GunnerCheatManagerExtension.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()
public:
	UGunnerCheatManagerExtension();
	UFUNCTION(Exec)
	void ToggleThirdPersonMode();


private:
	bool bIsTPMode;
	
};
