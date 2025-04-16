// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusSideEffectInstance.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "SideEffect/NexusSideEffect.h"


FNexusSideEffectInstanceDef::FNexusSideEffectInstanceDef() :
	SideEffectAsset(nullptr)
	, InjectedValues()
	, DynamicTagModifiers()
	, AppliedOperationHandles()
{
}

FNexusSideEffectInstanceDef::FNexusSideEffectInstanceDef(TSubclassOf<UNexusSideEffect> InSideEffectClass) :
	SideEffectAsset(InSideEffectClass.GetDefaultObject())
	, InjectedValues()
	, DynamicTagModifiers()
	, AppliedOperationHandles()
{
}

FNexusSideEffectInstance::FNexusSideEffectInstance() :
	Def()
	, RemainingDuration(0.0f)
	, ElapsedTime(0.0f)
	, Interval(0.0f)
	, AppliedCount(0)
	, PredictionTag()
	, GameplayTagComponent(nullptr)
	, PropertyComponent(nullptr)
	, bHasAuthority(false)
{
	Handle.GenerateNewHandle();
}

FNexusSideEffectInstance::FNexusSideEffectInstance(const FNexusSideEffectInstanceDef& InDef) :
	Def(InDef)
	, RemainingDuration(0.0f)
	, ElapsedTime(0.0f)
	, Interval(0.0f)
	, AppliedCount(0)
	, PredictionTag()
	, GameplayTagComponent(nullptr)
	, PropertyComponent(nullptr)
	, bHasAuthority(false)
{
	Handle.GenerateNewHandle();
}

bool FNexusSideEffectInstance::operator==(const FNexusSideEffectInstance& Other) const
{
	return Handle == Other.Handle;
}

bool FNexusSideEffectInstance::operator!=(const FNexusSideEffectInstance& Other) const
{
	return !(*this == Other);
}

void FNexusSideEffectInstance::InitializeSideEffectInstance(UNexusPropertyComponent* InPropertyComponent, UNexusGameplayTagComponent* InGameplayTagComponent, bool bInHasAuthority)
{
	PropertyComponent = InPropertyComponent;
	GameplayTagComponent = InGameplayTagComponent;
	bHasAuthority = bInHasAuthority;
}

void FNexusSideEffectInstance::PostReplicatedAdd(const FNexusSideEffectInstanceContainer& InArraySerializer)
{
	InArraySerializer.OnSideEffectInstanceAdded(*this);
}

void FNexusSideEffectInstance::PreReplicatedRemove(const FNexusSideEffectInstanceContainer& InArraySerializer)
{
	InArraySerializer.OnSideEffectInstanceRemoved(*this);
}

void FNexusSideEffectInstance::PostReplicatedChange(const FNexusSideEffectInstanceContainer& InArraySerializer)
{
	checkNoEntry();
}

void FNexusSideEffectInstance::OnApplied()
{
	RemainingDuration = Def.SideEffectAsset->Duration;
	ApplyAllModifiers();
}

void FNexusSideEffectInstance::OnTick(float DeltaTime)
{
	if (Def.SideEffectAsset->DurationType == ESideEffectDurationType::Duration)
	{
		RemainingDuration -= DeltaTime;
	}

	float TimePassedBeforDuration = DeltaTime;
	if (RemainingDuration < 0.0f)
	{
		TimePassedBeforDuration += RemainingDuration;
	}

	ElapsedTime += TimePassedBeforDuration;
	int32 ApplyCount = Interval > 0.0f ? ElapsedTime / Interval : 0;
	AppliedCount += ApplyCount;
	ElapsedTime -= ApplyCount * Interval;

	for (int32 i = 0; i < ApplyCount; ++i)
	{
		ApplyAllModifiers();
	}
}

void FNexusSideEffectInstance::OnRemoved() const
{
	for (const FNexusPropertyMod& Modifier : Def.SideEffectAsset->Modifiers)
	{
		for (FNexusPropertyOperationHandle OperationHandle : Def.AppliedOperationHandles)
		{
			if (PropertyComponent.IsValid())
			{
				PropertyComponent->RemoveOperationByHandle(Modifier.PropertyTag, OperationHandle);
			}
		}
	}

	if (!GameplayTagComponent.IsValid())
	{
		return;
	}


	if (Def.SideEffectAsset->DurationType != ESideEffectDurationType::Instant || !bHasAuthority)
	{
		for (const FNexusGameplayTagMod& TagMod : Def.SideEffectAsset->TagModifiers)
		{
			for (const FGameplayTag& Tag : TagMod.TagsToGrant)
			{
				GameplayTagComponent->PopDynamicTag(Tag);
			}
			for (const FGameplayTag& Tag : TagMod.TagsToRevoke)
			{
				GameplayTagComponent->PushDynamicTag(Tag);
			}
		}
	}
}

