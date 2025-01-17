// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SpawnExtraDummyMagazine.generated.h"

class AStaticMeshActor;
/**
 * 
 */
UCLASS(meta = (DisplayName = "Spawn Extra Dummy Magazine"))
class GUNNER_API UAnimNotifyState_SpawnExtraDummyMagazine : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> StaticMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere)
	FName SocketName = TEXT("Magazine_Extra");

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> ExtraDummyMagazine;

	
	
};
