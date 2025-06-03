
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

	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	RootComponent = PlaneMesh;
}
void AFlappyPlane::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlappyPlane, Health);
	DOREPLIFETIME(AFlappyPlane, Fuel);
	DOREPLIFETIME(AFlappyPlane, ProjectilesAmount);
	DOREPLIFETIME(AFlappyPlane, bIsSpeedUp);
	DOREPLIFETIME(AFlappyPlane, PawnOwner);
}

void AFlappyPlane::BeginPlay()
{
	Super::BeginPlay();
	
	ProjectilesAmount = MaxProjectilesAmount;
	OnProjectilesAmountChanged.Broadcast(ProjectilesAmount);

	OnActorBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
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
			if (PlaneMesh)
			{
				PlaneMesh->SetEnableGravity(false);
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
			if (PlaneMesh)
			{
				PlaneMesh->SetEnableGravity(true);
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
		if(PlaneMesh->IsSimulatingPhysics())
		{
			FVector LastLocation = GetActorLocation();
			FVector ForwardVector = GetActorForwardVector();
			ForwardVector.X = 0;
			//AddActorWorldOffset(ForwardVector * ForwardSpeed * DeltaTime);
			//PlaneMesh->AddForce(ForwardVector * ForwardSpeed);
			float Speed = PlaneMesh->GetPhysicsLinearVelocity().Length();
			PlaneMesh->AddForce(-PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
			
			//PlaneMesh->AddForce(-PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
			if (bIsSpeedUp && Fuel - FuelConsumption * DeltaTime >= 0)
			{
				/*FVector LiftVector = GetActorUpVector();
				float LiftForce = 0.5 * Speed * Speed * LiftCoefficient * AirDensity;*/


				float YOffset = FMath::Cos(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				float ZOffset = FMath::Sin(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				if (!bIsMovingForward)
				{
					YOffset = -YOffset;
				}
				UpwardOffsetDirection = FVector(0, YOffset, ZOffset);
				PlaneMesh->AddForce(UpwardOffsetDirection * SpeedUpOffsetForce);
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
				
				if(CurrentForceWhileSpeedUp + AccelerationForce * DeltaTime <= SpeedUpForce)
				{
					CurrentForceWhileSpeedUp += AccelerationForce * DeltaTime;
				}
				else
				{
					CurrentForceWhileSpeedUp = SpeedUpForce;
				}
				PlaneMesh->AddForce(ForwardVector * CurrentForceWhileSpeedUp);
				
				if (RotationRate + RotationAcceleration * DeltaTime <= MaxRotationRate)
				{
					RotationRate += RotationAcceleration* DeltaTime;
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
							}
							else
							{
								bReturnFlip = !bReturnFlip;
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
							}
							else 
							{
								bReturnFlip = !bReturnFlip;
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
					
					CurrentForceWhileSpeedUp = 0;
					
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
				PlaneMesh->AddForce(ForwardVector* MovementForceWhileFlip);

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
			if (GetWorld() && ProjectileClass)
			{
				FTransform SpawnTransform;
				FVector RelativeLocation = GetActorForwardVector() * FireLocation.X + GetActorRightVector() * FireLocation.Y + GetActorUpVector() * FireLocation.Z;
				SpawnTransform.SetLocation(GetActorLocation() + FireLocation);
				SpawnTransform.SetRotation(GetActorQuat());
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnTransform, SpawnParams);
				GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AFlappyPlane::Fire, FireRate, false);

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
	if (PlaneMesh)
	{
		bIsGameStarted = true;
		PlaneMesh->SetSimulatePhysics(true);
		RotationRate = MinRotationRate;
		FVector ImpulseDirection = GetActorForwardVector();
		PlaneMesh->AddImpulse(ImpulseDirection * StartImpulse);
	}
}

void AFlappyPlane::OnOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if (OtherActor)
		{		
			if (OtherActor->IsA(AFlappyPlane::StaticClass()))
			{
				if (this < OtherActor) //Logic must run once
				{
					if(AFlappyPlane* OtherPlane = Cast<AFlappyPlane>(OtherActor))
					{
						//Find collsion rotation
						FRotator TargetRotationForSelf = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OtherActor->GetActorLocation());
						float DeltaRotationForSelf = FMath::RadiansToDegrees(GetActorQuat().AngularDistance(FQuat(TargetRotationForSelf)));
						FRotator TargetRotationForOther = UKismetMathLibrary::FindLookAtRotation(OtherActor->GetActorLocation(), GetActorLocation());
						float DeltaRotationForOther = FMath::RadiansToDegrees(OtherPlane->GetActorQuat().AngularDistance(FQuat(TargetRotationForOther)));
						if (DeltaRotationForSelf < 45)
						{
							FVector ImpulceDirection = PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal();
							OtherPlane->PlaneMesh->AddImpulse(ImpulceDirection * CollisionImpulceForOther);
							PlaneMesh->AddImpulse(ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(CollisionDamageForOther);
						}
						if (DeltaRotationForOther < 45)
						{
							FVector ImpulceDirection = OtherPlane->PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal();
							PlaneMesh->AddImpulse(ImpulceDirection * CollisionImpulceForOther);
							OtherPlane->PlaneMesh->AddImpulse(ImpulceDirection * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForOther);
							OtherPlane->ReceiveDamage(CollisionDamageForSelf);
						}
						if (DeltaRotationForSelf > 45 && DeltaRotationForOther > 45)
						{
							FVector Direction = (OtherPlane->GetActorLocation() - GetActorLocation()).GetSafeNormal();
							OtherPlane->PlaneMesh->AddImpulse(Direction * CollisionImpulceForSelf);
							PlaneMesh->AddImpulse(Direction * -CollisionImpulceForSelf);
							ReceiveDamage(CollisionDamageForSelf);
							OtherPlane->ReceiveDamage(CollisionDamageForSelf);
						}
					}
				}				
			}
			else if(!OtherActor->IsA(AProjectile::StaticClass()))
			{
				FVector Direction = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				PlaneMesh->AddImpulse(Direction * -CollisionImpulceForSelf);
				ReceiveDamage(CollisionDamageForSelf);
			}
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
		SpeedUpNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraFX, PlaneMesh, NAME_None, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true, true);
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
	//if (CurrentFlightSound)
	//{
	//	//CurrentFlightSound->Stop();
	//	//CurrentFlightSound->DestroyComponent();

	//}
	if(!CurrentFlightSound)
	{
		CurrentFlightSound = UGameplayStatics::SpawnSoundAttached(FlightSound, PlaneMesh, NAME_None, FVector(0), EAttachLocation::SnapToTarget, false, FlightSoundVolume);
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
