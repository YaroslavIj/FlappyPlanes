
#include "FlappyPlane.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WorldDynamicShadow.h"
#include "GamePawn.h"

AFlappyPlane::AFlappyPlane()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	//OverlapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverlapMesh"));
	//OverlapMesh->SetupAttachment(RootComponent);
}
void AFlappyPlane::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlappyPlane, Health);
	DOREPLIFETIME(AFlappyPlane, Fuel);
	DOREPLIFETIME(AFlappyPlane, ProjectilesAmount);
	DOREPLIFETIME(AFlappyPlane, bIsSpeedUp);
	DOREPLIFETIME(AFlappyPlane, PawnOwner);
	DOREPLIFETIME(AFlappyPlane, CurrentProjectilesType);
}

void AFlappyPlane::BeginPlay()
{
	Super::BeginPlay();
	

	OnProjectilesAmountChanged.Broadcast(ProjectilesAmount);

	if(GetLocalRole() == ROLE_Authority)
	{
		if (ProjectileTypes.IsValidIndex(0))
		{
			CurrentProjectilesType = ProjectileTypes[0];
			ProjectilesAmount = CurrentProjectilesType.ProjectilesAmount;
		}
		for (FProjectilesSettings ProjectilesSettings : ProjectileTypes)
		{
			SetMaxProjectilesByTypeOnWidget_Multicast(ProjectilesSettings.MaxProjectilesAmount, ProjectilesSettings.ProjectileClass);
		}
	}
	//OnActorBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
	//OnActorHit.AddDynamic(this, &AFlappyPlane::OnHit);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
}

void AFlappyPlane::Dead()
{
	OnPlaneDied.Broadcast(this);
}

void AFlappyPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementTick(DeltaTime);
	ChangeFlightSoundVolumeTick_Multicast(DeltaTime);
}

void AFlappyPlane::SetIsSpeedUp_Server_Implementation(bool InbIsSpeedUp)
{
	if (InbIsSpeedUp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFlappyPlane::SetIsSpeedUp_Server_Implementation"));
		if(!bIsSpeedUp)
		{
			//ChangeFlightSound_Multicast(SpeedUpSound, SpeedUpSoundVolume);
			FTransform SpawnTransform;
			FVector RelativeLocation = FVector(GetActorForwardVector() * SpeedUpFXSpawnTransform.GetLocation().X + GetActorRightVector() * SpeedUpFXSpawnTransform.GetLocation().Y + GetActorUpVector() * SpeedUpFXSpawnTransform.GetLocation().Z);
			SpawnTransform.SetLocation(GetActorLocation() + RelativeLocation);
			SpawnTransform.SetRotation(GetActorQuat() * SpeedUpFXSpawnTransform.GetRotation());
			SpawnNiagaraAtLocation_Multicast(SpeedUpStartNiagara, SpawnTransform);
			//SpawnSpeedUpNiagaraAttached_Multicast(SpeedUpNiagara, SpawnTransform);
			bIsFalling = false;
			CurrentFallRotationSpeed = 0;
			if (Mesh)
			{
				//Mesh->SetEnableGravity(false);
			}
		}
	}
	else
	{
		if (bIsSpeedUp)
		{
			if (SpeedUpNiagaraComponent)
			{
				//SpeedUpNiagaraComponent->DestroyComponent();
			}
			StopSpeedUpNiagara_Multicast();
			if (Mesh)
			{
				Mesh->SetEnableGravity(true);
			}
			//ChangeFlightSound_Multicast(FlightSound, FlightSoundVolume);
		}
	}	
	bIsSpeedUp = InbIsSpeedUp;
}

void AFlappyPlane::FillFuel()
{
	Fuel = 1.f;
	OnFuelChanged.Broadcast(Fuel);
}

