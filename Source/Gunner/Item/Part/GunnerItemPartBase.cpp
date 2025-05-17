// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItemPartBase.h"

#if WITH_EDITOR
EDataValidationResult UGunnerItemPartBase::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif
