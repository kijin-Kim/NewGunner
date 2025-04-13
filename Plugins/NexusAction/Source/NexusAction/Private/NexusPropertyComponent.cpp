// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusPropertyComponent.h"

#include "NexusLog.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "SideEffect/NexusProperty.h"


UNexusPropertyComponent::UNexusPropertyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
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

void UNexusPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (UNexusProperty* Property : Properties)
	{
		if (Property)
		{
			Property->Tick();
		}
	}
}


void UNexusPropertyComponent::AuthAddProperty(FGameplayTag Tag, float Value)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		NX_LOG_SUB_FN(LogNexus, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return;
	}

	UNexusProperty* NewProperty = NewObject<UNexusProperty>(GetOwner());
	NewProperty->SetTag(Tag);
	NewProperty->SetStaticValue(Value);
	Properties.Add(NewProperty);
}

void UNexusPropertyComponent::AuthRemoveAllProperties()
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		NX_LOG_SUB_FN(LogNexus, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
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

void UNexusPropertyComponent::AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
	{
		return Property->GetTag() == Tag;
	});

	if (PropertyPtr)
	{
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
		(*PropertyPtr)->AddDynamicOperation(Operation);
	}
}

void UNexusPropertyComponent::RemoveOperationByHandle(FGameplayTag Tag, FNexusPropertyOperationHandle OperationHandle)
{
	TObjectPtr<UNexusProperty>* PropertyPtr = Properties.FindByPredicate([Tag](UNexusProperty* Property)
		{
			return Property->GetTag() == Tag;
		});

	if (PropertyPtr)
	{
		(*PropertyPtr)->RemoveOperationByHandle(OperationHandle);
	}
}
