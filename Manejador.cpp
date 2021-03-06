/*=================================================
* FileName: Manejador.cpp
* 
* Created by: Gustavo Magallanes Guijón
* Project name: Cardumen
* Unreal Engine version: 4.19.1
* Created on: 2018/04/17
*
* =================================================*/

#include "Manejador.h"
#include "Pez.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Classes/Engine/World.h"


AManejador::AManejador(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AManejador::Tick(float val)
{
	setup();

	if (attachToPlayer)
	{
		moveToPlayer();
	}
}

void AManejador::moveToPlayer()
{
	if (player)
		this->SetActorLocation(player->GetActorLocation());
}

float AManejador::getMinZ()
{
	return minZ;
}

float AManejador::getMaxZ()
{
	return maxZ;
}

void AManejador::setup()
{
	if (isSetup == false){
		if (!areFishSpawned)
		{
			//Se definen los límites de la pecera
			maxX = GetActorLocation().X + limitesPecera;
			maxY = GetActorLocation().Y + limitesPecera;
			minX = GetActorLocation().X - limitesPecera;
			minY = GetActorLocation().Y - limitesPecera;

			UWorld* const world = GetWorld();
			int numFlocks = flockTypes.Num();
			for (int i = 0; i < numFlocks; i++)
			{
				FVector spawnLoc = FVector(FMath::FRandRange(minX, maxX), FMath::FRandRange(minY, maxY), FMath::FRandRange(minZ, maxZ));
				APez *leaderFish = NULL;
				for (int j = 0; j < numInFlock[i]; j++)
				{
					APez *aFish = Cast<APez>(world->SpawnActor(flockTypes[i]));
					aFish->isLeader = false;
					aFish->DebugMode = DebugMode;
					aFish->limitesPeceraMax = FVector(maxX, maxY, maxZ);
					aFish->limitesPeceraMin = FVector(minX, minY, minZ);
					aFish->limitesPecera = limitesPecera;
					spawnLoc = FVector(FMath::FRandRange(minX, maxX), FMath::FRandRange(minY, maxY), FMath::FRandRange(minZ, maxZ));
					if (j == 0)
						//actua si es líder
					{
						aFish->isLeader = true;
						leaderFish = aFish;
					}
					else if (leaderFish != NULL)
					{	
						aFish->leader = leaderFish;
					}
					aFish->SetActorLocation(spawnLoc);
				}
			}
			areFishSpawned = true;
		}

		if (attachToPlayer)
		{
			TArray<AActor*> aPlayerList;
			UGameplayStatics::GetAllActorsOfClass(this, playerType, aPlayerList);
			if (aPlayerList.Num() > 0)
			{	
				player = aPlayerList[0];
				isSetup = true;
			}
		} 
		else
		{
			isSetup = true;
		}

	}
}
