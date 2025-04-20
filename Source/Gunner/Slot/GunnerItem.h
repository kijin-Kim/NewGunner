// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionDefHandle.h"
#include "GameFramework/Actor.h"
#include "GunnerItem.generated.h"

class UNexusAction;
class UGunnerSlotItemUiData;

UCLASS()
class GUNNER_API AGunnerItem : public AActor
{
	GENERATED_BODY()

public:
	AGunnerItem();
	
	virtual void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y) {}
	virtual bool CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const;
	virtual void OnAcquired(AActor* InAgentActor);
	virtual void PostOnAcquired() {}
	virtual void OnRemoved();
	
	const UGunnerSlotItemUiData* GetSlotItemUIData() const { return SlotItemUIData; }

protected:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles);

	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> PersistentActivationActions;
	TArray<FNexusActionDefHandle> PersistentActivationActionHandles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<const UGunnerSlotItemUiData> SlotItemUIData;

	UPROPERTY()
	TObjectPtr<AActor> AgentActor;
};