void AFlappyPlane::MovementTick(float DeltaTime)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if(Mesh && Mesh->IsSimulatingPhysics())
		{
			FVector LastLocation = GetActorLocation();
			FVector ForwardVector = GetActorForwardVector();
			ForwardVector.X = 0;
			//AddActorWorldOffset(ForwardVector * ForwardSpeed * DeltaTime);
			//PlaneMesh->AddForce(ForwardVector * ForwardSpeed);
			//PlaneMesh->AddForce(ForwardVector * ForwardSpeed);
			float Speed = Mesh->GetPhysicsLinearVelocity().Length();
			Mesh->AddForce(-Mesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
			
			//PlaneMesh->AddForce(-PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
			if (bIsSpeedUp && Fuel - FuelConsumption * DeltaTime >= 0)
			{
				TArray<AActor*> OverlapActors;
				
				GetOverlappingActors(OverlapActors, AActor::StaticClass());
				if(OverlapActors.Num() == 0)
				{
					Mesh->SetEnableGravity(true);
					//FVector LiftVector = GetActorUpVector();
					//if(RotationRate > MaxRotationRate / 2)
					{
						FVector Velocity = Mesh->GetPhysicsLinearVelocity();
						FVector Forward = GetActorForwardVector();
						FVector LiftVector = -FVector::CrossProduct(GetActorRightVector(), Velocity).GetSafeNormal();
						float LiftForce = 0.5 * Velocity.Length() * Velocity.Length() * LiftCoefficient * AirDensity;
						Mesh->AddForce(LiftVector * LiftForce);
					}

					float YOffset = FMath::Cos(FMath::DegreesToRadians(SpeedUpOffsetAngle));
					float ZOffset = FMath::Sin(FMath::DegreesToRadians(SpeedUpOffsetAngle));
					if (!bIsMovingForward)
					{
						YOffset = -YOffset;
					}
					UpwardOffsetDirection = FVector(0, YOffset, ZOffset);
					Mesh->AddForce(UpwardOffsetDirection * SpeedUpOffsetForce);
					/*	if (ForwardSpeed + AccelerationSpeed * DeltaTime < MaxForwardSpeed)
					{
						ForwardSpeed += AccelerationSpeed * DeltaTime;
					}
					else
					{
						ForwardSpeed = MaxForwardSpeed;
					}*/
					FVector CurrentLocation = GetActorLocation();

					//float Speed = FVector(CurrentLocation - LastLocation).Length();
					//float Speed = PlaneMesh->GetPhysicsLinearVelocity().Length();

					if (CurrentForceWhileSpeedUp + AccelerationForce * DeltaTime <= SpeedUpForce)
					{
						CurrentForceWhileSpeedUp += AccelerationForce * DeltaTime;
					}
					else
					{
						CurrentForceWhileSpeedUp = SpeedUpForce;
					}
					Mesh->AddForce(ForwardVector * CurrentForceWhileSpeedUp);

					if (RotationRate + RotationAcceleration * DeltaTime <= MaxRotationRate)
					{
						RotationRate += RotationAcceleration * DeltaTime;
					}
					else
					{
						RotationRate = MaxRotationRate;
					}
					//RotationRate = UKismetMathLibrary::MapRangeClamped(CurrentForceWhileSpeedUp, EnginePower, SpeedUpForce, MinRotationRate, MaxRotationRate);
					//RotationRate = UKismetMathLibrary::MapRangeClamped(CurrentForceWhileSpeedUp, 0, SpeedUpForce, MinRotationRate, MaxRotationRate);
					float NewRotationRate;
					if (bIsMovingForward)
					{
						NewRotationRate = -RotationRate;
					}
					else
					{
						NewRotationRate = RotationRate;
					}
					AddActorWorldRotation(FRotator(0, 0, NewRotationRate) * DeltaTime);
					Fuel -= FuelConsumption * DeltaTime;
					OnFuelChanged.Broadcast(Fuel);
				}
				else
				{
					Mesh->SetEnableGravity(false);
				}
			}
			else
			{
				//FVector LiftVector = PlaneMesh->GetPhysicsLinearVelocity() * -1.f;
				FVector LiftVector = FVector(0, 0, 1);
				float LiftForce = 0.5 * Speed * Speed * LiftCoefficient * AirDensity;
				for (FVector Point : LiftForcePoints)
				{
					//PlaneMesh->AddForceAtLocation(LiftVector * LiftForce, Point);

					//PlaneMesh->AddForceAtLocation(-LiftVector * LiftForce * LiftForce * LiftDragCoef, Point);
					/*FVector AngularVelocity = PlaneMesh->GetPhysicsAngularVelocityInDegrees();
					FVector AngularDampingTorque = -(AngularVelocity * AngularVelocity) * LiftDragCoef;
					PlaneMesh->AddTorqueInDegrees(AngularDampingTorque, NAME_None, true);*/
				}

				//CurrentForceWhileSpeedUp = EnginePower;

				SetIsSpeedUp_Server(false);
				//PlaneMesh->AddForce(ForwardVector * EnginePower);
				if (!bIsFalling)
				{
					FVector RotationAxis = FVector(1, 0, 0);

					FQuat CurrentOrientation = GetActorQuat();
					CurrentOrientation.Normalize();

					FQuat TargetOrientationFallingBackward = FQuat(FRotator(-55, -90, 0));
					FQuat TargetOrientationFallingForward = FQuat(FRotator(-55, 90, 0));
					TargetOrientationFallingBackward.Normalize();
					TargetOrientationFallingForward.Normalize();

					float DotProductFallingBackward = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationFallingBackward.Vector());
					float AngularDistanceToFallingBackward = FMath::Acos(FMath::Clamp(DotProductFallingBackward, -1.0f, 1.0f));
					float DotProductFallingForward = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationFallingForward.Vector());
					float AngularDistanceToFlallingForward = FMath::Acos(FMath::Clamp(DotProductFallingForward, -1.0f, 1.0f));

					/*FQuat FlipFromForwardQuat = FQuat(FRotator(0, -90, -180));
					FQuat FlipFromBackwardQuat = FQuat(FRotator(0, 90, 180));
					FlipFromForwardQuat.Normalize();
					FlipFromBackwardQuat.Normalize();*/
					//float DotProductFlipFromForward = FVector::DotProduct(CurrentOrientation.Vector(), FlipFromForwardQuat.Vector());
					//float AngularDistanceToFlipFromForward = FMath::Acos(FMath::Clamp(DotProductFlipFromForward, -1.0f, 1.0f));
					//float DotProductFlipFromBackward = FVector::DotProduct(CurrentOrientation.Vector(), FlipFromBackwardQuat.Vector());
					//float AngularDistanceToFlipFromBackward = FMath::Acos(FMath::Clamp(DotProductFlipFromBackward, -1.0f, 1.0f));
					//if (AngularDistanceToInverted < AngularDistanceToAligned)
					/*if(bIsMovingForward && GetActorRotation().Pitch > -90 && GetActorRotation().Pitch < 0 && FMath::IsNearlyEqual(GetActorRotation().Yaw, -90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, -180, 1)
						|| !bIsMovingForward && !(GetActorRotation().Pitch > -90 && GetActorRotation().Pitch < 0 && FMath::IsNearlyEqual(GetActorRotation().Yaw, 90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, 180, 1)))*/
					/*if (bIsMovingForward && FMath::RadiansToDegrees(AngularDistanceToFlipFromForward) < 90 && GetActorRotation().Pitch < 0
						|| !bIsMovingForward && !(FMath::RadiansToDegrees(AngularDistanceToFlipFromBackward) < 90 && GetActorRotation().Pitch < 0))*/
					float CurrentAngularDistance;
					bool bIsRotatedDown = false;
					if (AngularDistanceToFallingBackward < AngularDistanceToFlallingForward)
					{
						CurrentAngularDistance = AngularDistanceToFallingBackward;
						if (GetActorRotation().Pitch < -55 && GetActorRotation().Pitch >= -90)
						{
							bIsRotatedDown = true;
						}
						/*if (GetActorRotation().Pitch > 0 && GetActorRotation().Pitch < 90 && FMath::IsNearlyEqual(GetActorRotation().Yaw, 90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, 180, 1))
						{
							PlaneMesh->SetEnableGravity(false);
							PlaneMesh->AddForce(ForwardVector * AccelerationForce);
						}
						else
						{
							PlaneMesh->SetEnableGravity(true);
							PlaneMesh->AddForce(ForwardVector * EnginePower);
						}*/
						if (bIsMovingForward)
						{
							bIsMovingForward = false;
							if (!bNeedToFlip)
							{
								bNeedToFlip = true;
								bReturnFlip = false;
							}
							else
							{
								bReturnFlip = !bReturnFlip;
								//UE_LOG(LogTemp, Warning, TEXT("bReturnFlip = %f"), bReturnFlip);
							}
						}
						//else
						//{
						//
						//	//FQuat NewRotation;
						//	//NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationInverted, FMath::DegreesToRadians(AlignSpeed * DeltaTime));
						//	//SetActorRotation(NewRotation);		
						//	FQuat RotationQuat;
						//	float DeltaRotation = FMath::DegreesToRadians(CurrentFallRotationSpeed) * DeltaTime;
						//	if (AngularDistanceToFallingBackward - DeltaRotation < 0)
						//	{
						//		bIsFalling = true;
						//		RotationQuat = FQuat(RotationAxis, AngularDistanceToFallingBackward);
						//	}
						//	else
						//	{
						//		if (GetActorRotation().Pitch < -55 && GetActorRotation().Pitch >= -90)
						//		{
						//			DeltaRotation = -DeltaRotation;
						//		}
						//		RotationQuat = FQuat(RotationAxis, DeltaRotation);
						//	}
						//	AddActorWorldRotation(RotationQuat);
						//}
					}
					else
					{
						CurrentAngularDistance = AngularDistanceToFlallingForward;
						if (GetActorRotation().Pitch > -55 && GetActorRotation().Pitch <= 90)
						{
							bIsRotatedDown = true;
						}
						/*	if (GetActorRotation().Pitch > 0 && GetActorRotation().Pitch < 90 && FMath::IsNearlyEqual(GetActorRotation().Yaw, -90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, -180, 1))
						{
							PlaneMesh->SetEnableGravity(false);
							PlaneMesh->AddForce(ForwardVector* AccelerationForce);
						}
						else
						{
							PlaneMesh->SetEnableGravity(true);
							PlaneMesh->AddForce(ForwardVector * EnginePower);
						}*/
						if (!bIsMovingForward)
						{
							bIsMovingForward = true;
							if(!bNeedToFlip)
							{
								bNeedToFlip = true;
								bReturnFlip = false;
							}
							else
							{
								bReturnFlip = !bReturnFlip;
								//UE_LOG(LogTemp, Warning, TEXT("bReturnFlip = %f"), bReturnFlip);
							}
							
						}
						/*else
						{
							FQuat RotationQuat;
							float DeltaRotation = FMath::DegreesToRadians(CurrentFallRotationSpeed) * DeltaTime;
							if (AngularDistanceToFlallingForward - DeltaRotation < 0)
							{
								bIsFalling = true;
								RotationQuat = FQuat(RotationAxis, AngularDistanceToFlallingForward);
							}
							else
							{
								if (GetActorRotation().Pitch > -55 && GetActorRotation().Pitch <= 90)
								{
									DeltaRotation = -DeltaRotation;
								}
								RotationQuat = FQuat(RotationAxis, DeltaRotation);
							}
							AddActorWorldRotation(RotationQuat);
						}*/
					}
					
					CurrentForceWhileSpeedUp = MinSpeedUpForce;
					
					RotationRate = MinRotationRate;
					if(CurrentAngularDistance > FMath::DegreesToRadians(AngularDistanceToSlowFallRoation))
					{
						if (CurrentFallRotationSpeed + FallRotationAcceleration * DeltaTime <= MaxFallRotationSpeed)
						{
							CurrentFallRotationSpeed += FallRotationAcceleration * DeltaTime;
						}
						else if (CurrentFallRotationSpeed != MaxFallRotationSpeed)
						{
							CurrentFallRotationSpeed = MaxFallRotationSpeed;
						}
					}
					else
					{
						CurrentFallRotationSpeed = UKismetMathLibrary::MapRangeClamped(CurrentAngularDistance, FMath::DegreesToRadians(0), AngularDistanceToSlowFallRoation, MinFallRotationSpeed, MaxFallRotationSpeed);
					}
					FQuat RotationQuat;
					float DeltaRotation = FMath::DegreesToRadians(CurrentFallRotationSpeed) * DeltaTime;
					if (CurrentAngularDistance - DeltaRotation < 0)
					{
						bIsFalling = true;
						RotationQuat = FQuat(RotationAxis, CurrentAngularDistance);
					}
					else
					{
						if (bIsRotatedDown)
						{
							DeltaRotation = -DeltaRotation;
						}
						RotationQuat = FQuat(RotationAxis, DeltaRotation);
					}
					AddActorWorldRotation(RotationQuat);
					/*	if (GetActorRotation().Pitch > 0)
						{
							PlaneMesh->SetEnableGravity(false);
							PlaneMesh->AddForce(ForwardVector * AccelerationForce);
						}
						else
						{
							PlaneMesh->SetEnableGravity(true);
							PlaneMesh->AddForce(ForwardVector * EnginePower);
						}*/
				}
			}

			if (bNeedToFlip)
			{
				if(!bIsSpeedUp)
				{
					Mesh->AddForce(ForwardVector * MovementForceWhileFlip);
				}

				//FQuat CurrentQuat = GetActorQuat();
				//FQuat TargetQuat = FQuat(RotationAxis, FMath::DegreesToRadians(180)) * InitialRotationForFlip;
				//float DotProduct = CurrentQuat.X * TargetQuat.X + CurrentQuat.Y * TargetQuat.Y + CurrentQuat.Z * TargetQuat.Z + CurrentQuat.W * TargetQuat.W;
				//float AngularDistance = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
				//if (FMath::IsNearlyZero(AngularDistance, 0.2f))

				//FQuat NewRotation = FQuat::Slerp(CurrentQuat, TargetQuat, FlipSpeed * DeltaTime);
				//SetActorRotation(NewRotation);			

				//PlaneMesh->AddForce(ForwardVector* CurrentForceWhileSpeedUp);
				FVector RotationAxis = GetActorForwardVector();
				FQuat RotationQuat;
				float CurrentFlipSpeed;
				float TargetFlipRotation;
				if (!bReturnFlip)
				{
					CurrentFlipSpeed = FlipSpeed;
					TargetFlipRotation = 180.f;
				}
				else
				{
					CurrentFlipSpeed = -FlipSpeed;
					TargetFlipRotation = 0.f;
				}	
				if (CurrentFlipRotation + CurrentFlipSpeed * DeltaTime >= TargetFlipRotation && !bReturnFlip ||
					CurrentFlipRotation + CurrentFlipSpeed * DeltaTime <= TargetFlipRotation && bReturnFlip)
				{
					RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(TargetFlipRotation - CurrentFlipRotation));
					AddActorWorldRotation(RotationQuat);
					bNeedToFlip = false;
					bReturnFlip = false;
					CurrentFlipRotation = 180.f - TargetFlipRotation;
					if (TargetFlipRotation < 1)
					{
						UE_LOG(LogTemp, Warning, TEXT("TargetFlipRotation = 0"));
					}
					//bIsMovingForward = !bIsMovingForward;

				}
				else
				{
					RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(CurrentFlipSpeed) * DeltaTime);
					//RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(CurrentFallRotationSpeed) * DeltaTime);
					AddActorWorldRotation(RotationQuat);
					//PlaneMesh->AddForce(GetActorForwardVector()* EnginePower);
					CurrentFlipRotation += CurrentFlipSpeed * DeltaTime;
				}
				
				
			}
			if (!FMath::IsNearlyEqual(GetActorLocation().X, PlaneXPosition, 5))
			{
				FVector NewLocation = FVector(PlaneXPosition, GetActorLocation().Y, GetActorLocation().Z);
				SetActorLocation(NewLocation);
			}
		}
		if(LastVelocity.Length() - Mesh->GetPhysicsLinearVelocity().Length() > 1000.f)
		{
			LastVelocity = Mesh->GetPhysicsLinearVelocity();
			LastRotation = GetActorQuat();
		}
	}
}

