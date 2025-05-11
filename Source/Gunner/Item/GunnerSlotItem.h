// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerItem.h"
#include "Action/NexusActionDefHandle.h"
#include "GameFramework/Actor.h"
#include "GunnerSlotItem.generated.h"


class UNexusProperty;
class UGunnerSlotItemUiData;
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
class GUNNER_API AGunnerSlotItem : public AGunnerItem
{
	GENERATED_BODY()

public:
	virtual void CollectDisplayDebugString(TArray<FGunnerInventoryDisplayDebugString>& OutDisplayDebugstrings) const override;
	virtual bool CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const override;

	virtual void PostOnAcquired() override;
	virtual void OnRemoved() override;

	

	EGunnerSlotType GetSlotType() const { return SlotType; }

protected:
	virtual void OnActivated();
	virtual void OnDeactivated();

private:
	UFUNCTION()
	void HandleSlotIndexDirty(float OldValue, float NewValue);
	UNexusProperty* GetSlotIndexProperty() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGunnerSlotType SlotType = EGunnerSlotType::Num;
};
