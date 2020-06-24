// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class Particles
{
public:
	explicit Particles::Particles(const TArray<FVector>& positions) :
		m_pos(positions)
	{
		m_vel.AddUninitialized(positions.Size());
	}

	Particles(const Particles&) = delete;
	Particles& operator=(const Particles&) = delete;

	FORCEINLINE uint32 GetSize() const
	{
		return m_pos.Num();
	}

	FORCEINLINE FVector* GetPosPtr() const
	{
		return m_pos.GetData();
	}

	FORCEINLINE FVector* GetVelPtr() const
	{
		return m_vel.GetData();
	}

	FORCEINLINE const TArray<FVector>& GetPos() const
	{
		return m_pos;
	}

	virtual void Advect(float deltaTime);

	virtual ~Particles() noexcept { }

protected:

	TArray<FVector> m_pos;
	TArray<FVector> m_vel;
};