void AFlappyPlane::SetIsFiring_Server_Implementation(bool InbIsFiring)
{	
	if(bIsGameStarted)
	{
		bIsFiring = InbIsFiring;
		if (InbIsFiring)
		{
			if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimer))
			{
				Fire();
			}
		}
	}
}

void AFlappyPlane::Fire()
{
	if(bIsFiring)
	{
		if (ProjectilesAmount > 0)
		{
			if (GetWorld() && CurrentProjectilesType.ProjectileClass)
			{
				FTransform SpawnTransform;
				FVector RelativeLocation = GetActorForwardVector() * FireLocation.X + GetActorRightVector() * FireLocation.Y + GetActorUpVector() * FireLocation.Z;
				SpawnTransform.SetLocation(GetActorLocation() + FireLocation);
				SpawnTransform.SetRotation(GetActorQuat());
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				GetWorld()->SpawnActor<AProjectile>(CurrentProjectilesType.ProjectileClass, SpawnTransform, SpawnParams);
				GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AFlappyPlane::Fire, CurrentProjectilesType.FireRate, false);

				if (!bHasInfiniteAmmo)
				{
					ProjectilesAmount--;
					OnProjectilesAmountChanged.Broadcast(ProjectilesAmount);
				}
			}
		}
	}
}

void AFlappyPlane::ReceiveDamage(float Damage)
{
	if(bIsGameStarted)
	{
		if (Health - Damage > 0)
		{
			if (Health - Damage <= 100)
			{
				Health -= Damage;
			}
			else
			{
				Health = 100;
			}
		}
		else
		{
			Health = 0;
			Dead();
		}
		OnHealthChanged.Broadcast(Health);
	}
}

