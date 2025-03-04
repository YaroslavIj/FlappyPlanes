
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
}

void AFlappyPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	if (bIsSpeedUp)
	{
		if(!bNeedToFlip)
		{
			PlaneMesh->AddForce(ForwardVector * AccelerationForce);
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

			if (AngularDistanceToInverted < AngularDistanceToAligned)
			{
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
					if (AngularDistanceToInverted - FMath::DegreesToRadians(AlignSpeed) * DeltaTime < 0)
					{
						bIsFalling = true;
						RotationQuat = FQuat(RotationAxis, AngularDistanceToInverted);
					}
					else
					{
						RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(AlignSpeed) * DeltaTime);
					}
					AddActorWorldRotation(RotationQuat);
				}

			}
			else
			{
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
					if (AngularDistanceToAligned - FMath::DegreesToRadians(AlignSpeed) * DeltaTime < 0)
					{
						bIsFalling = true;
						RotationQuat = FQuat(RotationAxis, AngularDistanceToAligned);
					}
					else
					{
						RotationQuat = FQuat(RotationAxis, -FMath::DegreesToRadians(AlignSpeed) * DeltaTime);
					}
					AddActorWorldRotation(RotationQuat);
				}
			}
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
		
		if(CurrentFlipRotation + FlipSpeed * DeltaTime >= 180.f)
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
