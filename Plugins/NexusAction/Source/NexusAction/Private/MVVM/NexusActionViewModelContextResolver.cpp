// Fill out your copyright notice in the Description page of Project Settings.


#include "MVVM/NexusActionViewModelContextResolver.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"


UObject* UNexusActionViewModelContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UNexusActionViewModelBase::CreateInstance(ExpectedType, UserWidget, View);
}

void UNexusActionViewModelContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	Super::DestroyInstance(ViewModel, View);
	CastChecked<UNexusActionViewModelBase>(ViewModel)->OnDestroyViewModel(ViewModel, View);
}

UNexusActionViewModelBase* UNexusActionViewModelBase::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View)
{
	UNexusActionViewModelBase* ViewModel = NewObject<UNexusActionViewModelBase>(UserWidget->GetOwningPlayerState(), ExpectedType);
	check(ViewModel);
	ViewModel->OnCreateViewModel(UserWidget);
	return ViewModel;
}

void UNexusActionViewModelBase::OnCreateViewModel(const UUserWidget* UserWidget)
{
	ActionComponent = UNexusActionComponent::GetActionComponentFromActor(UserWidget->GetOwningPlayerState());
}
