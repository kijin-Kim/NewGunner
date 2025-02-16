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
}

bool FNexusAgentInfo::IsLocallyPlayerControlled() const
{
	return Controller.IsValid() && Controller->IsLocalPlayerController();
}

bool FNexusAgentInfo::IsLocallyControlled() const
{
	return Controller.IsValid() && Controller->IsLocalController();
}

bool FNexusAgentInfo::IsOwnerActorAuthoritative() const
{
	return OwnerActor.IsValid() && OwnerActor->GetLocalRole() == ROLE_Authority;
}
