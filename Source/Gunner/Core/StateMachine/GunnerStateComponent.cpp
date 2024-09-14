// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerStateComponent.h"

#include "Gunner/Gunner.h"

UGunnerStateComponent::UGunnerStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunnerStateComponent::OnEnter_Implementation()
{
	
}

void UGunnerStateComponent::OnUpdate_Implementation(float DeltaTime)
{
}

void UGunnerStateComponent::OnExit_Implementation()
{
}

void UGunnerStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

