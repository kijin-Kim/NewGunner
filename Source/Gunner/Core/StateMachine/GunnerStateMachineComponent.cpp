// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerStateMachineComponent.h"

#include "GunnerStateComponent.h"
#include "GunnerStateMachineTransitionCondition.h"
#include "ParticleHelper.h"


// Sets default values for this component's properties
UGunnerStateMachineComponent::UGunnerStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunnerStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterStateComponentFromOwner();
	RegisterTransitionData();
}


void UGunnerStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (CurrentStateComponent)
	{
		CurrentStateComponent->OnUpdate(DeltaTime);
	}

	CheckTransitions();
}


void UGunnerStateMachineComponent::RegisterStateComponentFromOwner()
{
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	ActorOwner->GetComponents<UGunnerStateComponent>(OwnerStateComponents);
	if (!ensure(OwnerStateComponents.Num() > 0))
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


	for (const auto& [FromStateClass, ToStateAndConditionClasses] : TransitionData->TransitionData)
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


		for (const auto& [ConditionClass, ToStateClass] : ToStateAndConditionClasses)
		{
			if (!ToStateClass || !ConditionClass)
			{
				continue;
			}

			if (UGunnerStateComponent* ToStateComponent = GetStateComponentByClass(ToStateClass))
			{
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
		if (!ToState || !Condition)
		{
			continue;
		}

		if (Condition->ShouldTransit())
		{
			CurrentStateComponent->OnExit();
			CurrentStateComponent = ToState;
			CurrentStateComponent->OnEnter();
			break;
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
