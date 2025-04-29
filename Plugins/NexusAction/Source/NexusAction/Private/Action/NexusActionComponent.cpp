// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionComponent.h"
#include "NexusActionInterface.h"
#include "NexusLog.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionListScopeLock.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Engine/Canvas.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "Event/NexusEventMessage.h"
#include "GameFramework/HUD.h"
#include "Net/UnrealNetwork.h"
#include "Prediction/NexusPredictionScope.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectInstance.h"

bool bShowActionFailedReason = false;
FAutoConsoleVariableRef ActionSystemShowActionTriggerFailedReasonCmd(
	TEXT("ActionSystem.Debug.ActionTriggerFailedReason"),
	bShowActionFailedReason,
	TEXT("")
);


UNexusActionComponent::UNexusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AgentInfo = MakeShared<FNexusAgentInfo>();
}

void UNexusActionComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponentFromActor(DebugTarget))
	{
		ActionComponent->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UNexusActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusActionComponent, ActionDefs, COND_OwnerOnly);
	DOREPLIFETIME(UNexusActionComponent, AgentActor);
}

void UNexusActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TeardownActionComponent();
	Super::EndPlay(EndPlayReason);
}

UNexusActionComponent* UNexusActionComponent::GetActionComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UNexusActionComponent* ActionComponent = Actor->GetComponentByClass<UNexusActionComponent>())
	{
		return ActionComponent;
	}

	if (INexusActionInterface* ActionInterface = Cast<INexusActionInterface>(Actor))
	{
		return ActionInterface->GetActionComponent();
	}

	return nullptr;
}

void UNexusActionComponent::UpdateAgentInfo(AActor* InAgentActor)
{
	if (InAgentActor == AgentInfo->GetAgentActor())
	{
		return;
	}

	AgentActor = InAgentActor;
	AgentInfo->Init(GetOwner(), InAgentActor);
}

void UNexusActionComponent::SetupActionComponent(AActor* InAgentActor)
{
	UpdateAgentInfo(InAgentActor);
	if (bSetupCompleted)
	{
		return;
	}
	bSetupCompleted = true;
	InternalSetupActionComponent();
	OnSetupActionComponent();
	if (OnActionComponentSetupCompletedDelegate.IsBound())
	{
		OnActionComponentSetupCompletedDelegate.Broadcast();
		OnActionComponentSetupCompletedDelegate.Clear();
	}
}

void UNexusActionComponent::TeardownActionComponent()
{
	bSetupCompleted = false;
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
		GetPropertyComponent()->AuthRemoveAllProperties();
		GetCueComponent()->RemoveAllLoopingCues();
	}

	if (OnActionComponentTeardownCompletedDelegate.IsBound())
	{
		OnActionComponentTeardownCompletedDelegate.Broadcast();
	}
}

void UNexusActionComponent::CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate)
{
	if (bSetupCompleted)
	{
		Delegate.ExecuteIfBound();
		return;
	}
	OnActionComponentSetupCompletedDelegate.Add(MoveTemp(Delegate));
}

void UNexusActionComponent::RemoveSetupCompletedDelegate(const void* Object)
{
	OnActionComponentSetupCompletedDelegate.RemoveAll(Object);
}

void UNexusActionComponent::AddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate)
{
	OnActionComponentSetupCompletedDelegate.Add(MoveTemp(Delegate));
}

void UNexusActionComponent::InternalSetupActionComponent()
{
	TArray<TObjectPtr<UNexusAgentBoundComponent>> SubComponents;
	GetOwner()->GetComponents(SubComponents);
	for (TObjectPtr<UNexusAgentBoundComponent> SubComponent : SubComponents)
	{
		SubComponent->Setup(AgentInfo);
	}


	ActionDefs.OnActionDefAddedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefAdded);
	ActionDefs.OnActionDefRemovedDelegate.BindUObject(this, &UNexusActionComponent::OnActionDefRemoved);
	ActionDefs.Init();

	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllActions();
	}
}

void UNexusActionComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	if (!AgentInfo.IsValid() || !AgentInfo->AgentActor.IsValid() || DebugTarget != AgentInfo->AgentActor.Get())
	{
		return;
	}


	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("ActionSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::White);
		FGameplayTagContainer OwnedTags;

		TMap<FGameplayTag, FString> PropertyLogs;

		for (const auto& SideEffectInstance : GetSideEffectComponent()->GetSideEffectInstances().SideEffectInstances)
		{
			if (SideEffectInstance.Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Instant)
			{
				continue;
			}

			for (const FNexusPropertyMod& Mod : SideEffectInstance.Def.SideEffectAsset->Modifiers)
			{
				float Value = 0.0f;
				switch (Mod.CalculationType)
				{
				case ENexusPropertyCalculationType::Direct:
					Value = Mod.DirectValue;
					break;
				case ENexusPropertyCalculationType::FromOutside:
					{
						const FNexusInjectedValuePair* PairPtr = SideEffectInstance.Def.InjectedValues.FindByPredicate([&Mod](const FNexusInjectedValuePair& InjectedValue)
						{
							return InjectedValue.Tag == Mod.InjectedValueTag;
						});
						Value = PairPtr ? PairPtr->Value : 0.0f;
						break;
					}

				case ENexusPropertyCalculationType::PropertyBased:
					{
						UNexusProperty* Property = GetProperty(Mod.PropertyTag);
						Value = Property ? Property->GetDynamicValue() : 0.0f;
						break;
					}
				default:
					continue;
				}
				FString& LogString = PropertyLogs.FindOrAdd(Mod.PropertyTag);
				UEnum* StaticOperatorEnum = StaticEnum<ENexusPropertyOperator>();
				check(StaticOperatorEnum);
				FString EffectNameString = FString::Printf(TEXT("%s (%s) - "), *SideEffectInstance.Def.SideEffectAsset->GetName(), *SideEffectInstance.Handle.ToString());
				FString ModString = FString::Printf(TEXT("%s %.2f "), *StaticOperatorEnum->GetNameStringByValue(static_cast<int64>(Mod.Operator)), Value);
				FString TimeString = FString::Printf(TEXT("[%.2f"), SideEffectInstance.ElapsedTime);
				FString DurationString = SideEffectInstance.Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Infinite
					                         ? TEXT("/INF]")
					                         : FString::Printf(TEXT("/%.2f]"), SideEffectInstance.Def.SideEffectAsset->Duration);
				FString IntervalString = SideEffectInstance.Def.SideEffectAsset->Interval <= 0.0f
					                         ? TEXT("")
					                         : FString::Printf(TEXT(" (Interval: %.2f Applied: %d)"), SideEffectInstance.Def.SideEffectAsset->Interval, SideEffectInstance.AppliedCount);
				LogString += EffectNameString + ModString + TimeString + DurationString + IntervalString + TEXT("\n");
			}
		}


		for (UNexusProperty* Property : GetPropertyComponent()->GetProperties())
		{
			DisplayDebugManager.SetDrawColor(FColor::White);
			FString TagString = Property->GetTag().ToString();
			check(TagString.RemoveFromStart(TEXT("Property.")));
			DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: %.2f (정적 값: %.2f)"), *TagString, Property->GetDynamicValue(), Property->GetStaticValue()));
			DisplayDebugManager.SetDrawColor(FColor::Orange);
			if (PropertyLogs.Contains(Property->GetTag()))
			{
				DisplayDebugManager.DrawString(PropertyLogs[Property->GetTag()], 4.0f);
			}
		}

		FString TagString;
		for (const auto& [Tag, Count] : GetGameplayTagComponent()->GetDynamicTagCountMap())
		{
			DisplayDebugManager.SetDrawColor(FColor::White);
			TagString += FString::Printf(TEXT("%s(%d) "), *Tag.ToString(), Count);
		}
		DisplayDebugManager.DrawString(FString::Printf(TEXT("소유 태그: %s"), *TagString));
	}
}

