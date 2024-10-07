// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AuthReloadBullet.h"

#include "Weapon.h"
#include "Gunner/Character/GunnerCharacter.h"

void UAnimNotify_AuthReloadBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	AWeapon* Weapon = MeshComp->GetOwner<AWeapon>();
	if(Weapon && Weapon->GetGunnerCharacterOwner() && Weapon->GetGunnerCharacterOwner()->HasAuthority())
	{
		int32 BulletCountToReload = Weapon->GetMaxBulletCount() - Weapon->GetBulletCount();
		BulletCountToReload = FMath::Min(BulletCountToReload, Weapon->GetMagazineBulletCount());
		Weapon->SetBulletCount(Weapon->GetBulletCount() + BulletCountToReload);
		Weapon->SetMagazineBulletCount(Weapon->GetMagazineBulletCount() - BulletCountToReload);
	}
	
}
