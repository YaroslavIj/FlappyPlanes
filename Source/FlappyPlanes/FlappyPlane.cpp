
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
	Mesh->SetAngularDamping(99999999.f);
	OverlapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverlapMesh"));
	OverlapMesh->SetupAttachment(RootComponent);
	OverlapMesh->SetVisibility(true);
	OverlapMesh->SetRelativeScale3D(FVector(1.1f));
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
		NextProjectilesType_Server();
		for (FProjectilesSettings ProjectilesSettings : ProjectileTypes)
		{
			SetMaxProjectilesByTypeOnWidget_Multicast(ProjectilesSettings.MaxProjectilesAmount, ProjectilesSettings.ProjectileClass);
		}
	}
	//OnActorBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
	//OverlapMesh->OnComponentBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
	OnActorHit.AddDynamic(this, &AFlappyPlane::OnHit);
	OverlapMesh->OnComponentBeginOverlap.AddDynamic(this, &AFlappyPlane::OnOverlap);
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

	if (HitedActor)
	{
		bool bIsHitedActorNearby = false;
		TArray<AActor*> OverlappingActors;
		OverlapMesh->GetOverlappingActors(OverlappingActors);
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor == HitedActor)
			{
				bIsHitedActorNearby = true;
			}
		}

		if (!bIsHitedActorNearby)
		{
			HitedActor = nullptr;
		}
	}
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
			float Speed = Mesh->GetPhysicsLinearVelocity().Length();
			Mesh->AddForce(-Mesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
			
			if (bIsSpeedUp && Fuel - FuelConsumption * DeltaTime >= 0)
			{
				TArray<AActor*> OverlapActors;
				
				GetOverlappingActors(OverlapActors, AActor::StaticClass());
				
					
				FVector Velocity = Mesh->GetPhysicsLinearVelocity();
				FVector Forward = GetActorForwardVector();
				FVector LiftVector = -FVector::CrossProduct(GetActorRightVector(), Velocity).GetSafeNormal();
				float LiftForce = 0.5 * Velocity.Length() * Velocity.Length() * LiftCoefficient * AirDensity;
				Mesh->AddForce(LiftVector * LiftForce);
					

				float YOffset = FMath::Cos(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				float ZOffset = FMath::Sin(FMath::DegreesToRadians(SpeedUpOffsetAngle));
				if (!bIsMovingForward)
				{
					YOffset = -YOffset;
				}
				UpwardOffsetDirection = FVector(0, YOffset, ZOffset);
				Mesh->AddForce(UpwardOffsetDirection * SpeedUpOffsetForce);
				FVector CurrentLocation = GetActorLocation();


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
				SetIsSpeedUp_Server(false);

				if (!bIsFalling && !HitedActor)
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

					float CurrentAngularDistance;
					bool bIsRotatedDown = false;
					if (AngularDistanceToFallingBackward < AngularDistanceToFlallingForward)
					{
						CurrentAngularDistance = AngularDistanceToFallingBackward;
						if (GetActorRotation().Pitch < -55 && GetActorRotation().Pitch >= -90)
						{
							bIsRotatedDown = true;
						}
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
							}
						}
					}
					else
					{
						CurrentAngularDistance = AngularDistanceToFlallingForward;
						if (GetActorRotation().Pitch > -55 && GetActorRotation().Pitch <= 90)
						{
							bIsRotatedDown = true;
						}
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
				}
			}

			if (bNeedToFlip)
			{
				if(!bIsSpeedUp)
				{
					Mesh->AddForce(ForwardVector * MovementForceWhileFlip);
				}
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
					CurrentFlipRotation = 0;
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
		//if(LastVelocity.Length() - Mesh->GetPhysicsLinearVelocity().Length() > 1000.f)
		{
			//LastVelocity = Mesh->GetPhysicsLinearVelocity();
			//LastRotation = GetActorQuat();
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
				
				float Dispersion = FMath::RandRange(-FireDispersionDegreese, FireDispersionDegreese);
				FRotator SpawnRotator = GetActorRotation() + FRotator(Dispersion, 0, 0);
				SpawnTransform.SetRotation(FQuat(SpawnRotator));

				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				GetWorld()->SpawnActor<AProjectile>(CurrentProjectilesType.ProjectileClass, SpawnTransform, SpawnParams);
				GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AFlappyPlane::Fire, CurrentProjectilesType.FireRate, false);

				SpawnSoundAtLocation_Multicast(CurrentProjectilesType.FireSound, SpawnTransform.GetLocation(), SpawnTransform.Rotator());
				SpawnFXAtLocation_Multicast(CurrentProjectilesType.FireFX, SpawnTransform.GetLocation(), SpawnTransform.Rotator());
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
		LastVelocity = Mesh->GetPhysicsLinearVelocity();
		//LastRotation = GetActorQuat();
	}
}

