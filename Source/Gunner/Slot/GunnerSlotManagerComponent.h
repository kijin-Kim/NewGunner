// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerSlotItem.h"
#include "Components/ActorComponent.h"
#include "Gunner/Action/GunnerActionSlotItemBase.h"
#include "SideEffect/NexusSideEffect.h"
#include "GunnerSlotManagerComponent.generated.h"

class UGunnerActionDropSlotItem;
struct FNexusEventMessage;
class UNexusActionComponent;
class UNexusAction;





UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerSlotManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerSlotManagerComponent();
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UGunnerSlotManagerComponent* GetSlotManagerComponentFromActor(const AActor* Actor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AuthAddItemToSlot(AGunnerSlotItem* Item);
	void AuthRemoveItemFromSlot(AGunnerSlotItem* Item, bool bDestroyItem = true);


	UFUNCTION(BlueprintCallable)
	AGunnerSlotItem* GetCurrentSlotItem() const;
	EGunnerSlotType GetCurrentSlotType() const;

	bool IsSlotEmpty(EGunnerSlotType SlotType) const;

	EGunnerSlotType FindActivableSlotType() const;
	AGunnerSlotItem* GetSlotItemByType(EGunnerSlotType SlotType) const;

protected:
	void OnItemAcquired(AGunnerSlotItem* Item);
	void OnItemRemoved(AGunnerSlotItem* Item);
	void OnItemActivated(AGunnerSlotItem* Item);
	void OnItemDeactivated(AGunnerSlotItem* Item);

private:
	UFUNCTION()
	void OnRep_SlotItemInstances(const TArray<AGunnerSlotItem*>& OldItems);
	UFUNCTION()
	void HandleSlotIndexDirty(float OldValue, float NewValue);

public:
	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EGunnerSlotType"))
	TSubclassOf<AGunnerSlotItem> StartItemClasses[static_cast<uint8>(EGunnerSlotType::Num)];

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerActionDropSlotItem> DropSlotItemActionClass;

private:
	UPROPERTY(ReplicatedUsing = OnRep_SlotItemInstances)
	TArray<TObjectPtr<AGunnerSlotItem>> SlotItemInstances;
};

UCLASS()
class GUNNER_API UGunnerSlotIndexChangeSideEffect : public UNexusSideEffect
{
	GENERATED_BODY()

public:
	UGunnerSlotIndexChangeSideEffect();
};


UCLASS()
class GUNNER_API UGunnerActionSlotActivation : public UGunnerActionSlotItemBase
{
	GENERATED_BODY()

protected:
	UGunnerActionSlotActivation();
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	EGunnerSlotType GetCurrentSlotType() const;
};


