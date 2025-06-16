// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "FlappyPlane.h"

void AHomingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * ProjectileMovement->InitialSpeed;
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlappyPlane::StaticClass(), OutActors);
	for (AActor* Plane : OutActors)
	{
		if (Plane != GetOwner())
		{
			FQuat NeededRotation = FQuat(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Plane->GetActorLocation()));
			FQuat CurrentRotation = GetActorQuat();
			float DotProduct = FVector::DotProduct(NeededRotation.Vector(), CurrentRotation.Vector());
			float AngulatDistance = FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f));
			if (AngulatDistance <= FMath::DegreesToRadians(FindTargetAngle))
			{
				ProjectileMovement->HomingTargetComponent = Plane->GetRootComponent();
			}
		}
	}
}
