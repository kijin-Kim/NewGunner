// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionDefHandle.h"
#include "GameFramework/Actor.h"
#include "GunnerSlotItem.generated.h"

class UNexusActionComponent;
class UNexusAction;

UENUM(BlueprintType)
enum class EGunnerSlotType : uint8
{
	Primary = 0,
	Secondary,
	Melee,
	Spike,
	BasicFirst,
	BasicSecond,
	Signature,
	Ultimate,
	Num UMETA(Hidden),
};


UCLASS()
class GUNNER_API AGunnerSlotItem : public AActor
{
	GENERATED_BODY()

public:
	AGunnerSlotItem();
	virtual void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	virtual void OnAcquired(AActor* AgentActor);
	virtual void OnRemoved(AActor* AgentActor);
	virtual void OnActivated(AActor* AgentActor);
	virtual void OnDeactivated(AActor* AgentActor);

	EGunnerSlotType GetSlotType() const { return SlotType; }

	UNexusActionComponent* GetActionComponent(AActor* AgentActor) const;

private:
	void AuthAddDesiredActions(AActor* AgentActor, const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(AActor* AgentActor, TArray<FNexusActionDefHandle>& AddedActionHandles);

protected:
	UPROPERTY(EditAnywhere)
	EGunnerSlotType SlotType = EGunnerSlotType::Num;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> PersistentActivationActions;
	TArray<FNexusActionDefHandle> PersistentActivationActionHandles;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> TransientActivationActions;
	TArray<FNexusActionDefHandle> TransientActivationActionHandles;
};


