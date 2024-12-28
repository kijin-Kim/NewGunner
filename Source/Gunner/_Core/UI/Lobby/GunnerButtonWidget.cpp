// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerButtonWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/NamedSlot.h"
#include "Components/SizeBox.h"

void UGunnerButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox)
	{
		if (Width > 0)
		{
			SizeBox->SetWidthOverride(Width);
		}

		if (Height > 0)
		{
			SizeBox->SetHeightOverride(Height);
		}
	}

	
	
	if (Border)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.OutlineSettings.RoundingType = RoundingType;
		Brush.OutlineSettings.CornerRadii = CornerRadii;
		Border->SetBrush(Brush);
		Border->SetBrushColor(BackgroundColor);
	}


	if (Button)
	{
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = InnerBackgroundColor;
		ButtonStyle.Normal.OutlineSettings.Color = OutlineColor;
		ButtonStyle.Normal.OutlineSettings.Width = OutlineWidth;
		ButtonStyle.Normal.OutlineSettings.RoundingType = RoundingType;
		ButtonStyle.Normal.OutlineSettings.CornerRadii = CornerRadii;

		ButtonStyle.Hovered.TintColor = InnerBackgroundColor;
		ButtonStyle.Hovered.OutlineSettings.Color = OutlineColor;
		ButtonStyle.Hovered.OutlineSettings.Width = OutlineWidth;
		ButtonStyle.Hovered.OutlineSettings.CornerRadii = CornerRadii;
		ButtonStyle.Hovered.OutlineSettings.RoundingType = RoundingType;

		ButtonStyle.Pressed.TintColor = InnerBackgroundColor;
		ButtonStyle.Pressed.OutlineSettings.Color = OutlineColor;
		ButtonStyle.Pressed.OutlineSettings.Width = OutlineWidth;
		ButtonStyle.Pressed.OutlineSettings.CornerRadii = CornerRadii;
		ButtonStyle.Pressed.OutlineSettings.RoundingType = RoundingType;

		ButtonStyle.Disabled.TintColor = InnerBackgroundColor;
		ButtonStyle.Disabled.OutlineSettings.Color = OutlineColor;
		ButtonStyle.Disabled.OutlineSettings.Width = OutlineWidth;
		ButtonStyle.Disabled.OutlineSettings.CornerRadii = CornerRadii;
		ButtonStyle.Disabled.OutlineSettings.RoundingType = RoundingType;
		
		ButtonStyle.Pressed.TintColor = HighlightColor;
		ButtonStyle.Pressed.OutlineSettings.Width = 0;
		Button->SetStyle(ButtonStyle);
	}
}

void UGunnerButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->OnPressed.AddDynamic(this, &UGunnerButtonWidget::OnButtonPressed);
		Button->OnReleased.AddDynamic(this, &UGunnerButtonWidget::OnButtonReleased);
	}
}

void UGunnerButtonWidget::OnButtonPressed()
{
	if (Button)
	{
		Button->SetColorAndOpacity(FLinearColor::Black);
	}

	if (Border)
	{
		Border->SetBrushColor(HighlightColor);
	}
}

void UGunnerButtonWidget::OnButtonReleased()
{
	if (Button)
	{
		Button->SetColorAndOpacity(FLinearColor::White);
	}

	if (Border)
	{
		Border->SetBrushColor(BackgroundColor);
	}
}