void UNexusActionComponent::OnRep_AgentActor()
{
	if (AgentInfo->GetAgentActor() != AgentActor || AgentInfo->GetOwnerActor() != GetOwner())
	{
		if (GetOwner())
		{
			UpdateAgentInfo(AgentActor);
		}
		else
		{
			UpdateAgentInfo(nullptr);
		}
	}
}

// ------------------------------------------------------------------------------
// Action Add/Remove
// ------------------------------------------------------------------------------
FNexusActionDefHandle UNexusActionComponent::AuthAddAction(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject)
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB_FN(GetAgentActor(), LogNexus, Error, TEXT("권한 없는 함수 호출"));
		return {};
	}

	FNexusActionDef ActionDef(SourceObject, ActionClass);
	return InternalAuthAddAction(ActionDef);
}

void UNexusActionComponent::AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle)
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB_FN(GetAgentActor(), LogNexus, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (ActionScopeLockCount > 0)
	{
		ActionPendingRemoves.Add(ActionDefHandle);
		return;
	}

	ACTION_LIST_SCOPE_LOCK();
	ActionDefs.AuthRemove(ActionDefHandle);
}

void UNexusActionComponent::AuthRemoveAllActions()
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB_FN(GetAgentActor(), LogNexus, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	ACTION_LIST_SCOPE_LOCK();
	ActionDefs.AuthRemoveAll();
}

void UNexusActionComponent::IncreaseActionListLock()
{
	ActionScopeLockCount++;
}

void UNexusActionComponent::DecreaseActionListLock()
{
	ActionScopeLockCount--;
	if (ActionScopeLockCount == 0 && (ActionPendingAdds.IsEmpty() || ActionPendingRemoves.IsEmpty()))
	{
		TArray<FNexusPendingAddActionInfo> PendingAdds = MoveTemp(ActionPendingAdds);
		TArray<FNexusActionDefHandle> PendingRemoves = MoveTemp(ActionPendingRemoves);

		for (const auto& PendingAddInfo : PendingAdds)
		{
			if (GetOwner()->HasAuthority())
			{
				FNexusActionDefHandle Handle = InternalAuthAddAction(PendingAddInfo.ActionDef);
				if (PendingAddInfo.bIsPendingTrigger)
				{
					TryTriggerAction(Handle, PendingAddInfo.PendingEventMessage);
				}
			}
		}

		for (const auto& ActionDef : PendingRemoves)
		{
			if (GetOwner()->HasAuthority())
			{
				AuthRemoveAction(ActionDef);
			}
		}
	}
}

FNexusActionDefHandle UNexusActionComponent::InternalAuthAddAction(const FNexusActionDef& ActionDef)
{
	if (ActionScopeLockCount > 0)
	{
		ActionPendingAdds.Add({ActionDef, false});
		return ActionDef.Handle;
	}

	ACTION_LIST_SCOPE_LOCK();
	NX_CLOG_SUB(GetAgentActor(), ActionDefs.HasSameActionClassAndSourceObject(ActionDef), LogNexusAction, Warning, TEXT("액션 데피니션 중복 추가 시도: %s"), *ActionDef.ToString());
	ActionDefs.AuthAdd(ActionDef);
	return ActionDef.Handle;
}


void UNexusActionComponent::OnActionDefAdded(const FNexusActionDef& ActionDef)
{
	check(bSetupCompleted);
	check(!LocalActionInstanceMap.Contains(ActionDef.Handle));
	UNexusAction* ActionInstance = CreateActionInstance(ActionDef);
	ActionInstance->OnActionEndedDelegate.AddUObject(this, &UNexusActionComponent::OnActionEnded);
	ActionInstance->CallOnAddAction();

	NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Verbose, TEXT("액션 추가: %s"), *ActionDef.ToString());
	HandleTriggerableActionOnAdded(ActionDef, ActionInstance);
}

void UNexusActionComponent::OnActionDefRemoved(const FNexusActionDef& ActionDef)
{
	UNexusAction* ActionInstance = FindActionInstanceByHandle(ActionDef.Handle);
	check(ActionInstance);
	ActionInstance->EndAction();
	ActionInstance->CallOnRemoveAction();
	NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Verbose, TEXT("액션 제거: %s"), *ActionDef.ToString());
	DestroyActionInstance(ActionDef.Handle);
	HandleTriggerableActionOnRemoved(ActionDef.Handle);
}

