// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponFireComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFiredSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();
	virtual void InitializeComponent() override;
	UFUNCTION()
	void OnPrimaryAction(bool bPressed);

public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponFiredSignature OnWeaponFiredDelegate;
};
