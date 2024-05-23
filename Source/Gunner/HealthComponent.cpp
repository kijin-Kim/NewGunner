// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "Core/GunnerGameInstance.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::OnRegister()
{
	Super::OnRegister();
	Health = MaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnTakePointDamage.AddUniqueDynamic(this, &ThisClass::OnTakePointDamage);
}

int32 UHealthComponent::ProcessDamage(float Distance, AActor* DamageCauser)
{
	UGunnerGameInstance* GameInstance = GetWorld()->GetGameInstance<UGunnerGameInstance>();
	check(GameInstance);
	UCurveTable* DamageTable = GameInstance->GetDamageTable();
	check(DamageTable);
	float OutDamage = 0.0f;
	TEnumAsByte<EEvaluateCurveTableResult::Type> OutResult;
	AWeapon* Weapon = Cast<AWeapon>(DamageCauser);
	check(Weapon);
	FName RowName = *FString::Printf(TEXT("%s.Body"), *Weapon->GetWeaponName().ToString());
	UDataTableFunctionLibrary::EvaluateCurveTableRow(DamageTable, RowName, Distance, OutResult, OutDamage, FString());
	check(OutResult == EEvaluateCurveTableResult::RowFound);
	return OutDamage;
}

void UHealthComponent::OnTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	int32 ProcessedDamage = ProcessDamage(0.0f, DamageCauser);
	Health -= ProcessedDamage;
	Health = FMath::Clamp(Health, 0, MaxHealth);
	if (OnHealthChangedDelegate.IsBound())
	{
		OnHealthChangedDelegate.Broadcast(Health);
	}
}

void UHealthComponent::OnRep_Health()
{
	if (OnHealthChangedDelegate.IsBound())
	{
		OnHealthChangedDelegate.Broadcast(Health);
	}
}
