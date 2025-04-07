// Fill out your copyright notice in the Description page of Project Settings.


#include "Refueling.h"
#include "FlappyPlane.h"

// Sets default values
ARefueling::ARefueling()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

}

// Called when the game starts or when spawned
void ARefueling::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &ARefueling::OnOverlap);
}

// Called every frame
void ARefueling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARefueling::Refill()
{
	Mesh->SetVisibility(true);
	bIsFilled = true;
}

void ARefueling::OnOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if (bIsFilled)
		{
			if (AFlappyPlane* Plane = Cast<AFlappyPlane>(OtherActor))
			{
				Plane->FillFuel();
				Plane->ReceiveDamage(-HealthRecovery);
				Plane->SetProjectilesAmount(Plane->GetMaxProjectilesAmount());
				if (bIsRefillable)
				{
					Mesh->SetVisibility(false);
					bIsFilled = false;
					if (GetWorld())
					{
						GetWorld()->GetTimerManager().SetTimer(RefillingTimer, this, &ARefueling::Refill, RefillingTime, false);
					}
				}
				else
				{
					Destroy();
				}
			}
		}
	}
}

