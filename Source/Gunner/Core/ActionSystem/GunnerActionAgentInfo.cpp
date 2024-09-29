// Fill out your copyright notice in the Description page of Project Settings.

#include "GunnerActionAgentInfo.h"

void FGunnerActionAgentInfo::Init(AActor* InOwnerActor, AActor* InAgentActor)
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

bool FGunnerActionAgentInfo::operator==(const FGunnerActionAgentInfo& Other) const
{
	return Controller == Other.Controller && AgentActor == Other.AgentActor && OwnerActor == Other.OwnerActor;
}

bool FGunnerActionAgentInfo::operator!=(const FGunnerActionAgentInfo& Other) const
{
	return !(*this == Other);
}

bool FGunnerActionAgentInfo::IsLocallyPlayerControlled() const
{
	return Controller.IsValid() && Controller->IsLocalPlayerController();
}

bool FGunnerActionAgentInfo::IsLocallyControlled() const
{
	return Controller.IsValid() && Controller->IsLocalController();
}

bool FGunnerActionAgentInfo::IsOwnerActorAuthoritative() const
{
	return OwnerActor.IsValid() && OwnerActor->GetLocalRole() == ROLE_Authority;
}
