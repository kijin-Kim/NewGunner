// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"
#include "GunnerUserWidget.h"
#include "Gunner/_Core/UI/GunnerOverlayWidgetController.h"


void AGunnerHUD::SetupHUD(APlayerState* PlayerState)
{
	check(OverlayWidgetClass);
	if (!OverlayWidget)
	{
		OverlayWidget = CreateWidget<UGunnerUserWidget>(GetOwningPlayerController(), OverlayWidgetClass);
		UGunnerOverlayWidgetController* OverlayWidgetController = NewObject<UGunnerOverlayWidgetController>(GetOwningPlayerController());
		OverlayWidgetController->InitWidgetController(PlayerState);
		OverlayWidget->InitUserWidget(OverlayWidgetController);
		OverlayWidget->AddToViewport();
	}
}
