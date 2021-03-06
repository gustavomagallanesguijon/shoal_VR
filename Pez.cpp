/*=================================================
* FileName: Pez.cpp
* 
* Created by: Gustavo Magallanes Guijón
* Project name: OceanProject
* Unreal Engine version: 4.18.3
* Created on: 2015/03/17

* =================================================*/

#include "Pez.h"
#include "PezState.h"
#include "Manejador.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


#define COLLISION_TRACE ECC_GameTraceChannel4

APez::APez(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	base = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("PezMesh"));
	RootComponent = base;

	PezInteractionSphere = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("PezInteractionSphere"));
	PezInteractionSphere->SetSphereRadius(10);
	PezInteractionSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	PezInteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APez::OnBeginOverlap);
	PezInteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APez::OnEndOverlap);

	if (isLeader == true)
	{
		spawnTarget();
	}
}

void APez::Tick(float delta)
{

	Setup();

	Debug();

	MoveBounds(delta);

	ManageTimers(delta);

	ChooseState();

	UpdateState(delta);

	this->SetActorRotation(curRotation);
	this->AddActorWorldOffset(curVelocity);

	Super::Tick(delta);
}

void APez::Debug()
{
	if (DebugMode)
	{
		FVector actorLocation = this->GetActorLocation();
		FVector forwardVector = (this->GetActorForwardVector() * AvoidanceDistance) + actorLocation;
		FVector forwardVector2 = (this->GetActorForwardVector() * (AvoidanceDistance * 0.1)) + actorLocation;

		DrawDebugLine(
			GetWorld(),
			actorLocation,
			forwardVector,
			FColor::Magenta,
			false, -1, 0,
			10
			);

		FColor indicatorColor = FColor::Cyan;
		if (Depredarores.IsValidIndex(0))
		{
			indicatorColor = FColor::Red;
		}
		else if (Presas.IsValidIndex(0) && isFull == false)
		{
			indicatorColor = FColor::Green;
		}
		DrawDebugSphere(
			GetWorld(),
			actorLocation,
			PezInteractionSphere->GetScaledSphereRadius(),
			20,
			indicatorColor
			);
		DrawDebugLine(
			GetWorld(),
			actorLocation,
			forwardVector2,
			indicatorColor,
			true, 10, 0,
			20
			);
	}

}

FVector APez::AvoidObstacle()
{
	FVector actorLocation = this->GetActorLocation();
	FVector forwardVector = (this->GetActorForwardVector() * AvoidanceDistance) + actorLocation;

	FHitResult OutHitResult;
	FCollisionQueryParams Line(FName("Collision param"), true);
	bool const bHadBlockingHit = GetWorld()->LineTraceSingleByChannel(OutHitResult, actorLocation, forwardVector, COLLISION_TRACE, Line);
	FVector returnVector = FVector(0, 0, 0);
	float distanceToBound = distanceToBound = (this->GetActorLocation() - OutHitResult.ImpactPoint).Size();
	if (bHadBlockingHit)
	{
		if (OutHitResult.ImpactPoint.Z > this->GetActorLocation().Z + PezInteractionSphere->GetScaledSphereRadius())
		{	
			returnVector.Z += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * -1;
		}
		else if (OutHitResult.ImpactPoint.Z < this->GetActorLocation().Z - PezInteractionSphere->GetScaledSphereRadius())
		{
			returnVector.Z += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * 1;
		}

		if (OutHitResult.ImpactPoint.X > this->GetActorLocation().X)
		{
			returnVector.X += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * -1;
		}
		else if (OutHitResult.ImpactPoint.X < this->GetActorLocation().X)
		{
			
			returnVector.X += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * 1;
		}

		if (OutHitResult.ImpactPoint.Y > this->GetActorLocation().Y)
		{
			returnVector.Y += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * -1;
		}
		else if (OutHitResult.ImpactPoint.Y < this->GetActorLocation().Y)
		{

			returnVector.Y  += (1 / (distanceToBound * (1 / AvoidForceMultiplier))) * 1;
		}

		returnVector.Normalize();
		FVector avoidance = returnVector * AvoidanceForce;
		return avoidance;
	}
	return FVector(0, 0, 0);
}

void APez::UpdateState(float delta)
{
	if (UpdateEveryTick == 0)
	{
		currentState->Update(delta);
	}
	else if (updateTimer >= UpdateEveryTick)
	{
		currentState->Update(delta);
		updateTimer = 0;
	}
}


void APez::OnBeginOverlap(UPrimitiveComponent* activatedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (enemyTypes.Find(otherActor->GetClass()) >= 0)
	{	
		Depredarores.Add(otherActor);
	}
	else if (preyTypes.Find(otherActor->GetClass()) >= 0)
	{	
		if (otherActor->GetClass() == this->GetClass())
		{
			if (!Cast<APez>(otherActor)->isLeader)
			{
				Presas.Add(otherActor);
			}
		}
		else
		{
			Presas.Add(otherActor);
		}
	}
	else if (otherActor->GetClass() == this->GetClass())
	{
		nearbyFriends.Add(otherActor);
	}
}

