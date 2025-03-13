
#include "FlappyPlane.h"
#include "Kismet/KismetMathLibrary.h"

AFlappyPlane::AFlappyPlane()
{
	PrimaryActorTick.bCanEverTick = true;

	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	RootComponent = PlaneMesh;
}

void AFlappyPlane::BeginPlay()
{
	Super::BeginPlay();
	
	//ForwardSpeed = MinForwardSpeed;
	RotationRate = MinRotationRate;	
	FVector ImpulseDirection = GetActorForwardVector();
	PlaneMesh->AddImpulse(ImpulseDirection * StartImpulse);
	ProjectilesAmount = MaxProjectilesAmount;
}

void AFlappyPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementTick(DeltaTime);
}

void AFlappyPlane::SetIsSpeedUp(bool InbIsSpeedUp)
{
	bIsSpeedUp = InbIsSpeedUp;
	if (bIsSpeedUp)
	{
		bIsFalling = false;
		if(PlaneMesh)
		{
			PlaneMesh->SetEnableGravity(false);
		}
	}
	else
	{
		//ForwardSpeed = MinForwardSpeed;
		if (PlaneMesh)
		{
			PlaneMesh->SetEnableGravity(true);
		}
	}
}

void AFlappyPlane::FillFuel()
{
	Fuel = 1.f;
}

void AFlappyPlane::MovementTick(float DeltaTime)
{

	if (bIsFalling)
	{
		//ForwardSpeed += FallingAcceleration * DeltaTime;	
	}
	FVector LastLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	ForwardVector.X = 0;
	//AddActorWorldOffset(ForwardVector * ForwardSpeed * DeltaTime);
	//PlaneMesh->AddForce(ForwardVector * ForwardSpeed);
	float Speed = PlaneMesh->GetPhysicsLinearVelocity().Length();
	PlaneMesh->AddForce(-PlaneMesh->GetPhysicsLinearVelocity().GetSafeNormal() * Speed * Speed * DragCoefficient);
	FVector LiftVector = GetActorUpVector();
	float LiftForce = 0.5 * Speed * Speed * LiftCoefficient * AirDensity;
	if (bIsSpeedUp && Fuel - FuelConsumption * DeltaTime >= 0)
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
		if (!bNeedToFlip)
		{
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
			RotationRate = UKismetMathLibrary::MapRangeClamped(Speed, MinForwardSpeed, MaxForwardSpeed, MinRotationRate, MaxRotationRate);
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
			PlaneMesh->AddForce(ForwardVector * AccelerationForce);
			Fuel -= FuelConsumption * DeltaTime;
		}
	}
	else
	{
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
			if(bIsMovingForward && FMath::RadiansToDegrees(AngularDistanceToFlipFromForward) < 90 && GetActorRotation().Pitch < 0 
				|| !bIsMovingForward && !(FMath::RadiansToDegrees(AngularDistanceToFlipFromBackward) < 90 && GetActorRotation().Pitch < 0))
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

void AFlappyPlane::SetIsFiring(bool InbIsFiring)
{	
	bIsFiring = InbIsFiring;
	if (InbIsFiring)
	{
		if(!GetWorld()->GetTimerManager().IsTimerActive(FireTimer))
		{
			Fire();
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
				SpawnTransform.SetLocation(GetActorLocation() + FireLocation);
				SpawnTransform.SetRotation(GetActorQuat());
				GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnTransform);
				GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AFlappyPlane::Fire, FireRate, false);

				ProjectilesAmount--;
			}
		}
	}
}
