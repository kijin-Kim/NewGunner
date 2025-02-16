// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/NexusAsync_PlayMontage.h"

#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"

UNexusAsync_PlayMontage* UNexusAsync_PlayMontage::PlayMontage(UNexusAction* InAction, AActor* InMontageActor, UAnimMontage* InMontageToPlay, bool InbIsThirdPerson, float InPlayRate, FName InStartSectionName)
{
	UNexusAsync_PlayMontage* SelfObject = NewNexusAsync<UNexusAsync_PlayMontage>(InAction);
	if (!InMontageActor || !InMontageToPlay)
	{
		return nullptr;
	}

	SelfObject->MontageActor = InMontageActor;
	SelfObject->MontageToPlay = InMontageToPlay;
	SelfObject->bIsThirdPerson = InbIsThirdPerson;

	SelfObject->PlayRate = InPlayRate;
	SelfObject->StartSectionName = InStartSectionName;
	SelfObject->RegisterWithGameInstance(SelfObject->MontageActor->GetWorld());

	return SelfObject;
}

void UNexusAsync_PlayMontage::Activate()
{
	Super::Activate();

	if (!MontageActor.IsValid() || !MontageToPlay.IsValid() || !MontageActor.Get()->Implements<UNexusAnimMontagePlayerInterface>())
	{
		Cancel();
		return;
	}


	UNexusAnimMontagePlayerComponent* MontagePlayerComponent = INexusAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(MontageActor.Get());
	MontagePlayerComponent->PlayMontage(MontageToPlay.Get(), bIsThirdPerson, PlayRate, StartSectionName);
	if (OnCompletedDelegate.IsBound() || OnInterruptedDelegate.IsBound() || OnBlendOutDelegate.IsBound())
	{
		FOnMontageEnded* MontageEnded = MontagePlayerComponent->GetMontageEndedDelegate(MontageToPlay.Get(), bIsThirdPerson);
		check(MontageEnded);
		MontageEnded->BindWeakLambda(this, [this](UAnimMontage* AnimMontage, bool bInterrupted)
		{
			if (ShouldBroadcastDelegates())
			{
				if (bInterrupted)
				{
					OnInterruptedDelegate.Broadcast();
				}
				else
				{
					OnCompletedDelegate.Broadcast();
				}
			}
		});
		FOnMontageBlendingOutStarted* OnMontageBlendingOutStarted = MontagePlayerComponent->GetMontageBlendingOutStartedDelegate(MontageToPlay.Get(), bIsThirdPerson);
		check(OnMontageBlendingOutStarted);
		OnMontageBlendingOutStarted->BindWeakLambda(this, [this](UAnimMontage* AnimMontage, bool bInterrupted)
		{
			if (ShouldBroadcastDelegates())
			{
				if (bInterrupted)
				{
					OnInterruptedDelegate.Broadcast();
				}
				else
				{
					OnBlendOutDelegate.Broadcast();
				}
			}
		});
	}
}