void AFlappyPlane::StartGame()
{
	if (Mesh)
	{
		bIsGameStarted = true;
		Mesh->SetSimulatePhysics(true);
		RotationRate = MinRotationRate;
		FVector ImpulseDirection = GetActorForwardVector();
		Mesh->AddImpulse(ImpulseDirection * StartImpulse);
	}
}

void AFlappyPlane::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if (OtherActor)
		{		
			FVector SelfVelocity = Mesh->GetPhysicsLinearVelocity();
			if (OtherActor->IsA(AFlappyPlane::StaticClass()))
			{
				if (this < OtherActor) //Logic must run once
				{
					if(AFlappyPlane* OtherPlane = Cast<AFlappyPlane>(OtherActor))
					{
						FVector OtherVelocity = OtherPlane->Mesh->GetPhysicsLinearVelocity();
						//Find collsion rotation
						FRotator TargetRotationForSelf = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OtherActor->GetActorLocation());
						float DeltaRotationForSelf = FMath::RadiansToDegrees(GetActorQuat().AngularDistance(FQuat(TargetRotationForSelf)));
						FRotator TargetRotationForOther = UKismetMathLibrary::FindLookAtRotation(OtherActor->GetActorLocation(), GetActorLocation());
						float DeltaRotationForOther = FMath::RadiansToDegrees(OtherPlane->GetActorQuat().AngularDistance(FQuat(TargetRotationForOther)));
						
						if (DeltaRotationForSelf < 45 && DeltaRotationForOther > 45)
						{
							float OtherSpeedRelativeSelfVelocity = -FVector::DotProduct(SelfVelocity.GetSafeNormal(), OtherVelocity);
							float OverallSpeed = SelfVelocity.Length() + OtherSpeedRelativeSelfVelocity;
							FVector ImpulceDirection = Mesh->GetPhysicsLinearVelocity().GetSafeNormal();
							OtherPlane->Mesh->AddImpulse(OverallSpeed * ImpulceDirection * CollisionImpulceForOther);
							Mesh->AddImpulse(OverallSpeed * ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(OverallSpeed * CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(OverallSpeed * CollisionDamageForOther);
						}
						if (DeltaRotationForOther < 45 && DeltaRotationForSelf > 45)
						{
							float SelfSpeedRelativeOtherVelocity = -FVector::DotProduct(OtherVelocity.GetSafeNormal(), SelfVelocity);
							float OverallSpeed = OtherVelocity.Length() + SelfSpeedRelativeOtherVelocity;
							FVector ImpulceDirection = OtherPlane->Mesh->GetPhysicsLinearVelocity().GetSafeNormal();
							Mesh->AddImpulse(OverallSpeed * ImpulceDirection * CollisionImpulceForOther);
							OtherPlane->Mesh->AddImpulse(OverallSpeed * ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(OverallSpeed * CollisionDamageForOther);
							OtherPlane->ReceiveDamage(OverallSpeed * CollisionDamageForSelf);
						}
						if (DeltaRotationForSelf < 45 && DeltaRotationForOther < 45)
						{
							float OtherSpeedRelativeSelfVelocity = -FVector::DotProduct(SelfVelocity.GetSafeNormal(), OtherVelocity);
							float OverallSpeed = SelfVelocity.Length() + OtherSpeedRelativeSelfVelocity;
							FVector Direction = (OtherPlane->GetActorLocation() - GetActorLocation()).GetSafeNormal();
							OtherPlane->Mesh->AddImpulse(OverallSpeed * Direction * CollisionImpulceForSelf);
							Mesh->AddImpulse(OverallSpeed * Direction * -CollisionImpulceForSelf);
							ReceiveDamage(OverallSpeed * CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(OverallSpeed * CollisionDamageForSelf);
						}
					}
				}				
			}
			else if(!OtherActor->IsA(AProjectile::StaticClass()))
			{
				//FVector Direction = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			/*	FVector Direction = Mesh->GetPhysicsLinearVelocity().GetSafeNormal();

				Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				Mesh->AddImpulse(Direction * -CollisionImpulceForSelf);*/

				ReceiveDamage(SelfVelocity.Length() * CollisionDamageForSelf);

				/*FVector Velocity = Mesh->GetPhysicsLinearVelocity();
				FVector SurfaceNormal = SweepResult.ImpactNormal.GetSafeNormal();
				float ImpactAngle = FMath::Acos(FVector::DotProduct(SweepResult.ImpactNormal, -LastVelocity.GetSafeNormal()));
				FVector ImpulsDirection = Velocity - 2 * FVector::DotProduct(Velocity, SurfaceNormal) * SurfaceNormal;
				ImpulsDirection.Normalize();
				DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + ImpulsDirection * 2000.f, FColor::Red, false, 3.f);
				Mesh->AddImpulse(ImpulsDirection * CollisionImpulceForSelf);*/

				Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			}
		}
	}
}

void AFlappyPlane::SetMaxProjectilesByTypeOnWidget_Multicast_Implementation(float MaxProjectiles, TSubclassOf<AProjectile> Type)
{
	//BP
}

void AFlappyPlane::NextProjectilesType_Server_Implementation()
{
	for (int32 i = 0; i < ProjectileTypes.Num(); i++)
	{
		if (CurrentProjectilesType.ProjectileClass == ProjectileTypes[i].ProjectileClass)
		{
			ProjectileTypes[i].ProjectilesAmount = ProjectilesAmount;
			if (ProjectileTypes.IsValidIndex(i + 1))
			{
				CurrentProjectilesType = ProjectileTypes[i + 1];
			}
			else
			{
				CurrentProjectilesType = ProjectileTypes[0];
			}
			ProjectilesAmount = CurrentProjectilesType.ProjectilesAmount;
			return;
		}
	}
}

void AFlappyPlane::CreateDynamicShadow_Multicast_Implementation()
{
	if (PawnOwner && PawnOwner->GetController() && PawnOwner->GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		DynamicShadow = GetWorld()->SpawnActor<AWorldDynamicShadow>(WorldDynamicShadowClass);
		if(DynamicShadow)
		{
			DynamicShadow->Plane = this;
		}
	}	
}

void AFlappyPlane::StopSpeedUpNiagara_Multicast_Implementation()
{
	if(SpeedUpNiagaraComponent)
	{
		SpeedUpNiagaraComponent->Deactivate();
		//SpeedUpNiagaraComponent->DestroyComponent();
	}
}

void AFlappyPlane::SpawnSpeedUpNiagaraAttached_Multicast_Implementation(UNiagaraSystem* NiagaraFX, FTransform SpawnTransform)
{
	if (NiagaraFX)
	{
		SpeedUpNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraFX, Mesh, NAME_None, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true, true);
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, SpeedUpStartFXSpawnTransform,);
	}
}

