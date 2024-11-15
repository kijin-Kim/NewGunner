// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActionSystem/GunnerActionSideEffectDefinitionHandle.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


UENUM()
enum class EGunnerActionPropertyOperator
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Override
};


USTRUCT()
struct FGunnerActionPropertyOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EGunnerActionPropertyOperator Operator;
	UPROPERTY(EditAnywhere)
	float Operand;
	UPROPERTY(EditAnywhere)
	float Duration;
	
	FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle;
};

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionProperty : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	void MakePropertyDirty();
	void Evaluate(const TArray<FGunnerActionPropertyOperation>& PropertyOperation, float& TargetValue);

	virtual void PreNetReceive() override;
	virtual void PostNetReceive() override;


	
public:
	UPROPERTY(Replicated)
	FGameplayTag Tag;
	UPROPERTY(Replicated)
	float StaticValue;
	UPROPERTY(Replicated)
	float DynamicValue;
	TArray<FGunnerActionPropertyOperation> StaticOperations;
	TArray<FGunnerActionPropertyOperation> DynamicOperations;

};