void UNexusActionComponent::HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDef, UNexusAction* ActionInstance)
{
	if (!HasActionTriggerAuthority(ActionInstance))
	{
		return;
	}

	BindActionTriggerEvent(NewActionDef, ActionInstance);
	if (ActionInstance->ShouldTriggerOnAdded())
	{
		TryTriggerAction(NewActionDef.Handle, FNexusEventMessage());
	}


	ClientPendingActionTriggerRequests.RemoveAll([ActionInstance, NewActionDef = NewActionDef.Handle, this](const FNexusPendingActionTriggerRequest& Request)
	{
		if (Request.ActionDefHandle == NewActionDef)
		{
			NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Verbose, TEXT("액션 지연 실행: %s"), *NewActionDef.ToString());
			TryTriggerAction(Request.ActionDefHandle, Request.EventMessage);
			return true;
		}
		return false;
	});
}

void UNexusActionComponent::HandleTriggerableActionOnRemoved(const FNexusActionDefHandle& ActionDefHandle)
{
	UnbindActionTriggerEvent(ActionDefHandle);
}

void UNexusActionComponent::BindActionTriggerEvent(const FNexusActionDef& NewActionDef, UNexusAction* ActionInstance)
{
	FGameplayTagContainer ActionTriggerEventTags = ActionInstance->GetActionTriggerEventTags();
	for (FGameplayTag Tag : ActionTriggerEventTags)
	{
		FNexusEventCallbackHandle EventCallbackHandle = GetEventManagerComponent()->BindEventCallback<FNexusEventMessage, UNexusActionComponent, const FNexusActionDefHandle&>(Tag, this, &UNexusActionComponent::OnActionEventTriggered, NewActionDef.Handle);
		BoundedActionEventHandles.FindOrAdd(NewActionDef.Handle).Add(EventCallbackHandle);
	}
}

void UNexusActionComponent::UnbindActionTriggerEvent(const FNexusActionDefHandle& ActionDefHandle)
{
	if (TArray<FNexusEventCallbackHandle>* EventCallbackHandles = BoundedActionEventHandles.Find(ActionDefHandle))
	{
		for (FNexusEventCallbackHandle EventCallbackHandle : *EventCallbackHandles)
		{
			GetEventManagerComponent()->UnbindEventCallback(EventCallbackHandle);
		}
		BoundedActionEventHandles.Remove(ActionDefHandle);
	}
}


// ------------------------------------------------------------------------------
// Action Trigger
// ------------------------------------------------------------------------------
bool UNexusActionComponent::HasActionTriggerAuthority(UNexusAction* Action) const
{
	check(Action);
	ENexusActionNetMethod ActionNetMethod = Action->GetActionNetMethod();
	if (ActionNetMethod == ENexusActionNetMethod::LocalOnly || ActionNetMethod == ENexusActionNetMethod::LocalPredicted)
	{
		return AgentInfo->IsLocallyControlled() || Action->IsRemoteTriggerable();
	}

	if (ActionNetMethod == ENexusActionNetMethod::ServerOnly || ActionNetMethod == ENexusActionNetMethod::ServerAuthoritative)
	{
		return GetOwner()->HasAuthority() || Action->IsRemoteTriggerable();
	}

	return false;
}

bool UNexusActionComponent::CanTriggerAction(UNexusAction* ActionInstance, const FNexusEventMessage& EventMessage)
{
	ActionInstance->SetActionCurrentEventMessage(EventMessage);
	return InternalCanTriggerAction(ActionInstance);
}

