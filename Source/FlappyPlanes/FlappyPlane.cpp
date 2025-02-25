
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
	
	ForwardSpeed = MinForwardSpeed;
	RotationRate = MinRotationRate;	
}

void AFlappyPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector LastLocation = GetActorLocation();
	//LastLocation.Z = 0;

	FVector ForwardVector = GetActorForwardVector();
	ForwardVector.X = 0;
	AddActorWorldOffset(ForwardVector * ForwardSpeed * DeltaTime);

	if (bIsSpeedUp/* && !bIsAligning*/)
	{		
		if (ForwardSpeed + AccelerationSpeed * DeltaTime < MaxForwardSpeed)
		{
			ForwardSpeed += AccelerationSpeed * DeltaTime;
		}
		else
		{
			ForwardSpeed = MaxForwardSpeed;
		}
		FVector CurrentLocation = GetActorLocation();
		//CurrentLocation.Z = 0;

		float Speed = FVector(CurrentLocation - LastLocation).Length();
		RotationRate = UKismetMathLibrary::MapRangeClamped(Speed, MinForwardSpeed* DeltaTime, MaxForwardSpeed * DeltaTime, MinRotationRate, MaxRotationRate);
/*		if (RotationRate + RotationAcceleration * DeltaTime < MaxRotationRate)
		{
			RotationRate += RotationAcceleration * DeltaTime;
		}
		else
		{
			RotationRate = MaxRotationRate;
		}*/
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
	}
	else
	{
		FQuat CurrentOrientation = GetActorQuat();
		CurrentOrientation.Normalize();
		FQuat TargetOrientationInverted = FQuat(FRotator(-45, -90, -180));
		FQuat TargetOrientationAligned = FQuat(FRotator(-45, 90, 0));
		//if(bIsMovingForward)
		//{
		//	//TargetOrientationInverted = FQuat(FRotator(0, -90, -180));
		//	TargetOrientationInverted = FQuat(FRotator(0, -90, 0));
		//	TargetOrientationAligned = FQuat(FRotator(0, 90, 0));
		//}
		//else
		//{
		//	TargetOrientationInverted = FQuat(FRotator(0, -90, 0));
		//	TargetOrientationAligned = FQuat(FRotator(0, 90, 180));
		//}
		TargetOrientationInverted.Normalize();
		
		TargetOrientationAligned.Normalize();
		float DotProductInverted = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationInverted.Vector());
		float AngularDistanceToInverted = FMath::Acos(FMath::Clamp(DotProductInverted, -1.0f, 1.0f));
		float DotProductAligned = FVector::DotProduct(CurrentOrientation.Vector(), TargetOrientationAligned.Vector());
		float AngularDistanceToAligned = FMath::Acos(FMath::Clamp(DotProductAligned, -1.0f, 1.0f));
		if (AngularDistanceToInverted < AngularDistanceToAligned)
		{
			if(bIsMovingForward)
			{
				bIsMovingForward = false;
				bIsTurnedUpsideDown = true;
			}
			/*if(bIsMovingForward)
			{
				TargetOrientationInverted = FQuat(FRotator(0, -90, 0));
			}
			else
			{
				TargetOrientationInverted = FQuat(FRotator(0, -90, -180));
			}	*/		
			/*		if (FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
			{
				bIsMovingForward = false;
				if (bIsAligning)
				{
					bIsAligning = false;
				}
			}*/
			if (!FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
			{
				if (!bIsAligning)
				{
					//bIsAligning = true;
				}
				FQuat NewRotation;
				NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationInverted, AlignSpeed * DeltaTime);
				SetActorRotation(NewRotation);
			}

		}
		else
		{
			if(!bIsMovingForward)
			{
				bIsMovingForward = true;
				bIsTurnedUpsideDown = true;
			}

			/*if(bIsMovingForward)
			{
				TargetOrientationAligned = FQuat(FRotator(0, 90, 0));
			}
			else
			{
				TargetOrientationAligned = FQuat(FRotator(0, 90, 180));
			}	*/	
		    /* if (FMath::IsNearlyZero(AngularDistanceToAligned, 0.1f))
			{
				bIsMovingForward = true;
				if(bIsAligning)
				{
					bIsAligning = false;
				}
			}*/
			if (!FMath::IsNearlyZero(AngularDistanceToInverted, 0.1f))
			{
				if(!bIsAligning)
				{
					//bIsAligning = true;
				}
				FQuat NewRotation;
				NewRotation = FQuat::Slerp(CurrentOrientation, TargetOrientationAligned, AlignSpeed * DeltaTime);
				SetActorRotation(NewRotation);
			}
	
		}		
	}
	if(bIsTurnedUpsideDown)
	{
		if (bIsMovingForward)
		{
			
		}
		else
		{

		}	
		FQuat CurrentQuat = GetActorQuat();
		FVector RotationAxis = GetActorForwardVector();
		FQuat TargetQuat = FQuat(RotationAxis, FMath::DegreesToRadians(180)) * CurrentQuat;
		float DotProduct = FVector::DotProduct(CurrentQuat.Vector(), TargetQuat.Vector());
		float AngularDistance = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
		/*if (FMath::IsNearlyZero(AngularDistance, 0.1f))
		{
			bIsTurnedUpsideDown = false;
		}*/
		//else
		{
			FQuat NewRotation = FQuat::Slerp(CurrentQuat, TargetQuat, CoupSpeed * DeltaTime);
			SetActorRotation(NewRotation);
		}
	}
}

void AFlappyPlane::SetIsSpeedUp(bool InbIsSpeedUp)
{
	bIsSpeedUp = InbIsSpeedUp;
	if (bIsSpeedUp)
	{
		if(PlaneMesh)
		{
			PlaneMesh->SetEnableGravity(false);
			//PlaneMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			//PlaneMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}
	else
	{
		ForwardSpeed = MinForwardSpeed;
		if (PlaneMesh)
		{
			PlaneMesh->SetEnableGravity(true);
		}
	}
}
