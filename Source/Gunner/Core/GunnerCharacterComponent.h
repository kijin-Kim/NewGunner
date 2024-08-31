// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "GunnerCharacterComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerCharacterComponent();

	// Following codes are stripped form UPawnComponent
	template <class T>
	T* GetGunnerCharacterOwner() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AGunnerCharacter>::Value, "'T' template parameter to GetPawn must be derived from AGunnerCharacter");
		return Cast<T>(GetOwner());
	}

	template <class T>
	T* GetGunnerCharacterOwnerChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AGunnerCharacter>::Value, "'T' template parameter to GetPawnChecked must be derived from AGunnerCharacter");
		return CastChecked<T>(GetOwner());
	}
};
