
#include "FlappyPlane.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"

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
			bIsFalling = false;
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
			FVector LiftVector = GetActorUpVector();
			float LiftForce = 0.5 * Speed * Speed * LiftCoefficient * AirDensity;
			if (bIsSpeedUp && Fuel - FuelConsumption * DeltaTime >= 0 && !bNeedToFlip)
			{
				float YOffset = FMath::Cos(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				float ZOffset = FMath::Sin(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				if (!bIsMovingForward)
				{
					YOffset = -YOffset;
				}
				UpwardOffsetDirection = FVector(0, YOffset, ZOffset);
				PlaneMesh->AddForce(UpwardOffsetDirection * SpeedUpOffsetForce);
				PlaneMesh->AddForce(LiftVector * LiftForce);

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
				RotationRate = UKismetMathLibrary::MapRangeClamped(CurrentForceWhileSpeedUp, EnginePower, SpeedUpForce, MinRotationRate, MaxRotationRate);
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
				if(CurrentForceWhileSpeedUp + AccelerationForce * DeltaTime <= SpeedUpForce)
				{
					CurrentForceWhileSpeedUp += AccelerationForce * DeltaTime;
				}
				else
				{
					CurrentForceWhileSpeedUp = SpeedUpForce;
				}
				PlaneMesh->AddForce(ForwardVector * CurrentForceWhileSpeedUp);
				Fuel -= FuelConsumption * DeltaTime;
				OnFuelChanged.Broadcast(Fuel);
			}
			else
			{
				//Old Version Code:
				
				//FQuat CurrentOrientation = GetActorQuat();
				//CurrentOrientation.Normalize();
				//FQuat TargetOrientationInverted = FQuat(FRotator(-45, -90, -180));
				//FQuat TargetOrientationAligned = FQuat(FRotator(-45, 90, 0));
				//TargetOrientationInverted.Normalize();
				//TargetOrientationAligned.Normalize();
				//float DotProductInverted = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationInverted.Vector());
				//float AngularDistanceToInverted = FMath::Acos(FMath::Clamp(DotProductInverted, -1.0f, 1.0f));
				//float DotProductAligned = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationAligned.Vector());
				//float AngularDistanceToAligned = FMath::Acos(FMath::Clamp(DotProductAligned, -1.0f, 1.0f));
				//if (AngularDistanceToInverted < AngularDistanceToAligned)
				//{
				//	if (bIsMovingForward)
				//	{
				//		bIsMovingForward = false;
				//		bIsTurnedUpsideDown = true;
				//	}
				//	/*if(bIsMovingForward)
				//	{
				//		TargetOrientationInverted = FQuat(FRotator(0, -90, 0));
				//	}
				//	else
				//	{
				//		TargetOrientationInverted = FQuat(FRotator(0, -90, -180));
				//	}	*/
				//	/*		if (FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
				//	{
				//		bIsMovingForward = false;
				//		if (bIsAligning)
				//		{
				//			bIsAligning = false;
				//		}
				//	}*/
				//	if (!FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
				//	{
				//		if (!bIsAligning)
				//		{
				//			//bIsAligning = true;
				//		}
				//		FQuat NewRotation;
				//		NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationInverted, AlignSpeed * DeltaTime);
				//		SetActorRotation(NewRotation);
				//	}
				//}
				//else
				//{
				//	if (!bIsMovingForward)
				//	{
				//		bIsMovingForward = true;
				//		bIsTurnedUpsideDown = true;
				//	}
				//	/*if(bIsMovingForward)
				//	{
				//		TargetOrientationAligned = FQuat(FRotator(0, 90, 0));
				//	}
				//	else
				//	{
				//		TargetOrientationAligned = FQuat(FRotator(0, 90, 180));
				//	}	*/
				//	/* if (FMath::IsNearlyZero(AngularDistanceToAligned, 0.1f))
				//	{
				//		bIsMovingForward = true;
				//		if(bIsAligning)
				//		{
				//			bIsAligning = false;
				//		}
				//	}*/
				//	if (!FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
				//	{
				//		if (!bIsAligning)
				//		{
				//			//bIsAligning = true;
				//		}
				//		FQuat NewRotation;
				//		NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationAligned, AlignSpeed * DeltaTime);
				//		SetActorRotation(NewRotation);
				//	}
				//}

				CurrentForceWhileSpeedUp = EnginePower;

				SetIsSpeedUp_Server(false);
				PlaneMesh->AddForce(ForwardVector * EnginePower);
				if (!bIsFalling)
				{
					FVector RotationAxis = FVector(1, 0, 0);

					FQuat CurrentOrientation = GetActorQuat();
					CurrentOrientation.Normalize();

					FQuat TargetOrientationInverted = FQuat(FRotator(-55, -90, 0));
					FQuat TargetOrientationAligned = FQuat(FRotator(-55, 90, 0));
					TargetOrientationInverted.Normalize();
					TargetOrientationAligned.Normalize();

					float DotProductInverted = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationInverted.Vector());
					float AngularDistanceToInverted = FMath::Acos(FMath::Clamp(DotProductInverted, -1.0f, 1.0f));
					float DotProductAligned = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationAligned.Vector());
					float AngularDistanceToAligned = FMath::Acos(FMath::Clamp(DotProductAligned, -1.0f, 1.0f));

					FQuat FlipFromForwardQuat = FQuat(FRotator(0, -90, -180));
					FQuat FlipFromBackwardQuat = FQuat(FRotator(0, 90, 180));
					FlipFromForwardQuat.Normalize();
					FlipFromBackwardQuat.Normalize();

					float DotProductFlipFromForward = FVector::DotProduct(CurrentOrientation.Vector(), FlipFromForwardQuat.Vector());
					float AngularDistanceToFlipFromForward = FMath::Acos(FMath::Clamp(DotProductFlipFromForward, -1.0f, 1.0f));
					float DotProductFlipFromBackward = FVector::DotProduct(CurrentOrientation.Vector(), FlipFromBackwardQuat.Vector());
					float AngularDistanceToFlipFromBackward = FMath::Acos(FMath::Clamp(DotProductFlipFromBackward, -1.0f, 1.0f));

					//if (AngularDistanceToInverted < AngularDistanceToAligned)
					/*if(bIsMovingForward && GetActorRotation().Pitch > -90 && GetActorRotation().Pitch < 0 && FMath::IsNearlyEqual(GetActorRotation().Yaw, -90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, -180, 1)
						|| !bIsMovingForward && !(GetActorRotation().Pitch > -90 && GetActorRotation().Pitch < 0 && FMath::IsNearlyEqual(GetActorRotation().Yaw, 90, 1) && FMath::IsNearlyEqual(GetActorRotation().Roll, 180, 1)))*/

					/*if (bIsMovingForward && FMath::RadiansToDegrees(AngularDistanceToFlipFromForward) < 90 && GetActorRotation().Pitch < 0
						|| !bIsMovingForward && !(FMath::RadiansToDegrees(AngularDistanceToFlipFromBackward) < 90 && GetActorRotation().Pitch < 0))*/
					if (AngularDistanceToInverted < AngularDistanceToAligned)
					{
						//(0, 90), 90, 180
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
							bNeedToFlip = true;
						}
						else
						{
							//FQuat NewRotation;
							//NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationInverted, FMath::DegreesToRadians(AlignSpeed * DeltaTime));
							//SetActorRotation(NewRotation);		
							FQuat RotationQuat;
							float DeltaRotation = FMath::DegreesToRadians(AlignSpeed) * DeltaTime;
							if (AngularDistanceToInverted - DeltaRotation < 0)
							{
								bIsFalling = true;
								RotationQuat = FQuat(RotationAxis, AngularDistanceToInverted);
							}
							else
							{
								if (GetActorRotation().Pitch < -55 && GetActorRotation().Pitch >= -90)
								{
									DeltaRotation = -DeltaRotation;
								}
								RotationQuat = FQuat(RotationAxis, DeltaRotation);
							}
							AddActorWorldRotation(RotationQuat);
						}
					}
					else
					{
						//(0, 90), -90, -180
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
							bNeedToFlip = true;
						}
						else
						{
							/*	FQuat NewRotation;
								NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationAligned, FMath::DegreesToRadians(AlignSpeed * DeltaTime));
								SetActorRotation(NewRotation);
								if (FMath::IsNearlyZero(AngularDistanceToAligned, 0.2f))
								{
									bIsFalling = true;
								}*/
							FQuat RotationQuat;
							float DeltaRotation = FMath::DegreesToRadians(AlignSpeed) * DeltaTime;
							if (AngularDistanceToAligned - DeltaRotation < 0)
							{
								bIsFalling = true;
								RotationQuat = FQuat(RotationAxis, AngularDistanceToAligned);
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
						}
					}
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

				//FQuat CurrentQuat = GetActorQuat();
				//FQuat TargetQuat = FQuat(RotationAxis, FMath::DegreesToRadians(180)) * InitialRotationForFlip;
				//float DotProduct = CurrentQuat.X * TargetQuat.X + CurrentQuat.Y * TargetQuat.Y + CurrentQuat.Z * TargetQuat.Z + CurrentQuat.W * TargetQuat.W;
				//float AngularDistance = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
				//if (FMath::IsNearlyZero(AngularDistance, 0.2f))

				//FQuat NewRotation = FQuat::Slerp(CurrentQuat, TargetQuat, FlipSpeed * DeltaTime);
				//SetActorRotation(NewRotation);


				FVector RotationAxis = GetActorForwardVector();
				FQuat RotationQuat;

				if (CurrentFlipRotation + FlipSpeed * DeltaTime >= 180.f)
				{
					RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(180.f - CurrentFlipRotation));
					AddActorWorldRotation(RotationQuat);
					bNeedToFlip = false;
					CurrentFlipRotation = 0;

				}
				else
				{
					RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(FlipSpeed) * DeltaTime);
					AddActorWorldRotation(RotationQuat);
					//PlaneMesh->AddForce(GetActorForwardVector()* EnginePower);
					CurrentFlipRotation += FlipSpeed * DeltaTime;
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
						}
						if (DeltaRotationForOther < 45)
						{
							FVector ImpulceDirection = OtherPlane->PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal();
							PlaneMesh->AddImpulse(ImpulceDirection * CollisionImpulceForOther);
							OtherPlane->PlaneMesh->AddImpulse(ImpulceDirection * -CollisionImpulceForSelf);
						}
					}
				}
				
			}
		}
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
