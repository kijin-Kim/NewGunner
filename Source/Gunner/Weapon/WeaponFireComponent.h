// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/CircularQueue.h"
#include "WeaponFireComponent.generated.h"


USTRUCT()
struct FHitBox
{
	GENERATED_BODY()
	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	float HalfHeight;
	UPROPERTY()
	float Radius;
	UPROPERTY()
	FName BoneName;
};

USTRUCT()
struct FHitBoxHistory
{
	GENERATED_BODY()
	double Time;
	TMap<AActor*, TArray<FHitBox>> HitBoxes;
};



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();
	void InitializeComponent() override;
	void DestroyComponent(bool bPromoteChildren) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void OnWeaponEquip();
	UFUNCTION()
	void OnWeaponUnequip();
	UFUNCTION()
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(double TimeStamp);
	void WeaponLineTrace(double TimeStamp);
	
	UFUNCTION(Client, Reliable)
	void ClientDrawServerRegisteredHitBox(const TArray<FHitBox>& HitBoxes, FColor Color);


private:
	void SaveHitBoxes();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;

	TCircularQueue<FHitBoxHistory> HitBoxHistories{9};
};
