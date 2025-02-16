// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusSideEffectDef.h"

#include "SideEffect/NexusSideEffect.h"

FNexusSideEffectDef::FNexusSideEffectDef(): SideEffectClass(nullptr)
{
	Handle.GenerateNewHandle();
}

FNexusSideEffectDef::FNexusSideEffectDef(TSubclassOf<UNexusSideEffect> InActionClass): SideEffectClass(InActionClass)
{
	Handle.GenerateNewHandle();
}

bool FNexusSideEffectDef::operator==(const FNexusSideEffectDef& Other) const
{
	return Handle == Other.Handle && SideEffectClass == Other.SideEffectClass;
}

bool FNexusSideEffectDef::operator!=(const FNexusSideEffectDef& Other) const
{
	return !(*this == Other);
}

void FNexusSideEffectDef::PostReplicatedAdd(const FNexusSideEffectDefContainer& InArraySerializer)
{
	InArraySerializer.OnAdded(*this);
}

void FNexusSideEffectDef::PreReplicatedRemove(const FNexusSideEffectDefContainer& InArraySerializer)
{
	InArraySerializer.OnRemoved(*this);
}

void FNexusSideEffectDefContainer::Init(AActor* InOwnerActor)
{
	check(InOwnerActor);
	OwnerActor = InOwnerActor;
	bHasAuthority = InOwnerActor->HasAuthority();
}

void FNexusSideEffectDefContainer::Add(const FNexusSideEffectDef& SideEffectDefinition, FNexusPredictionTag PredictionHandle)
{
	FNexusSideEffectDef NewItem;
	NewItem.Handle = SideEffectDefinition.Handle;
	NewItem.SideEffectClass = SideEffectDefinition.SideEffectClass;
	NewItem.SideEffectInstance = SideEffectDefinition.SideEffectInstance;
	NewItem.PredictionHandle = PredictionHandle;
	NewItem.SideEffectInstance->OnApplied(PredictionHandle, bHasAuthority);

	int Index = Items.Add(NewItem);
	if (bHasAuthority && NewItem.SideEffectInstance->DurationType != ESideEffectDurationType::Instant)
	{
		MarkItemDirty(Items[Index]);
	}
}

void FNexusSideEffectDefContainer::Remove(const FNexusSideEffectDefHandle& SideEffectDefinitionHandle)
{
	int32 Removed = Items.RemoveAll([SideEffectDefinitionHandle](const FNexusSideEffectDef& SideEffectDefinitionItem)
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
}

bool FNexusSideEffectDefContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusSideEffectDef, FNexusSideEffectDefContainer>(Items, DeltaParms, *this);
}

void FNexusSideEffectDefContainer::OnAdded(FNexusSideEffectDef& SideEffectDefinition) const
{
	SideEffectDefinition.SideEffectInstance = NewObject<UNexusSideEffect>(OwnerActor, SideEffectDefinition.SideEffectClass);
	SideEffectDefinition.SideEffectInstance->OnApplied(SideEffectDefinition.PredictionHandle, bHasAuthority);
}

void FNexusSideEffectDefContainer::OnRemoved(const FNexusSideEffectDef& SideEffectDefinition) const
{
	SideEffectDefinition.SideEffectInstance->OnRemoved();
}

void FNexusSideEffectDefContainer::Tick(float DeltaTime)
{
	for (FNexusSideEffectDef& SideEffectDefinition : Items)
	{
		SideEffectDefinition.SideEffectInstance->OnTick(DeltaTime, bHasAuthority);
	}

	if (bHasAuthority)
	{
		int32 Removed = Items.RemoveAll([this](const FNexusSideEffectDef& SideEffectDefinition)
		{
			ESideEffectDurationType DurationType = SideEffectDefinition.SideEffectInstance->DurationType;
			if ((DurationType == ESideEffectDurationType::Instant && bHasAuthority)
				|| ((DurationType == ESideEffectDurationType::Duration) && (SideEffectDefinition.SideEffectInstance->RemainingDuration <= 0.0f)))
			{
				SideEffectDefinition.SideEffectInstance->OnRemoved();
				return true;
			}
			return false;
		});
		if (Removed != 0)
		{
			MarkArrayDirty();
		}
	}
}
