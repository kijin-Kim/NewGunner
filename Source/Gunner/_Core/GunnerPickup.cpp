// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPickup.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGunnerPickup::AGunnerPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->SetBoxExtent({30.0f, 30.0f, 5.0f});

	SetRootComponent(BoxComponent);
}


bool AGunnerPickup::CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const
{
	return IsValid(OtherActor) && IsValid(OtherComponent);
}

void AGunnerPickup::OnPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent)
{
}

void AGunnerPickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FTimerHandle InitialDelayTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(InitialDelayTimerHandle, [this]()
		{
			BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGunnerPickup::OnBeginOverlap);
			ClearComponentOverlaps();
			UpdateOverlaps();
			
		}, PickupInitialDelay, false);
	}
}

void AGunnerPickup::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CanPickup(OtherActor, OtherComp))
	{
		OnPickup(OtherActor, OtherComp);
		Destroy();
	}
}
