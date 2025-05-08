// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerBlueprintFunctionLibrary.h"

#include "GunnerActionComponent.h"
#include "GunnerLobbyGameState.h"
#include "Action/NexusAction.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerItem.h"
#include "Gunner/Item/GunnerItemDef.h"

bool UGunnerBlueprintFunctionLibrary::IsTeamBoxSlotValid(const FTeamBoxSlot& Slot)
{
	return Slot.IsValid();
}

void UGunnerBlueprintFunctionLibrary::ServerTravelBySoftObjectPtr(const UObject* WorldContextObject, const TSoftObjectPtr<UWorld> Level, bool bAbsolute, FString Options)
{
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FString URL = LevelName.ToString();
	URL += FString::Printf(TEXT("?%s"), *Options);
	World->ServerTravel(URL, bAbsolute);
}

ETeamAttitude::Type UGunnerBlueprintFunctionLibrary::GetTeamAttitude(APlayerState* PlayerState, APlayerState* OtherPlayerState)
{
	if (!PlayerState || !OtherPlayerState)
	{
		return ETeamAttitude::Neutral;
	}

	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(PlayerState);
	if (!TeamAgentInterface)
	{
		return ETeamAttitude::Neutral;
	}

	return TeamAgentInterface->GetTeamAttitudeTowards(*OtherPlayerState);
}

FGenericTeamId UGunnerBlueprintFunctionLibrary::GetTeamId(APlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return FGenericTeamId();
	}

	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(PlayerState);
	if (!TeamAgentInterface)
	{
		return FGenericTeamId();
	}

	return TeamAgentInterface->GetGenericTeamId();
}

void UGunnerBlueprintFunctionLibrary::AuthAddDesiredActions(AActor* Actor, UObject* SourceObject, const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& OutAddedActionHandles)
{
	if (!Actor)
	{
		return;
	}

	if (!Actor->HasAuthority())
	{
		GR_LOG_SUB(Actor, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Actor);
	check(ActionComponent);

	for (TSubclassOf<UNexusAction> ActionClass : ActionsToAdd)
	{
		if (ActionClass)
		{
			FNexusActionDefHandle AddedHandle = ActionComponent->AuthAddAction(ActionClass, SourceObject);
			OutAddedActionHandles.Add(AddedHandle);
		}
	}
}


void UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredActions(AActor* Actor, const TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	if (!Actor)
	{
		return;
	}

	if (!Actor->HasAuthority())
	{
		GR_LOG_SUB(Actor, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Actor);
	if (ActionComponent)
	{
		for (const FNexusActionDefHandle& ActionHandle : AddedActionHandles)
		{
			ActionComponent->AuthRemoveAction(ActionHandle);
		}
	}
}

void UGunnerBlueprintFunctionLibrary::AuthAddDesiredItems(AActor* Actor, const TArray<UGunnerItemDef*>& ItemDefs, TArray<AGunnerItem*>& OutAddedItems)
{
	if (!Actor)
	{
		return;
	}

	if (!Actor->HasAuthority())
	{
		GR_LOG_SUB(Actor, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(Actor);
	if (!InventoryManagerComponent)
	{
		return;
	}

	for (UGunnerItemDef* ItemDef : ItemDefs)
	{
		if (ItemDef && ItemDef->ItemClass)
		{
			AGunnerItem* NewItem = Actor->GetWorld()->SpawnActorDeferred<AGunnerItem>(ItemDef->ItemClass, FTransform::Identity);
			check(NewItem);
			NewItem->InitializeItem(ItemDef);
			NewItem->FinishSpawning(FTransform::Identity);
			OutAddedItems.AddUnique(NewItem);
			InventoryManagerComponent->AuthAddItem(NewItem);
		}
	}
}

void UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredItems(AActor* Actor, const TArray<AGunnerItem*>& ItemsToRemove, bool bDestroyItem)
{
	if (!Actor)
	{
		return;
	}

	if (!Actor->HasAuthority())
	{
		GR_LOG_SUB(Actor, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(Actor);
	if (!InventoryManagerComponent)
	{
		return;
	}
	
	for (AGunnerItem* Item : ItemsToRemove)
	{
		if (Item)
		{
			InventoryManagerComponent->AuthRemoveItem(Item, bDestroyItem);
		}
	}
}
