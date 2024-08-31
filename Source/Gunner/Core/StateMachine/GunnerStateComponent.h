// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerStateComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerStateComponent();
	UFUNCTION(BlueprintNativeEvent)
	void OnEnter();
	// StateMachine에서 현재 State가 진입되었을 시에만 StateMachine의 매 프레임마다 호출 됩니다.
	UFUNCTION(BlueprintNativeEvent)
	void OnUpdate(float DeltaTime);
	UFUNCTION(BlueprintNativeEvent)
	void OnExit();


private:
	// 자식클래스가 TickComponent를 override하지 못하도록 final 키워드를 사용합니다. OnUpdate 참고
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override final;
};
