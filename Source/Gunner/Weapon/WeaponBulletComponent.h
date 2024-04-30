// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponBulletComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponBulletCountChangedSignature, int32, BulletCount, int32, MagazineBulletCount);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponBulletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponBulletComponent();
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnWeaponFired();

public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponBulletCountChangedSignature OnWeaponBulletCountChangedDelegate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxBulletCount;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxMagazineCount;

protected:
	UPROPERTY(BlueprintReadOnly)
	int32 BulletCount;
	UPROPERTY(BlueprintReadOnly)
	int32 MagazineCount;
};
