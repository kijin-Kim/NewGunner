// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GunnerViewModelContextResolver.h"
#include "View/MVVMViewModelContextResolver.h"
#include "GunnerViewModel_TeamDeathMatch.generated.h"

/**
 * 
 */


UCLASS()
class GUNNER_API UGunnerViewModel_TeamDeathMatch : public UGunnerViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

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