void AFlappyPlane::SpawnNiagaraAtLocation_Multicast_Implementation(UNiagaraSystem* NiagaraFX, FTransform SpawnTransform)
{
	if(NiagaraFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraFX, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), SpawnTransform.GetScale3D(), true, true);
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, SpeedUpStartFXSpawnTransform,);
	}
}

void AFlappyPlane::OnRep_Health_Implementation()
{
	//BP
}

void AFlappyPlane::OnRep_Fuel_Implementation()
{
	//BP
}

void AFlappyPlane::OnRep_ProjectilesAmount_Implementation()
{
	//BP
}

void AFlappyPlane::ChangeFlightSoundVolumeTick_Multicast_Implementation(float DeltaTime)
{	
	if(!CurrentFlightSound)
	{
		CurrentFlightSound = UGameplayStatics::SpawnSoundAttached(FlightSound, Mesh, NAME_None, FVector(0), EAttachLocation::SnapToTarget, false, FlightSoundVolume);
	}
	if(CurrentFlightSound && CurrentFlightSound->IsPlaying())
	{
		float ChangeRate;
		if (bIsSpeedUp)
		{
			ChangeRate = FlightSoundVolumeCangeRate * DeltaTime;
		}
		else
		{
			ChangeRate = -FlightSoundVolumeCangeRate * DeltaTime;
		}
		if (CurrentFlightSound->VolumeMultiplier + ChangeRate > FlightSoundVolume && CurrentFlightSound->VolumeMultiplier + ChangeRate < SpeedUpSoundVolume)
		{
			CurrentFlightSound->SetVolumeMultiplier(CurrentFlightSound->VolumeMultiplier + ChangeRate);
		}
	}
}

