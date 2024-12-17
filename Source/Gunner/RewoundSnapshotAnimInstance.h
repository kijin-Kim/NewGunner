// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RewoundSnapshotAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API URewoundSnapshotAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetBlendAlpha(float InBlendAlpha) { BlendAlpha = InBlendAlpha; }

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float BlendAlpha;
};
