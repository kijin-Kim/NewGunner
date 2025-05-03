// Fill out your copyright notice in the Description page of Project Set
// tings.


#include "GunnerItemDef.h"
#include "GunnerItem.h"
#include "Part/GunnerItemPartBase.h"
#include "Misc/DataValidation.h"


UGunnerItemDef::UGunnerItemDef()
{
	ItemClass = AGunnerItem::StaticClass();
}

EDataValidationResult UGunnerItemDef::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (TSubclassOf<UNexusAction> ActionClass : AcquiredActionClasses)
	{
		if (!ActionClass)
		{
			Context.AddError(NSLOCTEXT("Gunner", "InvalidActionClass", "유효하지 않은 액션 클래스 존재"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}
	for (const UGunnerItemPartBase* ItemPart : ItemParts)
	{
		if (!ItemPart)
		{
			Context.AddError(NSLOCTEXT("Gunner", "InvalidItemComponent", "유효하지 않은 아이템 파츠 존재"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
			continue;
		}

		Result = CombineDataValidationResults(Result, ItemPart->IsDataValid(Context));
	}

	TSet<TSubclassOf<UGunnerItemPartBase>> UniqueItemPartClasses;
	for (const TObjectPtr<const UGunnerItemPartBase>& ItemPart : ItemParts)
	{
		if (ItemPart && !UniqueItemPartClasses.Contains(ItemPart->GetClass()))
		{
			UniqueItemPartClasses.Add(ItemPart->GetClass());
			continue;
		}
		Context.AddError(NSLOCTEXT("Gunner", "DuplicateItemComponent", "중복된 아이템 파츠 클래스 존재"));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}

	return Result;
}

const UGunnerItemPartBase* UGunnerItemDef::FindItemPartByClass(TSubclassOf<UGunnerItemPartBase> ItemPartClass) const
{
	const TObjectPtr<const UGunnerItemPartBase>* ItemPartPtr = ItemParts.FindByPredicate([ItemPartClass](const UGunnerItemPartBase* ItemPart)
	{
		return ItemPart->IsA(ItemPartClass);
	});
		
	return ItemPartPtr ? *ItemPartPtr : nullptr;
}
