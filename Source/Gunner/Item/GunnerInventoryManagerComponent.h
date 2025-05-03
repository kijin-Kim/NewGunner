// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerInventoryManagerComponent.generated.h"

class UGunnerItemInstance;
class UGunnerItemDef;
class AGunnerItem;
class UGunnerAction_DropSlotItem;
struct FNexusEventMessage;
class UNexusActionComponent;
class UNexusAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerItemAcquiredSignature, AGunnerItem*, Item);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerItemRemovedSiganture, AGunnerItem*, Item);



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerInventoryManagerComponent();
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UGunnerInventoryManagerComponent* GetInventoryManagerComponentFromActor(const AActor* Actor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool CanAcquireItem(AGunnerItem* Item) const;
	void AuthAddItem(AGunnerItem* Item);
	void AuthRemoveItem(AGunnerItem* Item, bool bDestroyItem = true);


	const TArray<AGunnerItem*>& GetInventoryItems() const { return Items; }
	bool HasItem(AGunnerItem* Item) const;

protected:
	void OnItemAcquired(AGunnerItem* Item);
	void OnItemRemoved(AGunnerItem* Item);

	AActor* GetAgentActorChecked() const;

private:
	UFUNCTION()
	void OnRep_Items(const TArray<AGunnerItem*>& OldItems);

public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AGunnerItem>> StartItemClasses;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<const UGunnerItemDef>> StartItemDefs;


	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerAction_DropSlotItem> DropSlotItemActionClass;

	UPROPERTY(BlueprintAssignable)
	FOnGunnerItemAcquiredSignature OnItemAcquiredDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnGunnerItemRemovedSiganture OnItemRemovedDelegate;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UUserWidget>> InventoryWidgetClasses;
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> InventoryWidgets;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<TObjectPtr<AGunnerItem>> Items;

	UPROPERTY()
	TArray<TObjectPtr<AGunnerItem>> PendingAdds;
	UPROPERTY()
	TArray<TObjectPtr<AGunnerItem>> PendingRemoves;
	

};
