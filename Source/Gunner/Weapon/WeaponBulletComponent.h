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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void InitializeComponent() override;
	virtual void OnRegister() override;
	UFUNCTION()
	void OnWeaponFired();
	UFUNCTION()
	void OnReload();
	bool CanReload() const;
	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION()
	void OnRep_Bullet();

public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponBulletCountChangedSignature OnWeaponBulletCountChangedDelegate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxBulletCount;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxMagazineBulletCount;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_Bullet, BlueprintReadOnly)
	int32 BulletCount;
	UPROPERTY(ReplicatedUsing=OnRep_Bullet, BlueprintReadOnly)
	int32 MagazineBulletCount;
};
