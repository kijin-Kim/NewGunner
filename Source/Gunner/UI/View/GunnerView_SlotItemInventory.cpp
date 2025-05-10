// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerView_SlotItemInventory.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Gunner/UI/ViewModel/GunnerInventoryViewModel_SlotItem.h"

void UGunnerInventorySlotItemWidget::SetImageAndText(const FSlateBrush& IconBrush, const FText& DisplayName)
{
	ItemIconImage->SetBrush(IconBrush);
	DisplayNameTextBlock->SetText(DisplayName);
}

void UGunnerInventorySlotItemWidget::ClearImageAndText()
{
	FSlateBrush EmptyBrush;
	EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	ItemIconImage->SetBrush(EmptyBrush);
	DisplayNameTextBlock->SetText(FText::GetEmpty());
}

void UGunnerInventorySlotItemWidget::PlayFadeOutAnimation()
{
	StopAllAnimations();
	if (FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UGunnerInventorySlotItemWidget::PlayHighlightedFadeOutAnimation()
{
	StopAllAnimations();
	if (HighlightedFadeOutAnimation)
	{
		PlayAnimation(HighlightedFadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UGunnerView_SlotItemInventory::OnSlotItemChanged(const TMap<EGunnerSlotType, FGunnerSlotItemHudDataEntry>& SlotItemHudData)
{
	for (const auto& [SlotType, Widget] : SlotItemWidgets)
	{
		check(Widget);
		UGunnerInventorySlotItemWidget* SlotItemWidget = Cast<UGunnerInventorySlotItemWidget>(Widget);
		check(SlotItemWidget);

		if (!SlotItemHudData.Contains(SlotType))
		{
			SlotItemWidget->ClearImageAndText();
			continue;
		}

		SlotItemWidget->SetImageAndText(SlotItemHudData[SlotType].StandardIconBrush, SlotItemHudData[SlotType].ItemNames);
	}
}

void UGunnerView_SlotItemInventory::OnActiveSlotItemTypeChanged(EGunnerSlotType SlotType)
{
	for (const auto& [Type, Widget] : SlotItemWidgets)
	{
		check(Widget);
		UGunnerInventorySlotItemWidget* SlotItemWidget = Cast<UGunnerInventorySlotItemWidget>(Widget);
		check(SlotItemWidget);

		if (Type == SlotType)
		{
			SlotItemWidget->PlayHighlightedFadeOutAnimation();
		}
		else
		{
			SlotItemWidget->PlayFadeOutAnimation();
		}
	}
}

