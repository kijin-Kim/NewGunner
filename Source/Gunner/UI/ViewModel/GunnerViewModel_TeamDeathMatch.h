// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "MVVMViewModelBase.h"
#include "View/MVVMViewModelContextResolver.h"
#include "GunnerViewModel_TeamDeathMatch.generated.h"

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
class GUNNER_API UGunnerViewModel_TeamDeathMatch : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	static UGunnerViewModel_TeamDeathMatch* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View);
	virtual void OnCreateViewModel(const UUserWidget* UserWidget);
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const;

private:
	void OnTeamKillCountChanged(FGenericTeamId TeamID, int Count);

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FText AllyTeamKillCountText = FText::AsNumber(0);
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FText EnemyTeamKillCountText = FText::AsNumber(0);

private:
	FGenericTeamId OwnerTeamID;
};

