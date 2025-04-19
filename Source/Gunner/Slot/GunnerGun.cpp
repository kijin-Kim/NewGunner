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

void AGunnerGun::OnActivated(AActor* AgentActor)
{
	Super::OnActivated(AgentActor);
	if (AgentActor && AgentActor->HasAuthority())
	{
		UNexusActionComponent* ActionComponent = GetActionComponent(AgentActor);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
		BulletProperty->SetStaticValue(BulletCount);
		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
		MagazineBulletProperty->SetStaticValue(MagazineBulletCount);
		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MaxBulletPerMagazine);
		MaxBulletPerMagazineProperty->SetStaticValue(MaxBulletPerMagazineCount);
	}
}

void AGunnerGun::OnDeactivated(AActor* AgentActor)
{
	Super::OnDeactivated(AgentActor);
	if (AgentActor && AgentActor->HasAuthority())
	{
		UNexusActionComponent* ActionComponent = GetActionComponent(AgentActor);
		check(ActionComponent);

		UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
		BulletCount = BulletProperty->GetStaticValue();
		BulletProperty->SetStaticValue(0.0f);
		UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
		MagazineBulletCount = MagazineBulletProperty->GetStaticValue();
		MagazineBulletProperty->SetStaticValue(0.0f);
		UNexusProperty* MaxBulletPerMagazineProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MaxBulletPerMagazine);
		MaxBulletPerMagazineCount = MaxBulletPerMagazineProperty->GetStaticValue();
		MaxBulletPerMagazineProperty->SetStaticValue(0.0f);
	}
}