void AFlappyPlane::FillProjectilesAmount()
{
	for (FProjectilesSettings Type : ProjectileTypes)
	{
		SetMaxProjectilesByTypeOnWidget_Multicast(Type.MaxProjectilesAmount, Type.ProjectileClass);
	}
}

void AFlappyPlane::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if(!OtherActor->IsA(AProjectile::StaticClass()))
		{
			
			if (OtherActor->IsA(AFlappyPlane::StaticClass()))
			{
				if (this < OtherActor) //Logic must run once
				{
					if (AFlappyPlane* OtherPlane = Cast<AFlappyPlane>(OtherActor))
					{
						//Find collsion rotation
						FRotator TargetRotationForSelf = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OtherActor->GetActorLocation());
						float DeltaRotationForSelf = FMath::RadiansToDegrees(GetActorQuat().AngularDistance(FQuat(TargetRotationForSelf)));
						FRotator TargetRotationForOther = UKismetMathLibrary::FindLookAtRotation(OtherActor->GetActorLocation(), GetActorLocation());
						float DeltaRotationForOther = FMath::RadiansToDegrees(OtherPlane->GetActorQuat().AngularDistance(FQuat(TargetRotationForOther)));
						if (DeltaRotationForSelf < 45)
						{
							FVector ImpulceDirection = Mesh->GetPhysicsLinearVelocity().GetSafeNormal();
							OtherPlane->Mesh->AddImpulse(ImpulceDirection * CollisionImpulceForOther);
							Mesh->AddImpulse(ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(CollisionDamageForOther);
						}
						if (DeltaRotationForOther < 45)
						{
							FVector ImpulceDirection = OtherPlane->Mesh->GetPhysicsLinearVelocity().GetSafeNormal();
							Mesh->AddImpulse(ImpulceDirection * CollisionImpulceForOther);
							OtherPlane->Mesh->AddImpulse(ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForOther);
							OtherPlane->ReceiveDamage(CollisionDamageForSelf);
						}
						if (DeltaRotationForSelf > 45 && DeltaRotationForOther > 45)
						{
							FVector Direction = (OtherPlane->GetActorLocation() - GetActorLocation()).GetSafeNormal();
							OtherPlane->Mesh->AddImpulse(Direction * CollisionImpulceForSelf);
							Mesh->AddImpulse(Direction * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(CollisionDamageForSelf);
						}
					}
				}
			}
			else
			{
				//FVector Velocity = Mesh->GetPhysicsLinearVelocity();
				FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
				float ImpactAngle = FMath::Acos(FVector::DotProduct(Hit.ImpactNormal, -LastVelocity.GetSafeNormal()));
				FVector ImpulsDirection = LastVelocity.GetSafeNormal() - 2 * FVector::DotProduct(LastVelocity.GetSafeNormal(), SurfaceNormal) * SurfaceNormal;
				Mesh->AddImpulse(ImpulsDirection * CollisionImpulceForSelf);
			}
		}
		SetActorRotation(LastRotation);
	}
}
