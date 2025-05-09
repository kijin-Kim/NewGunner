// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusPropertyComponent.h"

#include "NexusLog.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "SideEffect/NexusProperty.h"


UNexusPropertyComponent::UNexusPropertyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNexusPropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNexusPropertyComponent, Properties);
}

bool UNexusPropertyComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UNexusProperty* Property : Properties)
	{
		bWroteSomething |= Channel->ReplicateSubobject(Property, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UNexusPropertyComponent::EvaluateProperties()
{
	for (UNexusProperty* Property : Properties)
	{
		if (!Property)
		{
			continue;
		}

		const float OldValue = Property->GetDynamicValue();
		if (Property->Evaluate())
		{
			DirtyPropertyInfos.Add(Property, TPair<float, float>(OldValue, Property->GetDynamicValue()));
		}
	}
}

void UNexusPropertyComponent::PostEvaluateProperties()
{
	for (const auto& [Property, ValueDelta] : DirtyPropertyInfos)
	{
		float OldValue = ValueDelta.Key;
		float NewValue = ValueDelta.Value;
		Property->OnDirtyDelegate.Broadcast(OldValue, NewValue);
		NX_VLOG_SUB(GetAgentActor(), LogNexusProperty, Verbose, TEXT("프로퍼티 더티:  %.2f->%.2f; %s"), OldValue, NewValue, *Property->ToString());
	}

	DirtyPropertyInfos.Empty();
}

void UNexusPropertyComponent::AuthAddProperty(FGameplayTag Tag)
{
	if (ensure(GetOwner()->HasAuthority()))
	{
		UNexusProperty* NewProperty = NewObject<UNexusProperty>(GetOwner());
		NewProperty->SetTag(Tag);
		Properties.Add(NewProperty);
	}
}

void UNexusPropertyComponent::AuthRemoveAllProperties()
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB_FN(GetAgentActor(), LogNexus, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	Properties.Empty();
}

UNexusProperty* UNexusPropertyComponent::GetProperty(FGameplayTag Tag)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});
	return PropertyPtr ? *PropertyPtr : nullptr;
}

float UNexusPropertyComponent::GetPropertyValue(FGameplayTag Tag)
{
	UNexusProperty* Property = GetProperty(Tag);
	return Property ? Property->GetDynamicValue() : 0.0f;
}

void UNexusPropertyComponent::AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		NX_VLOG_SUB(GetAgentActor(), LogNexusProperty, VeryVerbose, TEXT("정적 연산 추가: %s; %s"), *Operation.ToString(), *(*PropertyPtr)->ToString());
		(*PropertyPtr)->AddStaticOperation(Operation);
	}
}

void UNexusPropertyComponent::AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
		NX_VLOG_SUB(GetAgentActor(), LogNexusProperty, VeryVerbose, TEXT("동적 연산 추가: %s; %s"), *Operation.ToString(), *(*PropertyPtr)->ToString());
		(*PropertyPtr)->AddDynamicOperation(Operation);
	}
}

void UNexusPropertyComponent::RemoveOperationByHandle(FGameplayTag Tag, FNexusPropertyOperationHandle OperationHandle)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (UNexusProperty* Property = *PropertyPtr)
	{
		FNexusPropertyOperationQueryResult QueryResult = Property->FindOperationsByHandle(OperationHandle);
		for (const FNexusPropertyOperation& Operation : QueryResult.StaticOperations)
		{
			NX_VLOG_SUB(GetAgentActor(), LogNexusProperty, VeryVerbose, TEXT("정적 연산 삭제: %s; %s"), *Operation.ToString(), *Property->ToString());
			Property->RemoveStaticOperation(Operation);
		}
		for (const FNexusPropertyOperation& Operation : QueryResult.DynamicOperations)
		{
			NX_VLOG_SUB(GetAgentActor(), LogNexusProperty, VeryVerbose, TEXT("동적 연산 삭제: %s; %s"), *Operation.ToString(), *Property->ToString());
			Property->RemoveDynamicOperation(Operation);
		}
	}
}
