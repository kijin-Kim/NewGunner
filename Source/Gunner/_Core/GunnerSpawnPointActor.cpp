// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSpawnPointActor.h"

#if WITH_EDITORONLY_DATA
#include "Components/SphereComponent.h"
#endif


// Sets default values
AGunnerSpawnPointActor::AGunnerSpawnPointActor()
{
	PrimaryActorTick.bCanEverTick = false;

#ifdef WITH_EDITORONLY_DATA
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(SpawnRadius);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetHiddenInGame(false);
#endif
}

#if WITH_EDITORONLY_DATA
void AGunnerSpawnPointActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AGunnerSpawnPointActor, SpawnRadius))
	{
		if (SphereComponent)
		{
			SphereComponent->SetSphereRadius(SpawnRadius);
		}
	}
}
#endif
