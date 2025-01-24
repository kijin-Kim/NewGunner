// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_HideMagazine.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Hide Magazine"))
class GUNNER_API UAnimNotifyState_HideMagazine : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere)
	FName MagazineSocketName = TEXT("Magazine");

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Magazine;
	
};