void UNexusActionComponent::TryTriggerAction(const FNexusActionDefHandle& ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	check(ActionDefHandle.IsValid());


	const FNexusActionDef* ActionDef = FindActionDefByHandle(ActionDefHandle);

	/*
	 * "ActionDef를 찾을 수 없는 이유 (실행 대기를 하는 이유)"
	 *	1. 네트워킹 문제 (ClientRemoteRequestTryTriggerAction)
	 *		1) 클라이언트에서 서버의 리플리케이션 지연으로 인해 ActionDef가 아직 도착하지 않은 경우
	 *		2) 클라이언트에서 서버의 리플리케이션 지연으로 인해 AgentInfo가 유효하지 않은 경우 (아직 ActionComponent가 클라이언트에서 셋업 되지 않음)
	 *	2. 비네트워킹 문제
	 *		1) 클라이언트에서 ScopeLock때문에 ActionDef가 컨테이너에 아직 추가되지 않은 경우
	 */

	if (!ActionDef)
	{
		if (FNexusPendingAddActionInfo* FoundPendingInfo = FindPendingAddActionInfo(ActionDefHandle)) // 1-2
		{
			FoundPendingInfo->bIsPendingTrigger = true;
			return;
		}

		// 1-1
		NX_VLOG_SUB(GetAgentActor(), LogNexusAction, VeryVerbose, TEXT("클라이언트 액션 실행 대기 (리플리케이션 지연): %s"), *ActionDefHandle.ToString());
		ClientPendingActionTriggerRequests.Add({ActionDefHandle, EventMessage});
		return;
	}

	if (!bSetupCompleted) // 2-1
	{
		NX_VLOG_SUB(GetAgentActor(), LogNexusAction, VeryVerbose, TEXT("클라이언트 액션 실행 대기 (셋업이 완료되지 않음): %s"), *ActionDefHandle.ToString());
		ClientPendingActionTriggerRequests.Add({ActionDefHandle, EventMessage});
		return;
	}

	check(AgentInfo->OwnerActor.IsValid());

	UNexusAction* ActionInstance = FindActionInstanceByHandle(ActionDefHandle);
	check(ActionInstance);

	ensureMsgf(ActionDef, TEXT("AddAction을 통해 먼저 해당 Action을 부여해야 합니다"));
	ensureMsgf(AgentInfo->AgentActor->GetLocalRole() != ROLE_SimulatedProxy,
	           TEXT("Agent Actor: [%s]가 SimulatedProxy인 경우 액션 [%s]을(를) 실행할 수 없습니다"), *AgentInfo->AgentActor->GetName(),
	           *ActionInstance->GetName());

	if (!CanTriggerAction(ActionInstance, EventMessage))
	{
		return;
	}


	const bool bIsOwnerActorAuthoritative = AgentInfo->IsOwnerActorAuthoritative();
	FNexusPredictionTag PredictionTag;
	PredictionTag.GenerateNewHandle(bIsOwnerActorAuthoritative);
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());


	if (AgentInfo->OwnerActor->GetNetMode() == NM_Standalone)
	{
		LocalTriggerAction(ActionDefHandle, ActionInstance);
		return;
	}

	const bool bIsLocallyControlled = AgentInfo->IsLocallyControlled();

	const ENexusActionNetMethod ActionNetMethod = ActionInstance->GetActionNetMethod();
	const bool bIsRemoteTriggerable = ActionInstance->IsRemoteTriggerable();


	if (bIsOwnerActorAuthoritative)
	{
		if (bIsLocallyControlled || ActionNetMethod == ENexusActionNetMethod::ServerOnly)
		{
			LocalTriggerAction(ActionDefHandle, ActionInstance);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::ServerAuthoritative)
		{
			LocalTriggerAction(ActionDefHandle, ActionInstance);
			ClientTriggerAction(ActionDefHandle, EventMessage, GetPredictionComponent()->GetCurrentPredictionTag());
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ClientRemoteRequestTryTriggerAction(ActionDefHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
		return;
	}
	if (bIsLocallyControlled) // Autonomous Proxy
	{
		if (ActionNetMethod == ENexusActionNetMethod::LocalOnly)
		{
			LocalTriggerAction(ActionDefHandle, ActionInstance);
			return;
		}

		if (ActionNetMethod == ENexusActionNetMethod::LocalPredicted)
		{
			LocalTriggerAction(ActionDefHandle, ActionInstance);
			ServerTryTriggerAction(ActionDefHandle, EventMessage, GetPredictionComponent()->GetCurrentPredictionTag());
			return;
		}

		if (bIsRemoteTriggerable)
		{
			ServerRemoteRequestTryTriggerAction(ActionDefHandle, EventMessage);
			return;
		}

		checkNoEntry(); // 해당 호스트에서는 Action을(를) 실행할 수 없습니다.
	}
}

bool UNexusActionComponent::InternalCanTriggerAction(UNexusAction* ActionInstance) const
{
	bool bIsNotTriggeringOrRetriggerable = ActionInstance->IsRetriggerable() || !ActionInstance->IsTriggering();
	bool bMetTriggerCondition = ActionInstance->CallOnCanTriggerAction();
	FGameplayTagContainer OwnedTags;
	GetOwnedGameplayTags(OwnedTags);
	bool bHasRequiredTags = OwnedTags.HasAllExact(ActionInstance->GetShouldHaveTags());
	bool bDontHaveForbiddenTags = !OwnedTags.HasAnyExact(ActionInstance->GetShouldNotHaveTags());

	if (!bShowActionFailedReason)
	{
		return bIsNotTriggeringOrRetriggerable && bMetTriggerCondition && bHasRequiredTags && bDontHaveForbiddenTags;
	}

	if (!bIsNotTriggeringOrRetriggerable)
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusAction, Log, TEXT("액션 실행 거부 (실행 중): %s"), *ActionInstance->GetName());
		return false;
	}

	if (!bMetTriggerCondition)
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusAction, Log, TEXT("액션 실행 거부 (조건 미충족): %s"), *ActionInstance->GetName());
		return false;
	}

	if (!bHasRequiredTags)
	{
		const FGameplayTagContainer& ShouldHaveTags = ActionInstance->GetShouldHaveTags();
		FGameplayTagContainer NotOwnedTags;
		for (FGameplayTag Tag : ShouldHaveTags)
		{
			if (!OwnedTags.HasTag(Tag))
			{
				NotOwnedTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(GetAgentActor(), LogNexusAction, Log, TEXT("액션 실행 거부 (필수 태그 미보유): %s; 미보유 태그=[%s]"), *NotOwnedTags.ToStringSimple(true));
		return false;
	}

	if (!bDontHaveForbiddenTags)
	{
		const FGameplayTagContainer& ShouldNotHaveTags = ActionInstance->GetShouldNotHaveTags();
		FGameplayTagContainer OwnedForbiddenTags;
		for (FGameplayTag Tag : ShouldNotHaveTags)
		{
			if (OwnedTags.HasTag(Tag))
			{
				OwnedForbiddenTags.AddTag(Tag);
			}
		}
		NX_LOG_SUB(GetAgentActor(), LogNexusAction, Log, TEXT("액션 실행 거부 (금지 태그 보유): %s; 금지 태그=[%s]"), *OwnedForbiddenTags.ToStringSimple(true));
		return false;
	}

	return true;
}

void UNexusActionComponent::LocalTriggerAction(const FNexusActionDefHandle& ActionDefHandle, UNexusAction* ActionInstance)
{
	NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Display, TEXT( "액션 실행: %s" ), *ActionInstance->GetName());
	check(ActionInstance);
	ActionInstance->CallOnTriggerAction();
	if (IsOwnerActorAuthoritative())
	{
		LocalOnTriggerActionConfirmed(ActionDefHandle, ActionInstance->GetPrimaryPredictionTag());
	}

	if (!ActionInstance->GetActionOwnedTags().IsEmpty())
	{
		FNexusSideEffectInstanceDef SideEffectInstanceDef;
		SideEffectInstanceDef.SideEffectAsset = GetDefault<UNexusSideEffectInfinite>();
		FNexusGameplayTagMod TagMod;
		TagMod.TagsToGrant.AppendTags(ActionInstance->GetActionOwnedTags());
		SideEffectInstanceDef.DynamicTagModifiers.Add(TagMod);
		FNexusSideEffectInstanceHandle SideEffectInstanceHandle = ApplySideEffectByDef(SideEffectInstanceDef);
		if (IsOwnerActorAuthoritative())
		{
			TagSideEffectMap.Add({ActionDefHandle, SideEffectInstanceHandle});
		}
	}
}

