// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSideEffectDefinition.h"

#include "GunnerActionSideEffect.h"

FGunnerActionSideEffectDefinition::FGunnerActionSideEffectDefinition(): SideEffectClass(nullptr)
{
	Handle.GenerateNewHandle();
}

FGunnerActionSideEffectDefinition::FGunnerActionSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> InActionClass): SideEffectClass(InActionClass)
{
	Handle.GenerateNewHandle();
}

bool FGunnerActionSideEffectDefinition::operator==(const FGunnerActionSideEffectDefinition& Other) const
{
	return Handle == Other.Handle && SideEffectClass == Other.SideEffectClass;
}

bool FGunnerActionSideEffectDefinition::operator!=(const FGunnerActionSideEffectDefinition& Other) const
{
	return !(*this == Other);
}

void FGunnerActionSideEffectDefinition::PostReplicatedAdd(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnAdded(*this, PredictionHandle);
}

void FGunnerActionSideEffectDefinition::PreReplicatedRemove(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnRemoved(Handle);
}

void FGunnerActionSideEffectDefinition::PostReplicatedChange(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	UE_DEBUG_BREAK();
}

void FGunnerActionSideEffectDefinitionArray::Add(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority)
{
	FGunnerActionSideEffectDefinition NewItem;
	NewItem.Handle = SideEffectDefinition.Handle;
	NewItem.SideEffectClass = SideEffectDefinition.SideEffectClass;
	NewItem.SideEffectInstance = SideEffectDefinition.SideEffectInstance;
	NewItem.PredictionHandle = PredictionHandle;
	NewItem.SideEffectInstance->OnApplied(PredictionHandle);

	int Index = Items.Add(NewItem);
	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
	else
	{
		MarkArrayDirty();
	}


	OnAdded(SideEffectDefinition, PredictionHandle);
}

void FGunnerActionSideEffectDefinitionArray::OnAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle) const
{
	//OnSideEffectDefinitionAddedDelegate.ExecuteIfBound(SideEffectDefinition, PredictionHandle);
}

void FGunnerActionSideEffectDefinitionArray::Remove(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle)
{
	int32 Removed = Items.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionSideEffectDefinition& SideEffectDefinitionItem)
	{
		if (SideEffectDefinitionHandle == SideEffectDefinitionItem.Handle)
		{
			SideEffectDefinitionItem.SideEffectInstance->OnRemoved();
			return true;
		}
		return false;
	});
	check(Removed != 0);
	MarkArrayDirty();
	OnRemoved(SideEffectDefinitionHandle);
}

void FGunnerActionSideEffectDefinitionArray::OnRemoved(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle) const
{
	//OnSideEffectDefinitionRemovedDelegate.ExecuteIfBound(SideEffectDefinitionHandle);
}

bool FGunnerActionSideEffectDefinitionArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionSideEffectDefinition, FGunnerActionSideEffectDefinitionArray>(Items, DeltaParms, *this);
}

void FGunnerActionSideEffectDefinitionArray::Tick(float DeltaTime)
{
	for (FGunnerActionSideEffectDefinition& SideEffectDefinition : Items)
	{
		SideEffectDefinition.SideEffectInstance->OnTick(DeltaTime);
	}

	Items.RemoveAll([](const FGunnerActionSideEffectDefinition& SideEffectDefinition)
	{
		ESideEffectDurationType DurationType = SideEffectDefinition.SideEffectInstance->DurationType;
		if (DurationType == ESideEffectDurationType::Instant
			|| ((DurationType == ESideEffectDurationType::Duration) && (SideEffectDefinition.SideEffectInstance->RemainingDuration <= 0.0f)))
		{
			SideEffectDefinition.SideEffectInstance->OnRemoved();
			return true;
		}
		return false;
	});
}
