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
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGunnerPickup::OnBeginOverlap);
	BoxComponent->OnComponentCollisionSettingsChangedEvent.AddDynamic(this, &AGunnerPickup::OnCollisionSettingsChanged);
	if (PickupInitialDelay > 0.0f)
	{
		FTimerHandle InitialDelayTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(InitialDelayTimerHandle, [this]()
		{
			BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		}, PickupInitialDelay, false);
	}
	else
	{
		BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

void AGunnerPickup::OnCollisionSettingsChanged(UPrimitiveComponent* ChangedComponent)
{
	ClearComponentOverlaps();
	UpdateOverlaps();
}
