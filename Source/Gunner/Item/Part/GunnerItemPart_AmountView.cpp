// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItemPart_AmountView.h"

#include "Blueprint/UserWidget.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR
EDataValidationResult UGunnerItemPart_AmountView::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (!ViewClass)
	{
		Context.AddError(NSLOCTEXT("GunnerItemPart_AmountView", "InvalidWidgetClass", "유효하지 않은 뷰 클래스 존재"));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}

	return Result;
}
#endif