void APez::OnEndOverlap(UPrimitiveComponent* activatedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	if (Depredarores.Find(otherActor) >= 0)
	{
		Depredarores.Remove(otherActor);
	}
	else if (Presas.Find(otherActor) >= 0)
	{
		Presas.Remove(otherActor);
	}
	else if (nearbyFriends.Find(otherActor) >= 0)
	{
		nearbyFriends.Remove(otherActor);
	}
}

void APez::ChooseState()
{
	if (Depredarores.IsValidIndex(0))
	{
		currentState = new FleeState(this, Depredarores[0]);
	}
	else if (Presas.IsValidIndex(0) && isFull == false)
	{
		currentState = new ChaseState(this, Presas[0]);
	}
	else
	{
		currentState = new SeekState(this);
	}
}

void APez::ManageTimers(float delta)
{
	if (isFull)
	{
		hungerTimer += delta;

		if (hungerTimer >= hungerResetTime)
		{
			hungerTimer = 0.0f;
			isFull = false;
		}
	}

	if (turnTimer >= turnFrequency && isLeader == true)
	{
		spawnTarget();
		turnTimer = 0.0;
	}

	updateTimer += delta;
	turnTimer += delta;
}



void APez::MoveBounds(float delta)
{
	if (hasManejador)
	{
		FVector manejadorPosition = manejador->GetActorLocation();
		maxX = manejadorPosition.X + limitesPecera;
		minX = manejadorPosition.X - limitesPecera;
		maxY = manejadorPosition.Y + limitesPecera;
		minY = manejadorPosition.Y - limitesPecera;

		FVector actorLocation = this->GetActorLocation();
		if (actorLocation.Z > limitesPeceraMax.Z)
		{	
			actorLocation.Z = limitesPeceraMin.Z + FMath::Abs((0.999 * limitesPeceraMax.Z));
		}
		else if (actorLocation.Z < limitesPeceraMin.Z)
		{
			actorLocation.Z = limitesPeceraMin.Z + FMath::Abs((0.001 * limitesPeceraMax.Z));
		}

		if (actorLocation.X > maxX)
		{
			actorLocation.X = minX + FMath::Abs((0.1 * maxX));
		}
		else if (actorLocation.X < minX)
		{
			actorLocation.X = maxX - FMath::Abs((0.1 * maxX));
		}

		if (actorLocation.Y > maxY)
		{
			actorLocation.Y = minY + FMath::Abs((0.1 * maxY));
		}
		else if (actorLocation.Y < minY)
		{
			actorLocation.Y = maxY - FMath::Abs((0.1 * maxY));
		}

		this->SetActorLocation(actorLocation);
	}
}

void APez::spawnTarget()
{
	target = FVector(FMath::FRandRange(minX, maxX), FMath::FRandRange(minY, maxY), FMath::FRandRange(minZ, maxZ));
}


void APez::Setup()
{
	if (isSetup == false)
	{
		maxX = limitesPeceraMax.X;
		maxY = limitesPeceraMax.Y;
		minX = limitesPeceraMin.X;
		minY = limitesPeceraMin.Y;

		InteractionSphereRadius = PezInteractionSphere->GetScaledSphereRadius();

		if (CustomZSeekMax == 0.0)
		{
			minZ = limitesPeceraMin.Z;
			maxZ = limitesPeceraMax.Z;
		}
		else
		{
			minZ = CustomZSeekMin;
			maxZ = CustomZSeekMax;
		}

		fleeDistance = PezInteractionSphere->GetScaledSphereRadius() * FleeDistanceMultiplier;
		neighborSeperation = PezInteractionSphere->GetScaledSphereRadius() * SeperationDistanceMultiplier;
		AvoidanceDistance = PezInteractionSphere->GetScaledSphereRadius() * 2;

		currentState = new SeekState(this);

		TArray<AActor*> aManejadorList;
		UGameplayStatics::GetAllActorsOfClass(this, AManejador::StaticClass(), aManejadorList);
		if (aManejadorList.Num() > 0)
		{
			hasManejador = true;
			manejador = aManejadorList[0];
		}

		if (!manejador)
		{
			TArray<AActor*> aNeighborList;
			UGameplayStatics::GetAllActorsOfClass(this, neighborType, aNeighborList);
			neighbors.Append(aNeighborList);
			for (int i = 0; i < neighbors.Num(); i++)
			{
				if (Cast<APez>(neighbors[i])->isLeader)
				{
					leader = neighbors[i];
					break;
				}
			}
		}


		isSetup = true;
	}
}
