// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerSlotItem.h"
#include "Action/NexusAction.h"
#include "SideEffect/NexusSideEffect.h"
#include "GunnerAction_EquipBase.generated.h"

class AGunnerEquipmentItem;
class UNexusAnimMontagePlayerInterface;

UCLASS()
class GUNNER_API UGunnerSlotIndexChangeSideEffect : public UNexusSideEffect
{
	GENERATED_BODY()

public:
	UGunnerSlotIndexChangeSideEffect();
};


UCLASS(Abstract)
class GUNNER_API UGunnerAction_EquipBase : public UNexusAction
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

protected:
	UGunnerAction_EquipBase();
	virtual void OnAddAction() override;
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;
	virtual void OnEndAction() override;
	virtual void OnRemoveAction() override;

private:
	void RemoveEquippedWidgetsFromParent();
	EGunnerSlotType GetCurrentSlotType() const;

protected:

	UPROPERTY(EditAnywhere, Category = "Action")
	TArray<TSubclassOf<UNexusAction>> EquippedActions;
	TArray<FNexusActionDefHandle> EquippedActionDefHandles;
	UPROPERTY(EditAnywhere, Category = "UI")
	TArray<TSubclassOf<UUserWidget>> EquippedWidgetClasses;
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> EquippedWidgetMap;
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	TObjectPtr<AGunnerEquipmentItem> EquipmentItem;
};
