/*=================================================
* FileName: PezState.h
* 
* Created by: Gustavo Magallanes Guijón
* Project name: Cardumen
* Unreal Engine version: 4.19.1
* Created on: 2018/04/17
*
* =================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Pez.h"


class APez;


class PezState
{
protected:
	APez* Fish;

public:
	// Constructors
	PezState(){}
	PezState(APez* aFish)
	{
		Fish = aFish;
	};
	virtual void Update(float val){};
	virtual void HandleOverlapEvent(AActor *otherActor, UPrimitiveComponent *otherComponent, FString aColliderString){};
};


class SeekState : public PezState
{
public:

	SeekState(APez* aFish) : PezState(aFish){};
	virtual void Update(float delta) override;

protected:

	virtual void SeekTarget(float delta);
	virtual void Flock(float delta);
};



class FleeState : public PezState
{
protected:
	AActor* Enemy;

public:

	FleeState(APez* aFish, AActor* aEnemy) : PezState(aFish)
	{
		Enemy = aEnemy;
	};
	virtual void Update(float delta) override;

protected:

	virtual void FleeFromEnemy(float delta);

};



class ChaseState : public PezState
{
protected:
	AActor* Presa;

public:

	ChaseState(APez* aFish, AActor* aPresa) : PezState(aFish)
	{
		Presa = aPresa;
	};
	virtual void Update(float delta) override;

protected:

	virtual void ChasePresa(float delta);
	virtual void EatPresa();
};
