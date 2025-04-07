// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Engine/TargetPoint.h"
#include "GunnerSpawnPointActor.generated.h"


UCLASS()
class GUNNER_API AGunnerSpawnPointActor : public ATargetPoint
{
	GENERATED_BODY()

public:
	AGunnerSpawnPointActor();
#ifdef WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGenericTeamId TeamId = FGenericTeamId::NoTeam;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float SpawnRadius = 500.0f;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class USphereComponent> SphereComponent;
#endif


	
};
