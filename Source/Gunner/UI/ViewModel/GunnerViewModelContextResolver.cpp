// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModelContextResolver.h"

#include "Blueprint/UserWidget.h"

UObject* UGunnerViewModelContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UGunnerViewModelBase::CreateInstance(ExpectedType, UserWidget, View);
}

void UGunnerViewModelContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	Super::DestroyInstance(ViewModel, View);
	CastChecked<UGunnerViewModelBase>(ViewModel)->OnDestroyViewModel(ViewModel, View);
}

UGunnerViewModelBase* UGunnerViewModelBase::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View)
{
	UGunnerViewModelBase* ViewModel = NewObject<UGunnerViewModelBase>(UserWidget->GetOwningPlayer(), ExpectedType);
	check(ViewModel);
	ViewModel->OnCreateViewModel(UserWidget);
	return ViewModel;
}

void UGunnerViewModelBase::OnCreateViewModel(const UUserWidget* UserWidget)
{
}

void UGunnerViewModelBase::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
}
