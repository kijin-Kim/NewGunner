// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SpawnDummyMagazine.generated.h"

class AStaticMeshActor;
/**
 * 
 */
UCLASS(meta = (DisplayName = "Spawn Dummy Magazine"))
class GUNNER_API UAnimNotifyState_SpawnDummyMagazine : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;



	
public:
	UPROPERTY(EditAnywhere)
	FName SocketName = TEXT("Magazine");
	
private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> DummyMagazine;
	
};
