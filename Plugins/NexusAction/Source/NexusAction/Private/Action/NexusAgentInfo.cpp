// Fill out your copyright notice in the Description page of Project Settings.

#include "Action/NexusAgentInfo.h"

void FNexusAgentInfo::Init(AActor* InOwnerActor, AActor* InAgentActor)
{
	OwnerActor = InOwnerActor;
	AgentActor = InAgentActor;

	AActor* ActorTemp = InOwnerActor;
	while (ActorTemp)
	{
		if (APawn* PawnOwner = Cast<APawn>(ActorTemp))
		{
			Controller = PawnOwner->GetController();
			break;
		}

		if (AController* C = Cast<AController>(ActorTemp))
		{
			Controller = C;
			break;
		}

		ActorTemp = ActorTemp->GetOwner();
	}

	if (!GetController())
	{
		if (APawn* AgentPawn = Cast<APawn>(AgentActor))
		{
			Controller = AgentPawn->GetController();
		}
	}
}

bool FNexusAgentInfo::IsLocallyPlayerControlled() const
{
	return GetController() ? GetController()->IsLocalPlayerController() : false;
}

bool FNexusAgentInfo::IsLocallyControlled() const
{
	return GetController() ? GetController()->IsLocalController() : false;
}

bool FNexusAgentInfo::IsOwnerActorAuthoritative() const
{
	return GetOwnerActor() && GetOwnerActor()->HasAuthority();
}

AController* FNexusAgentInfo::GetController() const
{
	if (Controller.Get() && !Controller.Get()->IsPendingKillPending())
	{
		return Controller.Get();
	}
	return nullptr;
}

AActor* FNexusAgentInfo::GetAgentActor() const
{
	if (AgentActor.Get() && !AgentActor.Get()->IsPendingKillPending())
	{
		return AgentActor.Get();
	}
	return nullptr;
}

AActor* FNexusAgentInfo::GetOwnerActor() const
{
	if (OwnerActor.Get() && !OwnerActor.Get()->IsPendingKillPending())
	{
		return OwnerActor.Get();
	}
	return nullptr;
}
