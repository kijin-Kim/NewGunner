// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/GunnerActionSign.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Actor.h"
#include "Gunner/Action/TargetData/GunnerTargetData_Hit.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GunnerBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetDataHandle MakeHitTargetData(AActor* AgentActor, const TArray<FHitResult>& HitResults)
	{
		TSharedPtr<FGunnerTargetData_Hit> HitData = MakeShared<FGunnerTargetData_Hit>();
		HitData->AgentActor = AgentActor;
		HitData->LocalHitResult = HitResults;

		FGunnerTargetDataHandle Handle;
		Handle.SetData(HitData);

		return Handle;
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Hit GetAsHitTargetData(FGunnerTargetDataHandle Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Hit::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Hit>(Handle.GetData());
		}

		return FGunnerTargetData_Hit();
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetDataHandle MakeActorTargetData(AActor* Actor)
	{
		TSharedPtr<FGunnerTargetData_Actor> ActorData = MakeShared<FGunnerTargetData_Actor>();
		ActorData->Actor = Actor;

		FGunnerTargetDataHandle Handle;
		Handle.SetData(ActorData);

		return Handle;
	}

	UFUNCTION(BlueprintPure, Category = "Gunner|TargetData")
	static FGunnerTargetData_Actor GetAsActorTargetData(FGunnerTargetDataHandle Handle)
	{
		if (Handle.GetData()->GetStructType() == FGunnerTargetData_Actor::StaticStruct())
		{
			return *StaticCastSharedPtr<FGunnerTargetData_Actor>(Handle.GetData());
		}

		return FGunnerTargetData_Actor();
	}
};
