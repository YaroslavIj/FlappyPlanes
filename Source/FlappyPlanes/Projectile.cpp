// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "FlappyPlane.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AProjectile::OnProjectileOverlap);
}

void AProjectile::OnProjectileOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if (OtherActor->IsA(AFlappyPlane::StaticClass()))
		{
			if(OtherActor != GetOwner())
			{
				if (AFlappyPlane* Plane = Cast<AFlappyPlane>(OtherActor))
				{
					Plane->ReceiveDamage(Damage);
					Hit_Multicast(HitSound, HitFX);
				}
			}
		}
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Hit_Multicast_Implementation(USoundBase* Sound, UParticleSystem* FX)
{
	if (Sound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
	if (FX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, GetActorLocation());
	}
	this->Destroy();
}

