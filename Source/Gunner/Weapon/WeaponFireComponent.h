// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponFireComponent.generated.h"



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();
	void InitializeComponent() override;
	void DestroyComponent(bool bPromoteChildren) override;
	UFUNCTION()
	void OnWeaponEquip();
	UFUNCTION()
	void OnWeaponUnequip();
	UFUNCTION()
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire();
	void WeaponLineTrace();

	UFUNCTION(Client, Reliable)
	void ClientDrawServerRegisteredHitBox(const TArray<FTransform>& HitBoxTransforms, const TArray<FVector2D>& Sizes);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;
};
