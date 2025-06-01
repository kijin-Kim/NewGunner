// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "View/MVVMViewModelContextResolver.h"
#include "GunnerViewModelContextResolver.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerViewModelContextResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual void DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const override;
};

UCLASS()
class GUNNER_API UGunnerViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	static UGunnerViewModelBase* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View);
	virtual void OnCreateViewModel(const UUserWidget* UserWidget);
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const;
};

