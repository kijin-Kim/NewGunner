// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"
#include "Blueprint/UserWidget.h"


void AGunnerHUD::SetupHUD()
{
	check(OverlayWidgetClass);
	OverlayWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), OverlayWidgetClass);
	OverlayWidget->AddToViewport();
}
