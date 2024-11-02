// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GunnerPropertyComponent.h"
#include "GunnerBulletComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletCountChangedSignature, int32, BulletCount, int32, MagazineBulletCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerBulletComponent : public UGunnerPropertyComponent
{
	GENERATED_BODY()

public:
	UGunnerBulletComponent();
	virtual void OnRegister() override;
	virtual void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y) override;

	UFUNCTION(BlueprintCallable)
	bool CanReloadBullet() const;
	UFUNCTION(BlueprintCallable)
	void ReloadBullet();
	UFUNCTION(BlueprintCallable)
	void SetBulletCount(int32 NewBulletCount);
	UFUNCTION(BlueprintCallable)
	void AddBulletCount(int32 BulletCountToAdd);

	int32 GetBulletCount() const { return BulletCount; }
	int32 GetMagazineBulletCount() const { return MagazineBulletCount; }
	int32 GetMaxBulletCountPerMagazine() const { return MaxBulletCountPerMagazine; }
	int32 GetMaxMagazineBulletCount() const { return MaxMagazineBulletCount; }
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnBulletCountChangedSignature OnBulletCountChangedDelegate;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 BulletCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 MaxBulletCountPerMagazine;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 MagazineBulletCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 MaxMagazineBulletCount;

	
	FGunnerActionPropertyModifier BulletCountProperty(FGameplayTag());
	
};
