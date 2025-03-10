// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Object.h"
#include "NexusCue.generated.h"




/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class NEXUSACTION_API UNexusCue : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	void CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);

protected:
	virtual void OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnTriggered"))
	void BP_OnTriggered();

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FNexusTargetDataHandle TargetDataHandle;
};
