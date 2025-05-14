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
#include "UI/Minimap/GunnerMiniMapData.h"

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

void UGunnerBlueprintFunctionLibrary::ImportGeometryFromJson(const FString& FilePath, UGunnerMapGeometryData* TargetData)
{
	if (!TargetData) return;

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogGunner, Error, TEXT("파일 읽기 실패: %s"), *FilePath);
		return;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogGunner, Error, TEXT("JSON 파싱 실패: %s"), *FilePath);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* GroupsJson;
	if (!Root->TryGetArrayField(TEXT("Groups"), GroupsJson))
	{
		UE_LOG(LogGunner, Error, TEXT("Groups 배열이 없음: %s"), *FilePath);
		return;
	}

	TargetData->Groups.Empty();

	for (const auto& GroupVal : *GroupsJson)
	{
		const TSharedPtr<FJsonObject> GroupObj = GroupVal->AsObject();
		if (!GroupObj) continue;

		FGunnerGeometryGroup Group;
		Group.ZHeight = GroupObj->GetNumberField(TEXT("ZHeight"));

		const TArray<TSharedPtr<FJsonValue>>* VerticesJson;
		if (GroupObj->TryGetArrayField(TEXT("Vertices"), VerticesJson))
		{
			for (const auto& V : *VerticesJson)
			{
				const TSharedPtr<FJsonObject> VObj = V->AsObject();
				FGunnerGeometryVertex Vertex;
				Vertex.X = VObj->GetNumberField(TEXT("X"));
				Vertex.Y = VObj->GetNumberField(TEXT("Y"));
				Group.Vertices.Add(Vertex);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* LinesJson;
		if (GroupObj->TryGetArrayField(TEXT("Lines"), LinesJson))
		{
			for (const auto& L : *LinesJson)
			{
				const TSharedPtr<FJsonObject> LObj = L->AsObject();
				FGunnerGeometryLine Line;
				Line.Start = LObj->GetIntegerField(TEXT("Start"));
				Line.End = LObj->GetIntegerField(TEXT("End"));
				Group.Lines.Add(Line);
			}
		}

		TargetData->Groups.Add(Group);
	}

	UE_LOG(LogGunner, Display, TEXT("임포트 완료: GroupCount=%d, TargetData=%s"), TargetData->Groups.Num(), *TargetData->GetName());
}

double UGunnerBlueprintFunctionLibrary::GetSlateApplicationTime()
{
	return FSlateApplicationBase::Get().GetCurrentTime();
}
