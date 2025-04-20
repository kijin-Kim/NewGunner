// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerSlotItemUiData.generated.h"

class UGunnerSlotItemPersistentUiComponentBase;
class UGunnerSlotItemTransientUiComponentBase;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSlotItemUiData : public UDataAsset
{
	GENERATED_BODY()

public:
	template <typename T>
	const T* GetPersistentComponentByClass() const
	{
		for (const TObjectPtr<const UGunnerSlotItemPersistentUiComponentBase>& UIComponent : PersistentComponents)
		{
			if (UIComponent->IsA(T::StaticClass()))
			{
				return Cast<T>(UIComponent);
			}
		}
		return nullptr;
	}


	template <typename T>
	const T* GetTransientComponentByClass() const
	{
		for (const TObjectPtr<const UGunnerSlotItemTransientUiComponentBase>& UIComponent : TransientComponents)
		{
			if (UIComponent->IsA(T::StaticClass()))
			{
				return Cast<T>(UIComponent);
			}
		}
		return nullptr;
	}

public:
	
	UPROPERTY(EditAnywhere)
	FText ItemName;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> StandardIconTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> SimplifiedIconTexture;
	
	UPROPERTY(EditAnywhere, Instanced)
	TArray<TObjectPtr<const UGunnerSlotItemPersistentUiComponentBase>> PersistentComponents;

	UPROPERTY(EditAnywhere, Instanced)
	TArray<TObjectPtr<const UGunnerSlotItemTransientUiComponentBase>> TransientComponents;
};
