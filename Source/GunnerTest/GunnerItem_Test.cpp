// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItem_Test.h"


AGunnerItem_Test::AGunnerItem_Test()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AGunnerItem_Test::OnRemoved()
{
	LastAgentActor = AgentActor;
	Super::OnRemoved();
}