void UNexusActionComponent::ServerTryTriggerAction_Implementation(const FNexusActionDefHandle& ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag)
{
	if (!ActionDefHandle.IsValid())
	{
		unimplemented(); // FAIL DESYNC
	}

	UNexusAction* ActionInstance = FindActionInstanceByHandle(ActionDefHandle);
	if (!CanTriggerAction(ActionInstance, EventMessageReplicated.ToEventMessage()))
	{
		ClientTriggerActionRequestFailed(ActionDefHandle, PredictionTag);
		return;
	}

	ClientTriggerActionRequestSucceeded(ActionDefHandle, PredictionTag);
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());
	LocalTriggerAction(ActionDefHandle, ActionInstance);
}

void UNexusActionComponent::ClientTriggerAction_Implementation(const FNexusActionDefHandle& ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag)
{
	check(ActionDefHandle.IsValid());
	UNexusAction* ActionInstance = FindActionInstanceByHandle(ActionDefHandle);
	check(ActionInstance);
	ActionInstance->SetActionCurrentEventMessage(EventMessageReplicated.ToEventMessage());
	FNexusPredictionScope PredictionScope(*GetPredictionComponent(), PredictionTag);
	ActionInstance->SetPrimaryPredictionTag(GetPredictionComponent()->GetCurrentPredictionTag());
	LocalTriggerAction(ActionDefHandle, ActionInstance);
}

