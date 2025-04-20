// Fill out your copyright notice in the Description page of Project Settings.


#include "MVVM/NexusActionViewModelContextResolver.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"


UObject* UNexusActionViewModelContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UNexusActionViewModel::CreateInstance(ExpectedType, UserWidget, View);
}

void UNexusActionViewModelContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	Super::DestroyInstance(ViewModel, View);
	CastChecked<UNexusActionViewModel>(ViewModel)->OnDestroyViewModel(ViewModel, View);
}

UNexusActionViewModel* UNexusActionViewModel::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View)
{
	UNexusActionViewModel* ViewModel = NewObject<UNexusActionViewModel>(UserWidget->GetOwningPlayerPawn(), ExpectedType);
	check(ViewModel);
	ViewModel->OnCreateViewModel(UserWidget);
	return ViewModel;
}

void UNexusActionViewModel::OnCreateViewModel(const UUserWidget* UserWidget)
{
	ActionComponent = UNexusActionComponent::GetActionComponentFromActor(UserWidget->GetOwningPlayerPawn());
}
