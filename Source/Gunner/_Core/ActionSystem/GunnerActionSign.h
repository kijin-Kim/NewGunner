// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "GunnerActionSign.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GUNNER_API UGunnerActionSign : public UObject
{
	GENERATED_BODY()
	
public:
	virtual UWorld* GetWorld() const override;
	UFUNCTION(BlueprintNativeEvent)
	void OnSignaled();
	void SetSignalDataObject(UObject* InSignalDataObject) { SignalDataObject = InSignalDataObject; }
	UFUNCTION(BlueprintCallable)
	UObject* GetSignalDataObject() const { return SignalDataObject.Get(); }

	
private:
	TWeakObjectPtr<UObject> SignalDataObject;
};
