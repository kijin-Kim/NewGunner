// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerStateMachineTransitionCondition.generated.h"



/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerStateMachineTransitionCondition : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnEnter();
	UFUNCTION(BlueprintNativeEvent)
	void OnUpdate(float DeltaTime);
	UFUNCTION(BlueprintNativeEvent)
	void OnExit();
	
	bool ShouldTransit() const;

	UFUNCTION(BlueprintPure)
	AActor* GetOwner() const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldTransit = false;
	
};
