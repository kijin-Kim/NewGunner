// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AuthReloadBullet.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Auth Reload Bullet"))
class GUNNER_API UAnimNotify_AuthReloadBullet : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
