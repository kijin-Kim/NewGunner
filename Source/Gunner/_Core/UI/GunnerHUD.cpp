// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"

#include "Blueprint/UserWidget.h"

void AGunnerHUD::LocalDebugTarget()
{
	if (APawn* PossessedPawn = GetOwningPawn())
	{
		ShowDebugTargetActor = PossessedPawn;
	}
}

void AGunnerHUD::InitializeMainWidgets()
{
	check(GetOwningPlayerController() && GetOwningPlayerController()->PlayerState);
	for (const TSubclassOf<UUserWidget>& WidgetClass : MainWidgetClasses)
	{
		if (WidgetClass)
		{
			if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetOwningPlayerController(), WidgetClass))
			{
				Widget->AddToViewport();
			}
		}
	}
}