void AFlappyPlane::SetMaxProjectilesByTypeOnWidget_Multicast_Implementation(float MaxProjectiles, TSubclassOf<AProjectile> Type)
{
	//BP
}

void AFlappyPlane::NextProjectilesType_Server_Implementation()
{
	if(CurrentProjectilesType.ProjectileClass)
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
	else
	{
		CurrentProjectilesType = ProjectileTypes[0];
		ProjectilesAmount = CurrentProjectilesType.ProjectilesAmount;
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
		if(OtherActor)
		{
			TArray<AActor*> OverlappingActors;
			OverlapMesh->GetOverlappingActors(OverlappingActors);
			for (AActor* Actor : OverlappingActors)
			{
				if (HitedActor)
				{
					if (Actor == HitedActor)
					{
						return;
					}
				}
				else
				{
					break;
				}
			}

			if (!OtherActor->IsA(AProjectile::StaticClass()))
			{
				//FVector SelfVelocity = Mesh->GetPhysicsLinearVelocity();
				FVector SelfVelocity = LastVelocity;
				if (OtherActor->IsA(AFlappyPlane::StaticClass()))
				{
					if (this < OtherActor) //Logic must run once
					{
						if (AFlappyPlane* OtherPlane = Cast<AFlappyPlane>(OtherActor))
						{
							FVector OtherVelocity = OtherPlane->LastVelocity;
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
								OtherPlane->Mesh->AddImpulse(OverallSpeed * Direction * CollisionImpulceForOther);
								Mesh->AddImpulse(OverallSpeed * Direction * -CollisionImpulceForOther);
								ReceiveDamage(OverallSpeed * CollisionDamageForOther);
								OtherPlane->ReceiveDamage(OverallSpeed * CollisionDamageForOther);
							}
							if (DeltaRotationForSelf > 45 && DeltaRotationForOther > 45)
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
				else
				{
					ReceiveDamage(SelfVelocity.Length() * CollisionDamageForOther);

					//FVector Velocity = Mesh->GetPhysicsLinearVelocity();
					/*FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
					float ImpactAngle = FMath::Acos(FVector::DotProduct(Hit.ImpactNormal, -LastVelocity.GetSafeNormal()));
					FVector ImpulsDirection = LastVelocity.GetSafeNormal() - 2 * FVector::DotProduct(LastVelocity.GetSafeNormal(), SurfaceNormal) * SurfaceNormal;
					Mesh->AddImpulse(ImpulsDirection * CollisionImpulceForSelf);*/
				}
			}
			HitedActor = OtherActor;
		}

		//SetActorRotation(LastRotation);
	}
}

void AFlappyPlane::SpawnFXAtLocation_Multicast_Implementation(UParticleSystem* FX, FVector Location, FRotator Rotation)
{
	if (FX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, Location, Rotation, true);
	}
}

void AFlappyPlane::SpawnSoundAtLocation_Multicast_Implementation(USoundBase* Sound, FVector Location, FRotator Rotation)
{
	if(Sound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location, Rotation, 1);
	}
}
