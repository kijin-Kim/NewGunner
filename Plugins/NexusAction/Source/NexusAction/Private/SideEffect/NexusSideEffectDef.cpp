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

void FNexusSideEffectDef::PostReplicatedChange(const FNexusSideEffectDefContainer& InArraySerializer)
{
	checkNoEntry();
}

void FNexusSideEffectDefContainer::Init(AActor* InOwnerActor)
{
	check(InOwnerActor);
	OwnerActor = InOwnerActor;
	bHasAuthority = InOwnerActor->HasAuthority();
}

void FNexusSideEffectDefContainer::Add(const FNexusSideEffectDef& SideEffectDef, FNexusPredictionTag PredictionTag)
{
	FNexusSideEffectDef NewItem;
	NewItem.Handle = SideEffectDef.Handle;
	NewItem.SideEffectClass = SideEffectDef.SideEffectClass;
	NewItem.SideEffectInstance = SideEffectDef.SideEffectInstance;
	NewItem.PredictionTag = PredictionTag;
	NewItem.SideEffectInstance->OnApplied(PredictionTag, bHasAuthority);

	if (bHasAuthority && NewItem.SideEffectInstance->DurationType != ESideEffectDurationType::Instant)
	{
		MarkItemDirty(Items.Add_GetRef(NewItem));
	}
}

void FNexusSideEffectDefContainer::Remove(const FNexusSideEffectDefHandle& SideEffectDefHandle)
{
	int32 Removed = Items.RemoveAll([SideEffectDefHandle](const FNexusSideEffectDef& SideEffectDefItem)
	{
		if (SideEffectDefHandle == SideEffectDefItem.Handle)
		{
			SideEffectDefItem.SideEffectInstance->OnRemoved();
			return true;
		}
		return false;
	});
	check(Removed != 0);
	if (bHasAuthority)
	{
		MarkArrayDirty();
	}
}

bool FNexusSideEffectDefContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusSideEffectDef, FNexusSideEffectDefContainer>(Items, DeltaParms, *this);
}

void FNexusSideEffectDefContainer::OnAdded(FNexusSideEffectDef& SideEffectDef) const
{
	SideEffectDef.SideEffectInstance = NewObject<UNexusSideEffect>(OwnerActor, SideEffectDef.SideEffectClass);
	SideEffectDef.SideEffectInstance->OnApplied(SideEffectDef.PredictionTag, bHasAuthority);
}

void FNexusSideEffectDefContainer::OnRemoved(const FNexusSideEffectDef& SideEffectDef) const
{
	SideEffectDef.SideEffectInstance->OnRemoved();
}

void FNexusSideEffectDefContainer::Tick(float DeltaTime)
{
	for (FNexusSideEffectDef& SideEffectDef : Items)
	{
		SideEffectDef.SideEffectInstance->OnTick(DeltaTime, bHasAuthority);
	}

	if (bHasAuthority)
	{
		int32 Removed = Items.RemoveAll([this](const FNexusSideEffectDef& SideEffectDef)
		{
			ESideEffectDurationType DurationType = SideEffectDef.SideEffectInstance->DurationType;
			if ((DurationType == ESideEffectDurationType::Instant && bHasAuthority)
				|| ((DurationType == ESideEffectDurationType::Duration) && (SideEffectDef.SideEffectInstance->RemainingDuration <= 0.0f)))
			{
				SideEffectDef.SideEffectInstance->OnRemoved();
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

FNexusSideEffectDef* FNexusSideEffectDefContainer::FindSideEffectDefByHandle(FNexusSideEffectDefHandle Handle)
{
	return Items.FindByPredicate([Handle](const FNexusSideEffectDef& SideEffectDefItem)
	{
		return SideEffectDefItem.Handle == Handle;
	});
}
