// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_PawnState.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerViewModel_PawnState::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	
	FOnNexusGameplayTagChangedSignature& OnTagAddedDelegate = ActionComponent->GetOnGameplayTagAddedDelegate();
	if (!OnTagAddedDelegate.IsAlreadyBound(this, &UGunnerViewModel_PawnState::OnTagAdded))
	{
		OnTagAddedDelegate.AddDynamic(this, &UGunnerViewModel_PawnState::OnTagAdded);
	}

	FOnNexusGameplayTagChangedSignature& OnTagRemovedDelegate = ActionComponent->GetOnGameplayTagRemovedDelegate();
	if (!OnTagRemovedDelegate.IsAlreadyBound(this, &UGunnerViewModel_PawnState::OnTagRemoved))
	{
		OnTagRemovedDelegate.AddDynamic(this, &UGunnerViewModel_PawnState::OnTagRemoved);
	}

	if (ActionComponent->HasMatchingGameplayTag(GunnerNativeGameplayTags::TAG_State_Dead))
	{
		SetShouldShowPawnRelatedWidget(false);
	}
}

void UGunnerViewModel_PawnState::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	
	FOnNexusGameplayTagChangedSignature& OnTagAddedDelegate = ActionComponent->GetOnGameplayTagAddedDelegate();
	OnTagAddedDelegate.RemoveDynamic(this, &UGunnerViewModel_PawnState::OnTagAdded);
	FOnNexusGameplayTagChangedSignature& OnTagRemovedDelegate = ActionComponent->GetOnGameplayTagRemovedDelegate();
	OnTagRemovedDelegate.RemoveDynamic(this, &UGunnerViewModel_PawnState::OnTagRemoved);
}

void UGunnerViewModel_PawnState::OnTagAdded(const FGameplayTag& AddedTag)
{
	if (AddedTag == GunnerNativeGameplayTags::TAG_State_Dead)
	{
		SetShouldShowPawnRelatedWidget(false);
	}
}

void UGunnerViewModel_PawnState::OnTagRemoved(const FGameplayTag& RemovedTag)
{
	if (RemovedTag == GunnerNativeGameplayTags::TAG_State_Dead)
	{
		SetShouldShowPawnRelatedWidget(true);
	}
}
