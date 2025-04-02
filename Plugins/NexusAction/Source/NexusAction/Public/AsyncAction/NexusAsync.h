// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "Action/NexusAction.h"
#include "NexusActionComponent.h"
#include "NexusAsync.generated.h"

class UNexusActionComponent;
/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusAsync : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	template <typename T>
	static T* NewNexusAsync(UNexusAction* InAction)
	{
		check(InAction);
		T* SelfObject = NewObject<T>();
		SelfObject->Action = InAction;
		SelfObject->ActionComponent = UNexusActionComponent::GetActionComponentFromActor(InAction->GetOwnerActor());
		check(SelfObject->ActionComponent.IsValid());
		SelfObject->Action->OnActionEndedDelegate.AddWeakLambda(SelfObject, [SelfObject](FNexusActionDefHandle, UNexusAction*)
		{
			SelfObject->Cancel();
			SelfObject->Action->OnActionEndedDelegate.RemoveAll(SelfObject);
		});
		return SelfObject;
	}

	virtual bool ShouldBroadcastDelegates() const override;


protected:
	TWeakObjectPtr<UNexusAction> Action;
	TWeakObjectPtr<UNexusActionComponent> ActionComponent;
};
