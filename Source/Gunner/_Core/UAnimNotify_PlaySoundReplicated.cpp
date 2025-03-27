// Fill out your copyright notice in the Description page of Project Settings.


#include "UAnimNotify_PlaySoundReplicated.h"

#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"

void UUAnimNotify_PlaySoundReplicated::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	APawn* PawnOwner = Cast<APawn>(MeshComp->GetOwner());
	if (!PawnOwner)
	{
		Super::Notify(MeshComp, Animation, EventReference);
		return;
	}
	
	if (!bPlaySoundOnNonLocallyControlled && !PawnOwner->IsLocallyControlled())
	{
		return;
	}

	// Don't call super to avoid call back in to blueprints
	if (Sound && MeshComp)
	{
		if (!Sound->IsOneShot())
		{
			UE_LOG(LogGunner, Verbose, TEXT("사운드 [%s]은 일회성이 아닙니다"), *Sound->GetName());
			return;
		}

		UWorld* World = MeshComp->GetWorld();
#if WITH_EDITORONLY_DATA
		if (bPreviewIgnoreAttenuation && World && World->WorldType == EWorldType::EditorPreview)
		{
			UGameplayStatics::PlaySound2D(World, Sound, VolumeMultiplier, PitchMultiplier);
		}
		else
#endif
		{
			if (!bSpatializeOnLocallyControlled && PawnOwner->IsLocallyControlled())
			{
				UGameplayStatics::PlaySound2D(World, Sound, VolumeMultiplier, PitchMultiplier);
				return;
			}

			if (bFollow)
			{
				UGameplayStatics::SpawnSoundAttached(Sound, MeshComp, AttachName, FVector(ForceInit), EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), Sound, MeshComp->GetComponentLocation(), VolumeMultiplier, PitchMultiplier);
			}
		}
	}
}

