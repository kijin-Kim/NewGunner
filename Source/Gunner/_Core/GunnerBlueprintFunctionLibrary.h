// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Action/NexusActionComponent.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Actor.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Hit.h"
#include "Gunner/Action/TargetData/GunnerTargetData_SoundBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Minimap/GunnerMiniMapData.h"
#include "GunnerBlueprintFunctionLibrary.generated.h"

class UGunnerItemDef;
class AGunnerItem;
class UGunnerActionSet;
struct FTeamBoxSlot;


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|TargetData")
	static FNexusTargetDataHandle MakeHitTargetData(AActor* AgentActor, const TArray<FHitResult>& HitResults)
	{
		TSharedPtr<FGunnerTargetData_Hit> HitData = MakeShared<FGunnerTargetData_Hit>();
		HitData->AgentActor = AgentActor;
		HitData->HitResults = HitResults;

		FNexusTargetDataHandle Handle;
		Handle.SetData(HitData);

		return Handle;
	}

	UFUNCTION(BlueprintCallable, Category = "Gunner|TargetData")
	static FNexusTargetDataHandle MakeActorTargetData(AActor* Actor)
	{
		TSharedPtr<FGunnerTargetData_Actor> ActorData = MakeShared<FGunnerTargetData_Actor>();
		ActorData->Actor = Actor;

		FNexusTargetDataHandle Handle;
		Handle.SetData(ActorData);

		return Handle;
	}

	UFUNCTION(BlueprintCallable, Category = "Gunner|TargetData")
	static FNexusTargetDataHandle MakeSoundBaseTargetData(USoundBase* Sound, USceneComponent* ContextComponent, FName ContextBoneName = NAME_None)
	{
		if (!Sound || !ContextComponent)
		{
			return FNexusTargetDataHandle();
		}

		TSharedPtr<FGunnerTargetData_SoundBase> SoundData = MakeShared<FGunnerTargetData_SoundBase>();
		SoundData->SoundBase = Sound;
		SoundData->ContextComponent = ContextComponent;
		SoundData->ContextBoneName = ContextBoneName;

		FNexusTargetDataHandle Handle;
		Handle.SetData(SoundData);

		return Handle;
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Hit GetAsHitTargetData(const FNexusTargetDataHandle& Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Hit::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Hit>(Handle.GetData());
		}

		return FGunnerTargetData_Hit();
	}


	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Actor GetAsActorTargetData(const FNexusTargetDataHandle& Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Actor::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Actor>(Handle.GetData());
		}

		return FGunnerTargetData_Actor();
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_SoundBase GetAsSoundBaseTargetData(const FNexusTargetDataHandle& Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_SoundBase::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_SoundBase>(Handle.GetData());
		}

		return FGunnerTargetData_SoundBase();
	}


	UFUNCTION(BlueprintPure, Category = "Gunner|Lobby")
	static bool IsTeamBoxSlotValid(const FTeamBoxSlot& Slot);

	UFUNCTION(BlueprintCallable)
	static void ServerTravelBySoftObjectPtr(const UObject* WorldContextObject, const TSoftObjectPtr<UWorld> Level, bool bAbsolute, FString Options);


	UFUNCTION(BlueprintPure, Category = "Gunner|Team")
	static ETeamAttitude::Type GetTeamAttitude(APlayerState* PlayerState, APlayerState* OtherPlayerState);

	UFUNCTION(BlueprintPure, Category = "Gunner|Team")
	static FGenericTeamId GetTeamId(APlayerState* PlayerState);


	UFUNCTION(BlueprintCallable, Category = "Gunner|Action")
	static void AuthAddDesiredActions(AActor* Actor, UObject* SourceObject, const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& OutAddedActionHandles);
	UFUNCTION(BlueprintCallable, Category = "Gunner|Action")
	static void AuthRemoveDesiredActions(AActor* Actor, const TArray<FNexusActionDefHandle>& AddedActionHandles);

	UFUNCTION(BlueprintCallable, Category = "Gunner|Action")
	static void AuthAddDesiredItems(AActor* Actor, const TArray<UGunnerItemDef*>& ItemDefs, TArray<AGunnerItem*>& OutAddedItems);
	UFUNCTION(BlueprintCallable, Category = "Gunner|Action")
	static void AuthRemoveDesiredItems(AActor* Actor, const TArray<AGunnerItem*>& ItemsToRemove, bool bDestroyItem = true);

	UFUNCTION(CallInEditor, BlueprintCallable)
	static void ImportGeometryFromJson(const FString& FilePath, UGunnerMapGeometryData* TargetData);

	UFUNCTION(BlueprintPure, Category = "Gunner|Slate")
	static double GetSlateApplicationTime();
};
