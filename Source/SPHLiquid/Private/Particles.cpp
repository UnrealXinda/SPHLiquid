// Copyright Epic Games, Inc. All Rights Reserved.

#include "Particles.h"
#include "Async/ParallelFor.h"

void Particles::Advect(float deltaTime)
{
	ParallelFor(GetSize(), [deltaTime](int32 idx)
	{
		m_pos[idx] += m_vel[idx] * deltaTime;
	});
}
