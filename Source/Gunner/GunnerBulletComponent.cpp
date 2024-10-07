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
	MagazineCount = MaxMagazineCount;
}

void UGunnerBulletComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	Super::OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("BulletCount: %d"), BulletCount));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("MagazineCount: %d"), MagazineCount));
}

void UGunnerBulletComponent::SetBulletCount(int32 NewBulletCount)
{
	BulletCount = NewBulletCount;
	BulletCount = FMath::Min(BulletCount, MaxBulletCountPerMagazine);
	BroadcastOnBulletCountChangedDelegate();
}

void UGunnerBulletComponent::SetMagazineCount(int32 NewMagazineCount)
{
	MagazineCount = NewMagazineCount;
	MagazineCount = FMath::Min(MagazineCount, MaxMagazineCount);
	BroadcastOnBulletCountChangedDelegate();
}

void UGunnerBulletComponent::AddBulletCount(int32 BulletCountToAdd)
{
	BulletCount += BulletCountToAdd;
	BulletCount = FMath::Min(BulletCount, MaxBulletCountPerMagazine);
	BroadcastOnBulletCountChangedDelegate();
}

void UGunnerBulletComponent::AddMagazineCount(int32 MagazineCountToAdd)
{
	MagazineCount += MagazineCountToAdd;
	MagazineCount = FMath::Min(MagazineCount, MaxMagazineCount);
	BroadcastOnBulletCountChangedDelegate();
}

void UGunnerBulletComponent::BroadcastOnBulletCountChangedDelegate()
{
	OnBulletCountChangedDelegate.Broadcast(BulletCount, MagazineCount * MaxBulletCountPerMagazine);
}
