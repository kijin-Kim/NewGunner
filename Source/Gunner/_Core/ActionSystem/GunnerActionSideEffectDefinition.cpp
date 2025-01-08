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
	UE_LOG(LogTemp, Warning, TEXT( "FGunnerActionSideEffectDefinition::PostReplicatedAdd: [%s]" ), *SideEffectClass->GetName());
	InArraySerializer.OnAdded(*this);
}

void FGunnerActionSideEffectDefinition::PreReplicatedRemove(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT( "FGunnerActionSideEffectDefinition::PreReplicatedRemove: [%s]" ), *SideEffectClass->GetName());
	InArraySerializer.OnRemoved(*this);
}

void FGunnerActionSideEffectDefinition::PostReplicatedChange(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	UE_DEBUG_BREAK();
}

void FGunnerActionSideEffectDefinitionArray::Init(AActor* InOwnerActor)
{
	check(InOwnerActor);
	OwnerActor = InOwnerActor;
	bHasAuthority = InOwnerActor->HasAuthority();
}

void FGunnerActionSideEffectDefinitionArray::Add(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle)
{
	FGunnerActionSideEffectDefinition NewItem;
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
}

bool FGunnerActionSideEffectDefinitionArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionSideEffectDefinition, FGunnerActionSideEffectDefinitionArray>(Items, DeltaParms, *this);
}

void FGunnerActionSideEffectDefinitionArray::OnAdded(FGunnerActionSideEffectDefinition& SideEffectDefinition) const
{
	SideEffectDefinition.SideEffectInstance = NewObject<UGunnerActionSideEffect>(OwnerActor, SideEffectDefinition.SideEffectClass);
	SideEffectDefinition.SideEffectInstance->OnApplied(SideEffectDefinition.PredictionHandle, bHasAuthority);
}

void FGunnerActionSideEffectDefinitionArray::OnRemoved(const FGunnerActionSideEffectDefinition& SideEffectDefinition) const
{
	SideEffectDefinition.SideEffectInstance->OnRemoved();
}

void FGunnerActionSideEffectDefinitionArray::Tick(float DeltaTime)
{
	for (FGunnerActionSideEffectDefinition& SideEffectDefinition : Items)
	{
		SideEffectDefinition.SideEffectInstance->OnTick(DeltaTime, bHasAuthority);
	}

	if (bHasAuthority)
	{
		int32 Removed = Items.RemoveAll([this](const FGunnerActionSideEffectDefinition& SideEffectDefinition)
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
