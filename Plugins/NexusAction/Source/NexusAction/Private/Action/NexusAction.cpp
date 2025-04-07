// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusAction.h"


UWorld* UNexusAction::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusAction::InitializeAction(FNexusActionDefHandle InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo)
{
	check(InAgentInfo.IsValid());
	ActionDefHandle = InActionDefHandle;
	AgentInfo = InAgentInfo;
}

void UNexusAction::SetActionCurrentEventMessage(const FNexusEventMessage& InEventMessage)
{
	EventMessage = InEventMessage;
}

void UNexusAction::CallOnActionAdded()
{
	OnActionAdded();
	BP_OnActionAdded();
}

bool UNexusAction::CallOnCanTriggerAction() const
{
	bool bResult = OnCanTriggerAction();

	UFunction* Function = FindFunction(FName(TEXT("BP_OnCanTriggerAction")));
	if (Function && Function->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass()))
	{
		bResult &= BP_OnCanTriggerAction();
	}

	return bResult;
}

void UNexusAction::CallOnTriggerAction()
{
	OnTriggerAction();
	BP_OnTriggerAction();
}

void UNexusAction::CallOnEndAction()
{
	if (bIsTriggering)
	{
		bIsTriggering = false;
		OnEndAction();
		BP_OnEndAction();
	}
}

void UNexusAction::CallOnActionRemoved()
{
	OnActionRemoved();
	BP_OnActionRemoved();
}

void UNexusAction::OnActionAdded()
{
}

bool UNexusAction::OnCanTriggerAction() const
{
	return bIsRetriggerable ? true : !bIsTriggering;
}

void UNexusAction::OnTriggerAction()
{
	check(AgentInfo.IsValid());
	check(bIsRetriggerable || !bIsTriggering);

	if (bIsTriggering && bIsRetriggerable)
	{
		CallOnEndAction();
	}

	bIsTriggering = true;
	
}

void UNexusAction::OnEndAction()
{
	OnActionEndedDelegate.Broadcast(ActionDefHandle, this);
}

void UNexusAction::OnActionRemoved()
{
}