void UNexusActionComponent::ClientTriggerActionRequestSucceeded_Implementation(const FNexusActionDefHandle& ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	LocalOnTriggerActionConfirmed(ActionDefHandle, PredictionTag);
}

void UNexusActionComponent::ClientTriggerActionRequestFailed_Implementation(const FNexusActionDefHandle& ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	UNexusAction* ActionInstance = FindActionInstanceByHandle(ActionDefHandle);
	check(ActionInstance);
	ActionInstance->EndAction();
	FNexusPredictionEvents::BroadcastOnPredictionFailed(PredictionTag);
}

void UNexusActionComponent::ServerRemoteRequestTryTriggerAction_Implementation(const FNexusActionDefHandle& ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::ClientRemoteRequestTryTriggerAction_Implementation(const FNexusActionDefHandle& ActionDefHandle, const FNexusEventMessage& EventMessage)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::LocalOnTriggerActionConfirmed(const FNexusActionDefHandle& ActionDefHandle, FNexusPredictionTag PredictionTag)
{
	UNexusAction* ConfirmedActionInstance = FindActionInstanceByHandle(ActionDefHandle);
	NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Verbose, TEXT("액션 승인: %s"), *ConfirmedActionInstance->GetName());

	const FGameplayTagContainer& CancelTags = ConfirmedActionInstance->GetActionCancelTags();
	ACTION_INSTANCE_MAP_SCOPE_LOCK();
	for (const auto& [Handle, LocalActionInstance] : LocalActionInstanceMap)
	{
		if (ConfirmedActionInstance == LocalActionInstance)
		{
			continue;
		}

		if (LocalActionInstance->GetActionOwnedTags().HasAnyExact(CancelTags))
		{
			LocalActionInstance->EndAction();
		}
	}
	ConfirmedActionInstance->CallOnConfirmAction();
}

void UNexusActionComponent::OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, const FNexusActionDefHandle& ActionDefHandle)
{
	TryTriggerAction(ActionDefHandle, EventMessage);
}

void UNexusActionComponent::OnActionEnded(const FNexusActionDefHandle& ActionDefHandle, UNexusAction* Action)
{
	if (IsOwnerActorAuthoritative())
	{
		if (FNexusSideEffectInstanceHandle* TagSideEffectInstanceHandle = TagSideEffectMap.Find(ActionDefHandle))
		{
			GetSideEffectComponent()->UnregisterAndRemoveSideEffect(*TagSideEffectInstanceHandle);
			TagSideEffectMap.Remove(ActionDefHandle);
		}
	}


	NX_VLOG_SUB(GetAgentActor(), LogNexusAction, Display, TEXT("액션 종료: %s"), *Action->GetName());
}

