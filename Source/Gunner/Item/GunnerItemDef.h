// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"

#include "GunnerItemDef.generated.h"


class UGunnerItemInstance;
class AGunnerItem;
class UGunnerItemPartBase;

UCLASS()
class GUNNER_API UGunnerItemDef : public UDataAsset
{
	GENERATED_BODY()

public:
	UGunnerItemDef();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = "ItemPartClass"))
	const UGunnerItemPartBase* FindItemPartByClass(TSubclassOf<UGunnerItemPartBase> ItemPartClass) const;

	template <typename T>
	const T* FindItemPartByClass() const
	{
		return Cast<T>(FindItemPartByClass(T::StaticClass()));
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AGunnerItem> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UNexusAction>> AcquiredActionClasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<const UGunnerItemPartBase>> ItemParts;
};
