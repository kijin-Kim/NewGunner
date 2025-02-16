// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"

#include "EngineUtils.h"
#include "GunnerUserWidget.h"
#include "Gunner/_Core/GunnerOverlayWidget.h"
#include "NexusActionComponent.h"
#include "Gunner/_Core/UI/GunnerOverlayWidgetController.h"


void AGunnerHUD::SetupHUD(APlayerState* PlayerState)
{
	check(OverlayWidgetClass);
	if (!OverlayWidget)
	{
		OverlayWidget = CreateWidget<UGunnerOverlayWidget>(GetOwningPlayerController(), OverlayWidgetClass);
		UGunnerOverlayWidgetController* OverlayWidgetController = NewObject<UGunnerOverlayWidgetController>(GetOwningPlayerController());
		OverlayWidgetController->InitWidgetController(PlayerState);
		OverlayWidget->InitUserWidget(OverlayWidgetController);
		OverlayWidget->AddToViewport();
	}
}

void AGunnerHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	Super::GetDebugActorList(InOutList);
	InOutList.RemoveAll([this](AActor* Actor)
	{
		return UNexusActionComponent::GetActionComponentFromActor(Actor) == nullptr;
	});
	UWorld* World = GetWorld();
	check(World);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && UNexusActionComponent::GetActionComponentFromActor(Actor) != nullptr)
		{
			InOutList.AddUnique(Actor);
		}
	}
}