void FNexusSideEffectInstance::ApplyPropertyModifier(const FNexusPropertyMod& Modifier)
{
	if (Def.SideEffectAsset->DurationType != ESideEffectDurationType::Instant && Interval > 0.0f
		&& !bHasAuthority)
	{
		return;
	}


	if (!PropertyComponent.IsValid())
	{
		return;
	}

	UNexusProperty* Property = PropertyComponent->GetProperty(Modifier.PropertyTag);
	if (!Property)
	{
		return;
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::None)
	{
		return;
	}

	float DesiredValue = 0.0f;
	if (Modifier.CalculationType == ENexusPropertyCalculationType::Direct)
	{
		DesiredValue = Modifier.DirectValue;
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::PropertyBased)
	{
		UNexusProperty* BaseProperty = PropertyComponent->GetProperty(Modifier.PropertyTag);
		check(BaseProperty);
		DesiredValue = BaseProperty->GetDynamicValue();
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::FromOutside)
	{
		const FNexusInjectedValuePair* PairPtr = Def.InjectedValues.FindByPredicate([&Modifier](const FNexusInjectedValuePair& InjectedValue)
		{
			return InjectedValue.Tag == Modifier.InjectedValueTag;
		});
		DesiredValue = PairPtr ? PairPtr->Value : 0.0f;
	}


	if ((Def.SideEffectAsset->DurationType != ESideEffectDurationType::Instant && Interval <= 0.0f)
		|| (!bHasAuthority))
	{
		FNexusPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		Def.AppliedOperationHandles.Add(NewOperation.Handle);
		PropertyComponent->AddDynamicOperation(Modifier.PropertyTag, NewOperation);
	}
	else
	{
		FNexusPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		Def.AppliedOperationHandles.Add(NewOperation.Handle);
		PropertyComponent->AddStaticOperation(Modifier.PropertyTag, NewOperation);
	}
}

void FNexusSideEffectInstance::ApplyTagModifier(const FNexusGameplayTagMod& Modifier)
{
	if (GameplayTagComponent.IsValid())
	{
		for (const FGameplayTag& Tag : Modifier.TagsToGrant)
		{
			GameplayTagComponent->PushDynamicTag(Tag);
		}
		for (const FGameplayTag& Tag : Modifier.TagsToRevoke)
		{
			GameplayTagComponent->PopDynamicTag(Tag);
		}
	}
}

void FNexusSideEffectInstance::ApplyAllModifiers()
{
	for (const FNexusPropertyMod& Modifier : Def.SideEffectAsset->Modifiers)
	{
		ApplyPropertyModifier(Modifier);
	}

	for (const FNexusGameplayTagMod& TagMod : Def.SideEffectAsset->TagModifiers)
	{
		ApplyTagModifier(TagMod);
	}
}


bool FNexusSideEffectInstance::IsExpired() const
{
	return Def.SideEffectAsset->DurationType == ESideEffectDurationType::Instant
		|| (Def.SideEffectAsset->DurationType == ESideEffectDurationType::Duration && RemainingDuration <= 0.0f);
}


void FNexusSideEffectInstanceContainer::Init(UNexusPropertyComponent* InPropertyComponent, UNexusGameplayTagComponent* InGameplayTagComponent, bool bInHasAuthority)
{
	PropertyComponent = InPropertyComponent;
	GameplayTagComponent = InGameplayTagComponent;
	bHasAuthority = bInHasAuthority;
}

FNexusSideEffectInstanceHandle FNexusSideEffectInstanceContainer::ApplySideEffectByDef(const FNexusSideEffectInstanceDef& SideEffectInstanceDef)
{
	int32 Index = SideEffectInstances.Add(FNexusSideEffectInstance{SideEffectInstanceDef});
	check(Index != INDEX_NONE);
	if (bHasAuthority)
	{
		MarkItemDirty(SideEffectInstances[Index]);
	}
	OnSideEffectInstanceAdded(SideEffectInstances[Index]);
	return SideEffectInstances[Index].Handle;
}

void FNexusSideEffectInstanceContainer::RemoveSideEffectInstance(const FNexusSideEffectInstanceHandle& SideEffectInstanceHandle)
{
	const int32 Removed = SideEffectInstances.RemoveAll([this, SideEffectInstanceHandle](FNexusSideEffectInstance& Element)
	{
		if (Element.Handle == SideEffectInstanceHandle)
		{
			OnSideEffectInstanceRemoved(Element);
			return true;
		}
		return false;
	});
	check(Removed != 0);
	if (bHasAuthority) // TODO: 필요한지 테스트
	{
		MarkArrayDirty();
	}
}

bool FNexusSideEffectInstanceContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusSideEffectInstance, FNexusSideEffectInstanceContainer>(SideEffectInstances, DeltaParms, *this);
}

void FNexusSideEffectInstanceContainer::OnSideEffectInstanceAdded(FNexusSideEffectInstance& SideEffectInstance) const
{
	SideEffectInstance.InitializeSideEffectInstance(PropertyComponent.Get(), GameplayTagComponent.Get(), bHasAuthority);
	SideEffectInstance.OnApplied();
}

void FNexusSideEffectInstanceContainer::OnSideEffectInstanceRemoved(const FNexusSideEffectInstance& SideEffectInstance) const
{
	SideEffectInstance.OnRemoved();
}

void FNexusSideEffectInstanceContainer::Tick(float DeltaTime)
{
	for (FNexusSideEffectInstance& SideEffectInstance : SideEffectInstances)
	{
		SideEffectInstance.OnTick(DeltaTime);
	}

	if (bHasAuthority)
	{
		int32 Removed = SideEffectInstances.RemoveAll([this](FNexusSideEffectInstance& SideEffectInstance)
		{
			if (SideEffectInstance.IsExpired())
			{
				OnSideEffectInstanceRemoved(SideEffectInstance);
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