FNexusPendingAddActionInfo* UNexusActionComponent::FindPendingAddActionInfo(const FNexusActionDefHandle& ActionDefHandle)
{
	return ActionPendingAdds.FindByPredicate([ActionDefHandle](const FNexusPendingAddActionInfo& PendingActionInfo)
	{
		return PendingActionInfo.ActionDef.Handle == ActionDefHandle;
	});
}

FNexusActionDefHandle UNexusActionComponent::FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const
{
	return ActionDefs.FindActionDefHandle(ActionClass, SourceObject);
}

const FNexusActionDef* UNexusActionComponent::FindActionDefByHandle(const FNexusActionDefHandle& ActionDefHandle) const
{
	return ActionDefs.FindActionDefByHandle(ActionDefHandle);
}

void UNexusActionComponent::IncreaseLocalActionInstanceMapLock()
{
	LocalActionInstanceMapScopeLockCount++;
}

void UNexusActionComponent::DecreaseLocalActionInstanceMapLock()
{
	LocalActionInstanceMapScopeLockCount--;
	if (LocalActionInstanceMapScopeLockCount == 0)
	{
		TArray<FNexusPendingAddLocalActionInstanceInfo> PendingAdds = MoveTemp(LocalActionInstancePendingAdds);
		TArray<FNexusActionDefHandle> PendingRemoves = MoveTemp(LocalActionInstancePendingRemoves);

		for (const auto& PendingAddInfo : PendingAdds)
		{
			InternalAddActionInstance(PendingAddInfo.Handle, PendingAddInfo.ActionInstance);
		}

		for (const auto& ActionDef : PendingRemoves)
		{
			DestroyActionInstance(ActionDef);
		}
	}
}

UNexusAction* UNexusActionComponent::CreateActionInstance(const FNexusActionDef& ActionDef)
{
	if (LocalActionInstanceMap.Contains(ActionDef.Handle))
	{
		return LocalActionInstanceMap[ActionDef.Handle];
	}

	NX_LOG_SUB(GetAgentActor(), LogNexusAction, VeryVerbose, TEXT("액션 로컬 인스턴스 생성: %s"), *ActionDef.ToString());
	UNexusAction* ActionInstance = UNexusAction::NewNexusActionObject(ActionDef.ActionClass, ActionDef.Handle, AgentInfo, ActionDef.SourceObject);
	return InternalAddActionInstance(ActionDef.Handle, ActionInstance);
}

void UNexusActionComponent::DestroyActionInstance(const FNexusActionDefHandle& Handle)
{
	if (LocalActionInstanceMapScopeLockCount > 0)
	{
		LocalActionInstancePendingRemoves.Add(Handle);
		return;
	}

	ACTION_INSTANCE_MAP_SCOPE_LOCK();
	LocalActionInstanceMap.FindAndRemoveChecked(Handle);
}

UNexusAction* UNexusActionComponent::FindActionInstanceByHandle(const FNexusActionDefHandle& Handle)
{
	return Handle.IsValid() ? LocalActionInstanceMap.FindRef(Handle) : nullptr;
}

FNexusPredictionTag UNexusActionComponent::GetCurrentPredictionTag() const
{
	return GetPredictionComponent()->GetCurrentPredictionTag();
}

UNexusAction* UNexusActionComponent::InternalAddActionInstance(const FNexusActionDefHandle& Handle, UNexusAction* ActionInstance)
{
	if (LocalActionInstanceMapScopeLockCount > 0)
	{
		LocalActionInstancePendingAdds.Add({Handle, ActionInstance});
		return ActionInstance;
	}

	ACTION_INSTANCE_MAP_SCOPE_LOCK();
	return LocalActionInstanceMap.Add(Handle, ActionInstance);
}
