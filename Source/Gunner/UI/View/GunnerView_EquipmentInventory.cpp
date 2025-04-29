// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerView_EquipmentInventory.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Gunner/UI/ViewModel/GunnerInventoryViewModel_Equipment.h"

void UGunnerEquipmentInventoryItemWidget::SetImageAndText(const FSlateBrush& IconBrush, const FText& DisplayName)
{
	ItemIconImage->SetBrush(IconBrush);
	DisplayNameTextBlock->SetText(DisplayName);
}

void UGunnerEquipmentInventoryItemWidget::ClearImageAndText()
{
	FSlateBrush EmptyBrush;
	EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	ItemIconImage->SetBrush(EmptyBrush);
	DisplayNameTextBlock->SetText(FText::GetEmpty());
}

void UGunnerEquipmentInventoryItemWidget::PlayFadeOutAnimation()
{
	StopAllAnimations();
	if (FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UGunnerEquipmentInventoryItemWidget::PlayHighlightedFadeOutAnimation()
{
	StopAllAnimations();
	if (HighlightedFadeOutAnimation)
	{
		PlayAnimation(HighlightedFadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UGunnerView_EquipmentInventory::OnEquipmentItemChanged(const TMap<EGunnerSlotType, FGunnerEquipmentHudDataEntry>& EquipmentHudData)
{
	for (const auto& [SlotType, Widget] : EquipmentWidgets)
	{
		check(Widget);
		UGunnerEquipmentInventoryItemWidget* EquipmentWidget = Cast<UGunnerEquipmentInventoryItemWidget>(Widget);
		check(EquipmentWidget);

		if (!EquipmentHudData.Contains(SlotType))
		{
			EquipmentWidget->ClearImageAndText();
			continue;
		}

		EquipmentWidget->SetImageAndText(EquipmentHudData[SlotType].StandardIconBrush, EquipmentHudData[SlotType].ItemNames);
	}
}

void UGunnerView_EquipmentInventory::OnEquippedSlotTypeChanged(EGunnerSlotType SlotType)
{
	for (const auto& [Type, Widget] : EquipmentWidgets)
	{
		check(Widget);
		UGunnerEquipmentInventoryItemWidget* EquipmentWidget = Cast<UGunnerEquipmentInventoryItemWidget>(Widget);
		check(EquipmentWidget);

		if (Type == SlotType)
		{
			EquipmentWidget->PlayHighlightedFadeOutAnimation();
		}
		else
		{
			EquipmentWidget->PlayFadeOutAnimation();
		}
	}
}
