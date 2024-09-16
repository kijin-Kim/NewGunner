// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerAction.h"
#include "TestGunnerAction.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UTestGunnerAction : public UGunnerAction
{
	GENERATED_BODY()
public:
	UTestGunnerAction();
	virtual void TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InActionAgentInfo) override;
	virtual void EndAction() override;

private:
	inline static int32 Count = 0;
};
