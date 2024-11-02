// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerBulletComponent.h"
#include "Engine/Canvas.h"


UGunnerBulletComponent::UGunnerBulletComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunnerBulletComponent::OnRegister()
{
	Super::OnRegister();
	BulletCount = MaxBulletCountPerMagazine;
	MagazineBulletCount = MaxMagazineBulletCount;
}

void UGunnerBulletComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	Super::InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("BulletCount: %d"), BulletCount));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("MagazineBulletCount: %d"), MagazineBulletCount));
}

bool UGunnerBulletComponent::CanReloadBullet() const
{
	return BulletCount < MaxBulletCountPerMagazine && MagazineBulletCount > 0;
}

void UGunnerBulletComponent::ReloadBullet()
{
	if (CanReloadBullet())
	{
		const int32 BulletCountToAdd = FMath::Min(MaxBulletCountPerMagazine - BulletCount, MagazineBulletCount);
		BulletCount += BulletCountToAdd;
		MagazineBulletCount -= BulletCountToAdd;
		OnBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}

void UGunnerBulletComponent::SetBulletCount(int32 NewBulletCount)
{
	BulletCount = NewBulletCount;
	BulletCount = FMath::Min(BulletCount, MaxBulletCountPerMagazine);
	OnBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
}

void UGunnerBulletComponent::AddBulletCount(int32 BulletCountToAdd)
{
	BulletCount += BulletCountToAdd;
	BulletCount = FMath::Min(BulletCount, MaxBulletCountPerMagazine);
	OnBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
}
