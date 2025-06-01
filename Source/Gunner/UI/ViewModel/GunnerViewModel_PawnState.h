// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerViewModel_PawnState.generated.h"


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerViewModel_PawnState : public UNexusActionViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

	void SetShouldShowPawnRelatedWidget(bool bInShouldShowPawnRelatedWidget)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bShouldShowPawnRelatedWidget, bInShouldShowPawnRelatedWidget))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bShouldShowPawnRelatedWidget);
		}
	}

	
	
private:
	UFUNCTION()
	void OnTagAdded(const FGameplayTag& GameplayTag);
	UFUNCTION()
	void OnTagRemoved(const FGameplayTag& GameplayTag);

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	bool bShouldShowPawnRelatedWidget = true;
};
