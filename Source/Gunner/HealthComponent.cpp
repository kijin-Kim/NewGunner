// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "Gunner.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::OnDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	FVector Location;
	FRotator Rotation;
	InstigatedBy->GetPlayerViewPoint(Location, Rotation);
	GR_LOG(LogGunner, Warning, TEXT("Distance: %fm, Bone: %s"), FVector::Distance(HitLocation, Location) / 100.0f,*BoneName.ToString());
	Health -= Damage;
	Health = FMath::Max(Health, 0.0f);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnTakePointDamage.AddDynamic(this, &ThisClass::OnDamage);
	Health = MaxHealth;
}
