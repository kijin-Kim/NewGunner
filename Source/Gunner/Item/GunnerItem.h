// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionDefHandle.h"
#include "GameFramework/Actor.h"
#include "GunnerItem.generated.h"

struct FGunnerItemActionEntry;
class UGunnerItemDef;
class UNexusAction;
class UGunnerSlotItemUiData;

DECLARE_DELEGATE_OneParam(FOnGunnerItemDefReplicatedSignature, class AGunnerItem* /*Item*/);

UCLASS()
class GUNNER_API AGunnerItem : public AActor
{
	GENERATED_BODY()

public:
	struct FGunnerInventoryDisplayDebugString
	{
		FString DisplayDebugString;
		FColor Color = FColor::White;
	};

public:
	AGunnerItem();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeItem(const UGunnerItemDef* InItemDef);


	virtual bool CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const;
	virtual void OnAcquired(AActor* InAgentActor);

	virtual void PostOnAcquired()
	{
	}

	virtual void OnRemoved();

	const UGunnerItemDef* GetItemDef() const { return ItemDef; }
	FString ToString() const { return FString::Printf(TEXT("Item={Name=%s}"), *GetName()); }


	virtual void CollectDisplayDebugString(TArray<FGunnerInventoryDisplayDebugString>& OutDisplayDebugstrings) const
	{
		
	}

protected:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles);

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> PersistentActivationActions;
	TArray<FNexusActionDefHandle> PersistentActivationActionHandles;

	UPROPERTY()
	TObjectPtr<AActor> AgentActor;


	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UGunnerItemDef> ItemDef;
};
