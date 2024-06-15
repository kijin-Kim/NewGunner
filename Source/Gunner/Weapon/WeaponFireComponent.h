// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponFireComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponBulletCountChangedSignature, int32, BulletCount, int32, MagazineBulletCount);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	virtual void InitializeComponent() override;
	virtual void OnRegister() override;
	UFUNCTION()
	void OnPrimaryAction(bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire(bool bPressed);

	void LocalFire(bool bPressed);

	
	UFUNCTION()
	void ConsumeBullets();
	void LocalReload();
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
