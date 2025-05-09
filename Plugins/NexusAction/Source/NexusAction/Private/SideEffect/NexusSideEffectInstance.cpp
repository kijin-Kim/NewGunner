// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusSideEffectInstance.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "SideEffect/NexusSideEffect.h"
#include "NexusLog.h"


FNexusSideEffectInstanceDef::FNexusSideEffectInstanceDef() :
	SideEffectAsset(nullptr)
	, InjectedValues()
	, InjectedTagModifiers()
	, AppliedOperationHandles()
{
}

FNexusSideEffectInstanceDef::FNexusSideEffectInstanceDef(TSubclassOf<UNexusSideEffect> InSideEffectClass) :
	SideEffectAsset(InSideEffectClass.GetDefaultObject())
	, InjectedValues()
	, InjectedTagModifiers()
	, AppliedOperationHandles()
{
}

FString FNexusSideEffectInstanceDef::ToString() const
{
	return GetNameSafe(SideEffectAsset);
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
	if (Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Instant)
	{
		return;
	}


	if (Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Duration)
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


	if (Def.SideEffectAsset->DurationType != ENexusSideEffectDurationType::Instant || !bHasAuthority)
	{
		for (const FNexusGameplayTagMod& TagMod : Def.SideEffectAsset->TagModifiers)
		{
			RemoveTagModifier(TagMod);
		}

		for (const FNexusGameplayTagMod& TagMod : Def.InjectedTagModifiers)
		{
			RemoveTagModifier(TagMod);
		}
	}
}

