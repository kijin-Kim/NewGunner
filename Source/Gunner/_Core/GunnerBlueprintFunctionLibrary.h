// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "NexusCue.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Actor.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Hit.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GunnerBlueprintFunctionLibrary.generated.h"

struct FTeamBoxSlot;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FNexusRepDataHandle MakeHitTargetData(AActor* AgentActor, const TArray<FHitResult>& HitResults)
	{
		TSharedPtr<FGunnerTargetData_Hit> HitData = MakeShared<FGunnerTargetData_Hit>();
		HitData->AgentActor = AgentActor;
		HitData->LocalHitResult = HitResults;

		FNexusRepDataHandle Handle;
		Handle.SetData(HitData);

		return Handle;
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Hit GetAsHitTargetData(FNexusRepDataHandle Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Hit::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Hit>(Handle.GetData());
		}

		return FGunnerTargetData_Hit();
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FNexusRepDataHandle MakeActorTargetData(AActor* Actor)
	{
		TSharedPtr<FGunnerTargetData_Actor> ActorData = MakeShared<FGunnerTargetData_Actor>();
		ActorData->Actor = Actor;

		FNexusRepDataHandle Handle;
		Handle.SetData(ActorData);

		return Handle;
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Actor GetAsActorTargetData(FNexusRepDataHandle Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Actor::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Actor>(Handle.GetData());
		}

		return FGunnerTargetData_Actor();
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|Lobby")
	static bool IsTeamBoxSlotValid(const FTeamBoxSlot& Slot);

	UFUNCTION(BlueprintCallable)
	static void ServerTravelBySoftObjectPtr(const UObject* WorldContextObject, const TSoftObjectPtr<UWorld> Level, bool bAbsolute, FString Options);


	UFUNCTION(BlueprintPure, Category = "Gunner|Team")
	static ETeamAttitude::Type GetTeamAttitude(APlayerState* PlayerState, APlayerState* OtherPlayerState);
};
