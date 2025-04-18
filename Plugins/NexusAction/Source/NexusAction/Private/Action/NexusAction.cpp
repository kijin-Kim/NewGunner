// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusAction.h"


UNexusAction* UNexusAction::NewNexusActionObject(UClass* Class, const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo)
{
	TSharedPtr<FNexusAgentInfo> AgentInfo = InAgentInfo.Pin();
	check(AgentInfo.IsValid() && AgentInfo->GetAgentActor() && AgentInfo->GetOwnerActor());
	UNexusAction* NewAction = NewObject<UNexusAction>(AgentInfo->GetOwnerActor(), Class);
	check(NewAction);
	NewAction->InitializeAction(InActionDefHandle, AgentInfo);
	return NewAction;
}

UWorld* UNexusAction::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusAction::SetActionCurrentEventMessage(const FNexusEventMessage& InEventMessage)
{
	EventMessage = InEventMessage;
}

void UNexusAction::CallOnActionAdded()
{
	check(ActionDefHandle.IsValid() && AgentInfo.IsValid() && TEXT("액션이 올바르게 생성되지 않았습니다. NewNexusActionObject를 사용하여 액션을 생성해야 합니다"));
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

void UNexusAction::EndAction()
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
		EndAction();
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

void UNexusAction::InitializeAction(const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo)
{
	check(InActionDefHandle.IsValid() && InAgentInfo.IsValid());
	ActionDefHandle = InActionDefHandle;
	AgentInfo = InAgentInfo;
}
