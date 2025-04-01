// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionDefHandle.h"
#include "Components/ActorComponent.h"
#include "Gunner/Action/GunnerActionSlotItemBase.h"
#include "GunnerSlotManagerComponent.generated.h"

struct FNexusEventMessage;
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

	virtual void OnAcquired()
	{
	};

	virtual void OnRemoved()
	{
	};

	virtual void OnActivated()
	{
		bIsActivated = true;
	};

	virtual void OnDeactivated()
	{
		bIsActivated = false;
	};

	void AuthAddPersistentActivationActions();
	void AuthAddTransientActivationActions();
	void AuthRemovePersistentActivationActions();
	void AuthRemoveTransientActivationActions();

	EGunnerSlotType GetSlotType() const { return SlotType; }

	UNexusActionComponent* GetActionComponent() const;
	bool IsActivated() const { return bIsActivated; }

private:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles);

private:
	UPROPERTY(EditAnywhere)
	EGunnerSlotType SlotType = EGunnerSlotType::Num;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> PersistentActivationActions;
	TArray<FNexusActionDefHandle> PersistentActivationActionHandles;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> TransientActivationActions;
	TArray<FNexusActionDefHandle> TransientActivationActionHandles;

	bool bIsActivated = false;
};


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

	void AuthAddItemToSlot(AGunnerSlotItem* Item);
	void AuthRemoveItemFromSlot(AGunnerSlotItem* Item, bool bDestroyItem = true);


	// 서버에게 클라이언트에 슬롯 아이템의 인스턴스가 리플리케이트된 것을 알립니다 
	UFUNCTION(Server, Reliable)
	void ServerAckNewItem(AGunnerSlotItem* Item);
	UFUNCTION()
	void OnSlotIndexChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable)
	AGunnerSlotItem* GetCurrentSlotItem() const;
	EGunnerSlotType GetCurrentSlotType() const;

private:
	void ActivateSlot(AGunnerSlotItem* Item);
	void DeactivateSlot(AGunnerSlotItem* Item);

	UFUNCTION()
	void OnRep_SlotItems(const TArray<AGunnerSlotItem*>& OldItems);

private:
	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EGunnerSlotType"))
	TSubclassOf<AGunnerSlotItem> StartItemClasses[static_cast<uint8>(EGunnerSlotType::Num)];

	UPROPERTY(ReplicatedUsing = OnRep_SlotItems)
	TArray<TObjectPtr<AGunnerSlotItem>> SlotItems;
};

UCLASS()
class UGunnerActionSlotActivation : public UGunnerActionSlotItemBase
{
	GENERATED_BODY()
	
protected:
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	EGunnerSlotType GetCurrentSlotType() const;
};
