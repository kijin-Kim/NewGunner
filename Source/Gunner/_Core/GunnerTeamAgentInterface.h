// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/Object.h"
#include "GunnerTeamAgentInterface.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGunnerTeamSetSignature, FGenericTeamId, /*OldTeamID*/ FGenericTeamId /*NewTeamID*/);

/**
 * 
 */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UGunnerTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class GUNNER_API IGunnerTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	 virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() { return nullptr; }
	
};
