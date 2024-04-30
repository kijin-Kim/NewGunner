// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponDebugFireComponent.generated.h"


class AGunnerCharacter;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponDebugFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponDebugFireComponent();
	virtual void InitializeComponent() override;
	virtual void DestroyComponent(bool bPromoteChildren) override;
	

	UFUNCTION()
	void OnPrimaryAction(bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire(double TimeStamp);
	void WeaponLineTrace(double TimeStamp);
	void LineTrace();
	

	UFUNCTION(Client, Reliable)
	void ClientDrawHitBoxes(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time);
	
	AGunnerCharacter* GetGunnerCharacterOwner() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;

	

};
