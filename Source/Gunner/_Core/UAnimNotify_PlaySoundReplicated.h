// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "UAnimNotify_PlaySoundReplicated.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Play Sound Replicated"))
class GUNNER_API UUAnimNotify_PlaySoundReplicated : public UAnimNotify_PlaySound
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere)
	bool bPlaySoundOnNonLocallyControlled = true;
	UPROPERTY(EditAnywhere)
	bool bSpatializeOnLocallyControlled = false;
};
