// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGun.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

AGunnerGun::AGunnerGun()
{

}

void AGunnerGun::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BulletCount = Magazine;
	MagazineBulletCount = Reserve;
	MaxBulletPerMagazineCount = Magazine;
}

void AGunnerGun::OnActivated()
{
	Super::OnActivated();
	AActor* ActorOwner = GetOwner();

	if (ActorOwner && ActorOwner->HasAuthority())
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_Bullet);
		BulletProperty->SetStaticValue(BulletCount);
		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_MagazineBullet);
		MagazineBulletProperty->SetStaticValue(MagazineBulletCount);
		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(TAG_Property_Weapon_MaxBulletPerMagazine);
		MaxBulletPerMagazineProperty->SetStaticValue(MaxBulletPerMagazineCount);
	}
}

void AGunnerGun::OnDeactivated()
{
	Super::OnDeactivated();
	AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->HasAuthority())
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_Bullet);
		BulletCount = BulletProperty->GetStaticValue();
		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_MagazineBullet);
		MagazineBulletCount = MagazineBulletProperty->GetStaticValue();
		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(TAG_Property_Weapon_MaxBulletPerMagazine);
		MaxBulletPerMagazineCount = MaxBulletPerMagazineProperty->GetStaticValue();
	}
}
