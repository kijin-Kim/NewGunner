// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "Gunner/Core/ActionSystem/GunnerAction.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"
#include "GunnerCancellableAsyncAction.generated.h"

class UGunnerActionComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerCancellableAsyncAction : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	template <typename T>
	static T* NewGunnerAsync(UGunnerAction* InAction)
	{
		check(InAction);
		T* SelfObject = NewObject<T>();
		SelfObject->Action = InAction;
		SelfObject->ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(InAction->GetOwnerActor());
		check(SelfObject->ActionComponent.IsValid());
		SelfObject->Action->OnGunnerActionEndedDelegate.AddWeakLambda(SelfObject, [SelfObject](FGunnerActionDefinitionHandle, UGunnerAction*)
		{
			SelfObject->Cancel();
			SelfObject->Action->OnGunnerActionEndedDelegate.RemoveAll(SelfObject);
		});
		return SelfObject;
	}

	virtual bool ShouldBroadcastDelegates() const override;
	virtual void Cancel() override;

protected:
	TWeakObjectPtr<UGunnerAction> Action;
	TWeakObjectPtr<UGunnerActionComponent> ActionComponent;
};
