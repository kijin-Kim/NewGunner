// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerUserWidget.h"

void UGunnerUserWidget::InitUserWidget(UGunnerUserWidgetController* InController)
{
	UserWidgetController = InController;
}

UGunnerUserWidgetController* UGunnerUserWidget::GetUserWidgetController() const
{
	return UserWidgetController;
}