void FNexusSideEffectInstance::ApplyPropertyModifier(const FNexusPropertyMod& Modifier)
{
	if (Def.SideEffectAsset->DurationType != ENexusSideEffectDurationType::Instant && Interval > 0.0f
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


	if ((Def.SideEffectAsset->DurationType != ENexusSideEffectDurationType::Instant && Interval <= 0.0f)
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
	check(GameplayTagComponent.IsValid());

	if ((Def.SideEffectAsset->DurationType != ENexusSideEffectDurationType::Instant && Interval <= 0.0f)
		|| (!bHasAuthority))
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
	else
	{
		for (const FGameplayTag& Tag : Modifier.TagsToGrant)
		{
			GameplayTagComponent->PushStaticTag(Tag);
		}
		for (const FGameplayTag& Tag : Modifier.TagsToRevoke)
		{
			GameplayTagComponent->PopStaticTag(Tag);
		}
	}
}

void FNexusSideEffectInstance::RemoveTagModifier(const FNexusGameplayTagMod& Modifier) const
{
	check(GameplayTagComponent.IsValid());

	if ((Def.SideEffectAsset->DurationType != ENexusSideEffectDurationType::Instant && Interval <= 0.0f)
		|| (!bHasAuthority))
	{
		for (const FGameplayTag& Tag : Modifier.TagsToGrant)
		{
			GameplayTagComponent->PopDynamicTag(Tag);
		}
		for (const FGameplayTag& Tag : Modifier.TagsToRevoke)
		{
			GameplayTagComponent->PushDynamicTag(Tag);
		}
	}
	else
	{
		for (const FGameplayTag& Tag : Modifier.TagsToGrant)
		{
			GameplayTagComponent->PopStaticTag(Tag);
		}
		for (const FGameplayTag& Tag : Modifier.TagsToRevoke)
		{
			GameplayTagComponent->PushStaticTag(Tag);
		}
	}
}

void FNexusSideEffectInstance::ApplyAllModifiers()
{
	for (const FNexusPropertyMod& Modifier : Def.SideEffectAsset->Modifiers)
	{
		NX_LOG_SUB(PropertyComponent->GetAgentActor(), LogNexusSideEffect, VeryVerbose, TEXT("프로퍼티 모디파이어 적용: %s"), *Modifier.ToString());
		ApplyPropertyModifier(Modifier);
	}

	for (const FNexusGameplayTagMod& TagMod : Def.SideEffectAsset->TagModifiers)
	{
		NX_LOG_SUB(PropertyComponent->GetAgentActor(), LogNexusSideEffect, VeryVerbose, TEXT("태그 모디파이어 적용: %s"), *TagMod.ToString());
		ApplyTagModifier(TagMod);
	}

	for (const FNexusGameplayTagMod& TagMod : Def.InjectedTagModifiers)
	{
		NX_LOG_SUB(PropertyComponent->GetAgentActor(), LogNexusSideEffect, VeryVerbose, TEXT("태그 모디파이어 적용: %s"), *TagMod.ToString());
		ApplyTagModifier(TagMod);
	}
}


bool FNexusSideEffectInstance::IsExpired() const
{
	return Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Instant
		|| (Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Duration && RemainingDuration <= 0.0f);
}


void FNexusSideEffectInstanceContainer::Init(UNexusPropertyComponent* InPropertyComponent, UNexusGameplayTagComponent* InGameplayTagComponent, bool bInHasAuthority)
{
	PropertyComponent = InPropertyComponent;
	GameplayTagComponent = InGameplayTagComponent;
	bHasAuthority = bInHasAuthority;
}

FNexusSideEffectInstanceHandle FNexusSideEffectInstanceContainer::ApplySideEffectByDef(const FNexusSideEffectInstanceDef& SideEffectInstanceDef)
{
	const int32 Index = SideEffectInstances.Add(FNexusSideEffectInstance{SideEffectInstanceDef});
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
	const int32 Removed = RemoveSideEffectInstanceByPredicate([SideEffectInstanceHandle](const FNexusSideEffectInstance& SideEffectInstance)
	{
		return SideEffectInstance.Handle == SideEffectInstanceHandle;
	});
	check(Removed != 0);
}

bool FNexusSideEffectInstanceContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusSideEffectInstance, FNexusSideEffectInstanceContainer>(SideEffectInstances, DeltaParms, *this);
}

void FNexusSideEffectInstanceContainer::OnSideEffectInstanceAdded(FNexusSideEffectInstance& SideEffectInstance) const
{
	SideEffectInstance.InitializeSideEffectInstance(PropertyComponent.Get(), GameplayTagComponent.Get(), bHasAuthority);
	NX_CVLOG_SUB(PropertyComponent.IsValid(), PropertyComponent->GetAgentActor(), LogNexusSideEffect, Display, TEXT("사이드이펙트 적용: %s"), *SideEffectInstance.ToString());
	SideEffectInstance.OnApplied();
}

void FNexusSideEffectInstanceContainer::OnSideEffectInstanceRemoved(const FNexusSideEffectInstance& SideEffectInstance) const
{
	NX_CVLOG_SUB(PropertyComponent.IsValid(), PropertyComponent->GetAgentActor(), LogNexusSideEffect, Display, TEXT("사이드이펙트 제거: %s"), *SideEffectInstance.ToString());
	SideEffectInstance.OnRemoved();
}

void FNexusSideEffectInstanceContainer::Tick(float DeltaTime)
{
	for (FNexusSideEffectInstance& SideEffectInstance : SideEffectInstances)
	{
		SideEffectInstance.OnTick(DeltaTime);
	}

	if (PropertyComponent.IsValid())
	{
		PropertyComponent->EvaluateProperties();
	}

	if (GameplayTagComponent.IsValid())
	{
		GameplayTagComponent->EvaluateTagCounts();
	}

	if (bHasAuthority)
	{
		RemoveSideEffectInstanceByPredicate([](const FNexusSideEffectInstance& SideEffectInstance)
		{
			return SideEffectInstance.IsExpired();
		});
	}
	
	if (PropertyComponent.IsValid())
	{
		PropertyComponent->PostEvaluateProperties();
	}

}


int32 FNexusSideEffectInstanceContainer::RemoveSideEffectInstanceByPredicate(const TFunction<bool(const FNexusSideEffectInstance&)>& Predicate)
{
	int32 Removed = SideEffectInstances.RemoveAll([this, Predicate](FNexusSideEffectInstance& SideEffectInstance)
	{
		if (Predicate(SideEffectInstance))
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
	return Removed;
}
