// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Gunner/PickupComponent.h"

AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = true;
	PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("Pickup"));
	PickupComponent->SetupAttachment(GetRootComponent());
}
