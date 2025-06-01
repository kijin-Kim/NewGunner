// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerView_HitIndicatorOverlay.h"

#include "GunnerView_HitIndicator.h"
#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "View/MVVMView.h"


void UGunnerView_HitIndicatorOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	APawn* OwningPawn = GetOwningPlayerPawn();
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(OwningPawn);
	if (OwningPawn && ActionComponent)
	{
		EventCallbackHandle = ActionComponent->BindEventCallback<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_GameEvent_Damaged, this, &UGunnerView_HitIndicatorOverlay::OnDamagedEvent);
	}
}

void UGunnerView_HitIndicatorOverlay::NativeDestruct()
{
	Super::NativeDestruct();
	APawn* OwningPawn = GetOwningPlayerPawn();
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(OwningPawn);
	if (OwningPawn && ActionComponent)
	{
		ActionComponent->UnbindEventCallback(EventCallbackHandle);
	}
}

void UGunnerView_HitIndicatorOverlay::OnDamagedEvent(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	if (HitIndicatorWidgetClass)
	{
		UGunnerView_HitIndicator* HitIndicatorOverlay = CreateWidget<UGunnerView_HitIndicator>(GetOwningPlayer(), HitIndicatorWidgetClass);
		HitIndicatorOverlay->AddToViewport();
		UGunnerViewModel_HitIndicator* HitIndicatorViewModel = NewObject<UGunnerViewModel_HitIndicator>(GetOwningPlayer(), UGunnerViewModel_HitIndicator::StaticClass());
		check(HitIndicatorViewModel);
		HitIndicatorViewModel->SetCauserLocation(EventMessage.Instigator->GetActorLocation());
		HitIndicatorViewModel->OnCreateViewModel(HitIndicatorOverlay);

		if (UMVVMView* View = Cast<UMVVMView>(HitIndicatorOverlay->GetExtension(UMVVMView::StaticClass())))
		{
			const FName ViewModelName("GunnerViewModel_HitIndicator");
			View->SetViewModel(ViewModelName, HitIndicatorViewModel);
		}
	}
}
