// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusAction.h"

#include "Misc/DataValidation.h"


#if WITH_EDITOR
EDataValidationResult UNexusAction::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	TArray<const FGameplayTagContainer*> TagContainerPtrs = {
		&ActionTriggerEventTags,
		&ActionOwnedTags,
		&ActionCancelTags,
		&ShouldHaveTags,
		&ShouldNotHaveTags
	};

	for (const FGameplayTagContainer* TagContainer : TagContainerPtrs)
	{
		for (const FGameplayTag& Tag : *TagContainer)
		{
			if (!Tag.IsValid())
			{
				Context.AddError(NSLOCTEXT("NexusAction", "InvalidTag", "유효하지 않은 태그 존재"));
				Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
			}
		}
	}

	return Result;
}
#endif

UNexusAction* UNexusAction::NewNexusActionObject(UClass* Class, const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo, TWeakObjectPtr<UObject> InSourceObject)
{
	
	TSharedPtr<FNexusAgentInfo> AgentInfo = InAgentInfo.Pin();
	check(AgentInfo.IsValid() && AgentInfo->GetAgentActor() && AgentInfo->GetOwnerActor());
	UNexusAction* NewAction = NewObject<UNexusAction>(AgentInfo->GetOwnerActor(), Class);
	check(NewAction);
	NewAction->InitializeAction(InActionDefHandle, AgentInfo, InSourceObject);
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

void UNexusAction::CallOnAddAction()
{
	check(ActionDefHandle.IsValid() && AgentInfo.IsValid() && TEXT("액션이 올바르게 생성되지 않았습니다. NewNexusActionObject를 사용하여 액션을 생성해야 합니다"));
	OnAddAction();
	BP_OnAddAction();
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

void UNexusAction::CallOnConfirmAction()
{
	OnConfirmAction();
	BP_OnConfirmAction();
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

void UNexusAction::CallOnRemoveAction()
{
	OnRemoveAction();
	BP_OnRemoveAction();
}

void UNexusAction::OnAddAction()
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

void UNexusAction::OnConfirmAction()
{
}

void UNexusAction::OnEndAction()
{
	OnActionEndedDelegate.Broadcast(ActionDefHandle, this);
}

void UNexusAction::OnRemoveAction()
{
}

void UNexusAction::InitializeAction(const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo, TWeakObjectPtr<UObject> InSourceObject)
{
	check(InActionDefHandle.IsValid() && InAgentInfo.IsValid());
	ActionDefHandle = InActionDefHandle;
	AgentInfo = InAgentInfo;
	SourceObject = InSourceObject;
}
