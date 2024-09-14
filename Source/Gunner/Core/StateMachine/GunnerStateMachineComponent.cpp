// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerStateMachineComponent.h"

#include "GunnerStateComponent.h"
#include "GunnerStateMachineTransitionCondition.h"
#include "ParticleHelper.h"
#include "Gunner/Gunner.h"


UGunnerStateMachineComponent::UGunnerStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGunnerStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() != ROLE_SimulatedProxy)
	{
		RegisterStateComponentFromOwner();
		RegisterTransitionData();
		LocalEnterNewState(CurrentStateComponent);
	}
}

void UGunnerStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Update(CurrentStateComponent, DeltaTime);
		CheckTransitions();
	}
}

void UGunnerStateMachineComponent::RegisterStateComponentFromOwner()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	ActorOwner->GetComponents<UGunnerStateComponent>(OwnerStateComponents);
	if (!ensureMsgf(OwnerStateComponents.Num() > 0, TEXT("No StateComponent is found")))
	{
		return;
	}

	CurrentStateComponent = GetStateComponentByClass(StartStateComponentClass);
	CurrentStateComponent = CurrentStateComponent ? CurrentStateComponent : OwnerStateComponents[0];
}

void UGunnerStateMachineComponent::RegisterTransitionData()
{
	if (!TransitionData)
	{
		return;
	}


	for (const auto& [FromStateClass, ToStateAndConditionClasses] : TransitionData->TransitionDataEntries)
	{
		if (!FromStateClass)
		{
			continue;
		}

		UGunnerStateComponent* FromStateComponent = GetStateComponentByClass(FromStateClass);
		if (!FromStateComponent)
		{
			continue;
		}


		for (const auto& [ToStateClass, ConditionClass] : ToStateAndConditionClasses)
		{
			if (!ensure(ToStateClass && ConditionClass))
			{
				continue;
			}

			UGunnerStateComponent* ToStateComponent = GetStateComponentByClass(ToStateClass);
			if (ensureMsgf(ToStateComponent, TEXT("%s ToStateComponent is not found"), *ToStateClass->GetName()))
			{
				AActor* owner = GetOwner();
				StateTransitionMap.FindOrAdd(FromStateComponent).Add({
					ToStateComponent,
					NewObject<UGunnerStateMachineTransitionCondition>(GetOwner(), ConditionClass)
				});
			}
		}
	}
}

void UGunnerStateMachineComponent::CheckTransitions()
{
	if (!CurrentStateComponent || !StateTransitionMap.Contains(CurrentStateComponent))
	{
		return;
	}


	for (const auto& [ToState, Condition] : StateTransitionMap[CurrentStateComponent])
	{
		if (!ToState || !Condition->IsValidLowLevel())
		{
			continue;
		}

		if (GetNetMode() == NM_Standalone && Condition->ShouldTransit())
		{
			LocalEnterNewState(ToState);
			break;
		}
		
		const EGunnerStateNetTransitionPolicy NetTransitionPolicy = ToState->GetNetTransitionPolicy();
		const ENetRole OwnerRole = GetOwnerRole();

		if (OwnerRole == ROLE_AutonomousProxy && Condition->ShouldTransit())
		{
			if (NetTransitionPolicy == EGunnerStateNetTransitionPolicy::ClientOnly)
			{
				LocalEnterNewState(ToState);
			}
			else if (NetTransitionPolicy == EGunnerStateNetTransitionPolicy::ClientPredicted)
			{
				LocalEnterNewState(ToState);
				ServerEnterNewState(ToState);
			}
			break;
		}

		if (OwnerRole == ROLE_Authority && Condition->ShouldTransit())
		{
			if (NetTransitionPolicy == EGunnerStateNetTransitionPolicy::ServerOnly)
			{
				LocalEnterNewState(ToState);
			}
			else if (NetTransitionPolicy == EGunnerStateNetTransitionPolicy::ServerAuthoritative)
			{
				LocalEnterNewState(ToState);
				ClientEnterNewState(ToState);
			}
			break;
		}
	}
}

bool UGunnerStateMachineComponent::CanTransitionToState(UGunnerStateComponent* InToStateComponent) const
{
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
	bool bFoundTest = false;
	for (const auto& [ToState, Condition] : StateTransitionMap[CurrentStateComponent])
	{
		if (!ToState || !Condition->IsValidLowLevel())
		{
			continue;
		}

		if (ToState == InToStateComponent)
		{
			bFoundTest = true;
		}

		if (ToState == InToStateComponent && Condition->ShouldTransit())
		{
			return true;
		}
	}
	check(bFoundTest);
	return false;
}


void UGunnerStateMachineComponent::LocalEnterNewState(UGunnerStateComponent* ToStateComponent)
{
	if (ToStateComponent)
	{
		Exit(CurrentStateComponent);
		CurrentStateComponent = ToStateComponent;
		Enter(CurrentStateComponent);
	}
}

void UGunnerStateMachineComponent::ServerEnterNewState_Implementation(UGunnerStateComponent* ToStateComponent)
{
	if (CanTransitionToState(ToStateComponent))
	{
		LocalEnterNewState(ToStateComponent);
	}
}

void UGunnerStateMachineComponent::ClientEnterNewState_Implementation(UGunnerStateComponent* ToStateComponent)
{
	LocalEnterNewState(ToStateComponent);
}

void UGunnerStateMachineComponent::Enter(UGunnerStateComponent* StateComponent)
{
	if (StateComponent)
	{
		StateComponent->OnEnter();

		if (!StateTransitionMap.Contains(StateComponent))
		{
			return;
		}

		for (const auto& [ToState, Condition] : StateTransitionMap[StateComponent])
		{
			if (!ToState || !Condition->IsValidLowLevel())
			{
				continue;
			}

			Condition->OnEnter();
		}
	}
}

void UGunnerStateMachineComponent::Update(UGunnerStateComponent* StateComponent, float DeltaTime)
{
	if (StateComponent)
	{
		StateComponent->OnUpdate(DeltaTime);

		if (!StateTransitionMap.Contains(StateComponent))
		{
			return;
		}

		for (const auto& [ToState, Condition] : StateTransitionMap[StateComponent])
		{
			if (!ToState || !Condition->IsValidLowLevel())
			{
				continue;
			}
			Condition->OnUpdate(DeltaTime);
		}
	}
}

void UGunnerStateMachineComponent::Exit(UGunnerStateComponent* StateComponent)
{
	if (StateComponent)
	{
		StateComponent->OnExit();

		if (!StateTransitionMap.Contains(StateComponent))
		{
			return;
		}

		for (const auto& [ToState, Condition] : StateTransitionMap[StateComponent])
		{
			if (!ToState || !Condition->IsValidLowLevel())
			{
				continue;
			}

			Condition->OnExit();
		}
	}
}

UGunnerStateComponent* UGunnerStateMachineComponent::GetStateComponentByClass(TSubclassOf<UGunnerStateComponent> StateComponentClass) const
{
	const TObjectPtr<UGunnerStateComponent>* StateComponentPtr = OwnerStateComponents.FindByPredicate([StateComponentClass](const UGunnerStateComponent* StateComponent)
	{
		return StateComponent->GetClass() == StateComponentClass;
	});
	return StateComponentPtr ? *StateComponentPtr : nullptr;
}
