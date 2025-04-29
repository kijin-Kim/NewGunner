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
	//ApplyAllModifiers();
}

void FNexusSideEffectInstance::OnTick(float DeltaTime)
{
	if (Def.SideEffectAsset->DurationType == ENexusSideEffectDurationType::Instant)
	{
		ApplyAllModifiers();
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
	return InternalApplySideEffectByInstance(FNexusSideEffectInstance{SideEffectInstanceDef});
}

void FNexusSideEffectInstanceContainer::RemoveSideEffectInstance(const FNexusSideEffectInstanceHandle& SideEffectInstanceHandle)
{
	if (ScopeLockCount > 0)
	{
		PendingRemoves.Add(SideEffectInstanceHandle);
		return;
	}

	EFFECT_CONTAINER_SCOPE_LOCK();
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
	NX_CLOG_SUB(PropertyComponent->GetAgentActor(), ScopeLockCount > 1, LogNexusSideEffect, Warning, TEXT("잠긴 상태의 사이드이펙트 컨테이너에 추가 시도: %s; ScopeLockCount=%d"), *SideEffectInstance.ToString(), ScopeLockCount);
	SideEffectInstance.InitializeSideEffectInstance(PropertyComponent.Get(), GameplayTagComponent.Get(), bHasAuthority);
	NX_CVLOG_SUB(PropertyComponent.IsValid(), PropertyComponent->GetAgentActor(), LogNexusSideEffect, Display, TEXT("사이드이펙트 적용: %s; ScopeLockCount=%d"), *SideEffectInstance.ToString(), ScopeLockCount);
	SideEffectInstance.OnApplied();
}

void FNexusSideEffectInstanceContainer::OnSideEffectInstanceRemoved(const FNexusSideEffectInstance& SideEffectInstance) const
{
	NX_CLOG_SUB(PropertyComponent->GetAgentActor(), ScopeLockCount > 1, LogNexusSideEffect, Warning, TEXT("잠긴 상태의 사이드이펙트 컨테이너에 제거 시도: %s; ScopeLockCount=%d"), *SideEffectInstance.ToString(), ScopeLockCount);
	NX_CVLOG_SUB(PropertyComponent.IsValid(), PropertyComponent->GetAgentActor(), LogNexusSideEffect, Display, TEXT("사이드이펙트 제거: %s; ScopeLockCount=%d"), *SideEffectInstance.ToString(), ScopeLockCount);
	SideEffectInstance.OnRemoved();
}

void FNexusSideEffectInstanceContainer::Tick(float DeltaTime)
{
	TEMP_LOOPING = true;
	check(ScopeLockCount == 0);
	EFFECT_CONTAINER_SCOPE_LOCK();
	for (FNexusSideEffectInstance& SideEffectInstance : SideEffectInstances)
	{
		SideEffectInstance.OnTick(DeltaTime);
	}

	if (PropertyComponent.IsValid())
	{
		PropertyComponent->EvaluateProperties();
	}

	if (bHasAuthority)
	{
		RemoveSideEffectInstanceByPredicate([](const FNexusSideEffectInstance& SideEffectInstance)
		{
			return SideEffectInstance.IsExpired();
		});
	}
	TEMP_LOOPING = false;
}

void FNexusSideEffectInstanceContainer::IncreaseSideEffectContainerLock()
{
	ScopeLockCount++;
}

void FNexusSideEffectInstanceContainer::DecreaseSideEffectContainerLock()
{
	ScopeLockCount--;
	if (ScopeLockCount == 0 && (PendingAdds.Num() != 0 || PendingRemoves.Num() != 0))
	{
		TArray<FNexusSideEffectInstance> PendingAddsCopy = MoveTemp(PendingAdds);
		TArray<FNexusSideEffectInstanceHandle> PendingRemovesCopy = MoveTemp(PendingRemoves);

		for (const FNexusSideEffectInstance& SideEffectInstance : PendingAddsCopy)
		{
			InternalApplySideEffectByInstance(SideEffectInstance);
		}

		for (const FNexusSideEffectInstanceHandle& SideEffectInstanceHandle : PendingRemovesCopy)
		{
			RemoveSideEffectInstance(SideEffectInstanceHandle);
		}
	}
}

FNexusSideEffectInstanceHandle FNexusSideEffectInstanceContainer::InternalApplySideEffectByInstance(const FNexusSideEffectInstance& SideEffectInstance)
{
	if (ScopeLockCount > 0)
	{
		const int32 Index = PendingAdds.Add(SideEffectInstance);
		check(Index != INDEX_NONE);
		return PendingAdds[Index].Handle;
	}

	EFFECT_CONTAINER_SCOPE_LOCK();
	const int32 Index = SideEffectInstances.Add(SideEffectInstance);
	check(Index != INDEX_NONE);
	if (bHasAuthority)
	{
		MarkItemDirty(SideEffectInstances[Index]);
	}
	OnSideEffectInstanceAdded(SideEffectInstances[Index]);
	return SideEffectInstances[Index].Handle;
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

FNexusSideEffectContainerLock::FNexusSideEffectContainerLock(FNexusSideEffectInstanceContainer& InSideEffectContainer) :
	SideEffectContainer(InSideEffectContainer)
{
	SideEffectContainer.IncreaseSideEffectContainerLock();
}

FNexusSideEffectContainerLock::~FNexusSideEffectContainerLock()
{
	SideEffectContainer.DecreaseSideEffectContainerLock();
}
