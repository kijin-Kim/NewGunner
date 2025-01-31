// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTeamAgentInterface.h"

UGunnerTeamAgentInterface::UGunnerTeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ETeamAttitude::Type IGunnerTeamAgentInterface::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGunnerTeamAgentInterface* OtherTeamAgent = Cast<IGunnerTeamAgentInterface>(&Other);
	if (OtherTeamAgent == nullptr)
	{
		return ETeamAttitude::Neutral;
	}

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId() ? ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}
