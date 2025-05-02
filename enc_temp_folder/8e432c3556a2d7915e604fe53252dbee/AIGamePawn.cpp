// Fill out your copyright notice in the Description page of Project Settings.


#include "AIGamePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void AAIGamePawn::BeginPlay()
{
	Super::BeginPlay();
}

void AAIGamePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetLocalRole() == ROLE_Authority)
	{
		if (Plane)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlappyPlane::StaticClass(), FoundActors);
			for (AActor* Actor : FoundActors)
			{
				if (AFlappyPlane* FoundPlane = Cast<AFlappyPlane>(Actor))
				{
					if (FoundPlane != Plane)
					{	
						
						//float DeltaRotationToEnemy = FMath::RadiansToDegrees(Plane->GetActorQuat().AngularDistance(FQuat(TargetRotation)));
						FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Plane->GetActorLocation(), FoundPlane->GetActorLocation());
						FVector RightAxis = Plane->GetActorRightVector();
						FQuat TargetQuat = FQuat(TargetRotation);
						FQuat CurrentQuat = Plane->GetActorQuat();
					/*	FQuat TargetQuatUp = FQuat(RightAxis, FMath::DegreesToRadians(5)) * CurrentQuat;
						FQuat TargetQuatDown = FQuat(RightAxis, FMath::DegreesToRadians(-5)) * CurrentQuat;*/
					/*	if(CurrentQuat.AngularDistance(TargetQuatUp) > CurrentQuat.AngularDistance(TargetQuatDown) && Plane->bIsMovingForward 
							|| CurrentQuat.AngularDistance(TargetQuatUp) < CurrentQuat.AngularDistance(TargetQuatDown) && !Plane->bIsMovingForward)
						{
							Plane->SetIsSpeedUp_Server(true);
						}
						else
						{
							Plane->SetIsSpeedUp_Server(false);
						}*/
						FVector CurrentForward = Plane->GetActorForwardVector();
						FVector TargetForward = FoundPlane->GetActorLocation() - Plane->GetActorLocation();
						TargetForward.Normalize();

						// ”бираем компоненту по RightAxis, оставл€€ только "вращение вокруг Right"
						FVector CurrentProjected = FVector::VectorPlaneProject(CurrentForward, RightAxis).GetSafeNormal();
						FVector TargetProjected = FVector::VectorPlaneProject(TargetForward, RightAxis).GetSafeNormal();

						// ”гол между направлени€ми в плоскости
						float Angle = FMath::Acos(FVector::DotProduct(CurrentProjected, TargetProjected));

						// ќпредел€ем знак через CrossProduct и Dot с RightAxis
						float Sign = FVector::DotProduct(FVector::CrossProduct(CurrentProjected, TargetProjected), RightAxis);
						float SignedAngle = Angle * FMath::Sign(Sign);
						if(!bNeedToCoup)
						{
							if (SignedAngle > 0 && Plane->bIsMovingForward || SignedAngle < 0 && !Plane->bIsMovingForward)
							{
								Plane->SetIsSpeedUp_Server(true);
							}
							else
							{
								if (Plane->bIsFalling)
								{
									bNeedToCoup = true;
								}
								Plane->SetIsSpeedUp_Server(false);
							}
						}
						else
						{
							Plane->SetIsSpeedUp_Server(true);
						}
						if (CurrentQuat.AngularDistance(TargetQuat) < FMath::DegreesToRadians(10))
						{
							bNeedToCoup = false;
							//Plane->SetIsSpeedUp_Server(false);
							Plane->SetIsFiring_Server(true);
						}
						else
						{
							//Plane->SetIsSpeedUp_Server(true);
							Plane->SetIsFiring_Server(false);
						}
					/*	FVector ToTarget = FoundPlane->GetActorLocation() - Plane->GetActorLocation();
						FVector CurretSightLocation = Plane->GetActorLocation() + ToTarget;
						for()
						if ()
						{
							Plane->SetIsSpeedUp_Server(true);
						}
						else
						{
							Plane->SetIsSpeedUp_Server(false);
						}*/
						/*FQuat DeltaRotation = CurrentQuat.Inverse() * TargetQuat;
						float Angle;
						FVector Axis;
						DeltaRotation.ToAxisAndAngle(Axis, Angle);
						float Projection = FVector::DotProduct(Axis, RightAxis);*/
						//FinalAngle = Angle * Projection;
						//if(!FMath::IsNearlyZero(FinalAngle, 0.f))
						//{	
						//	if (FinalAngle > 0 && !Plane->bIsMovingForward || FinalAngle < 0 && Plane->bIsMovingForward)
						//	{
						//		Plane->SetIsSpeedUp_Server(true);
						//	}
						//	else
						//	{
						//		Plane->SetIsSpeedUp_Server(false);
						//	}
						//	/*FVector DirectionToTarget = FoundPlane->GetActorLocation() - Plane->GetActorLocation();
						//	DirectionToTarget.Normalize();
						//	FVector RotationAxis = Plane->GetActorRightVector();
						//	FVector CrossProduct = FVector::CrossProduct(RotationAxis, DirectionToTarget);
						//	float Direction = FVector::DotProduct(RotationAxis, DirectionToTarget);*/
						//}
						/*if (FMath::RadiansToDegrees(FinalAngle) <= 10.f)
						{
							Plane->SetIsFiring_Server(true);
						}
						else
						{
							Plane->SetIsFiring_Server(false);
						}	*/
						break;
					}
				}
			}
		}
	}
}

void AAIGamePawn::StartGame()
{
	if (Plane)
	{
		//Plane->SetIsSpeedUp_Server(true);
	}
}
