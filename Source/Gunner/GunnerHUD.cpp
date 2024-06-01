// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"

#include "Gunner.h"
#include "Blueprint/UserWidget.h"


void AGunnerHUD::SetupHUD()
{
	check(OverlayWidgetClass);
	if (!OverlayWidget)
	{
		OverlayWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), OverlayWidgetClass);
		OverlayWidget->AddToViewport();
	}
}
