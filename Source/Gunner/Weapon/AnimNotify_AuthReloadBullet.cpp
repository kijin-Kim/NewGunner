// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AuthReloadBullet.h"

void UAnimNotify_AuthReloadBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
}
