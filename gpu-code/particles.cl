// gpu-code/particles.cl

__constant float attraction[5][5] = { // Effect table courtesy of Claude
 // | Red   | Yel    | Blu    | Grn  | Part |
    {2.5f,   -2.5f,  2.789f,  0.3f,  0.1f   },   // Red attractions
    {2.15f,  1.2f,   1.89f,   0.15f, 1.05f  }, // Yellow attractions  
    {2.7f,   3.1f,  2.3f,    -0.3f, 2.4f   },   // Blue attractions
    {3.05f,  -0.25f, 1.05f,   1.98f, -0.76f }, // Green attractions
    {0.01f,  3.6f,   1.0f,    -1.6f, 1.7f   }   // Abiotic particle attractions
};

__constant float margin = 10.0f;
__constant float maxVel = 1.0f;

void computeParticle(__global float4* particles,
								__global int* species,
								int i, float effectDist,
								int width, int height,
								float dt, int N) {
	float4 p = particles[i];
	float fx = 0.0f;
	float fy = 0.0f;

	int selfSpecies = species[i];
	float4 other;
	float dx, dy, dist, dist_recip, force, repulsion; // Only allocate once
	int otherSpecies;

	for (int j = 0; j < N; j++) {
		if (i == j) continue;

		other = particles[j];
		dx = other.x - p.x;
		dy = other.y - p.y;
		if (fabs(dx) > effectDist || fabs(dy) > effectDist) continue;
		dist = sqrt(dx*dx + dy*dy + 1e-5f);

		// Convert dist x and y to direction x and y
		dist_recip = 1.0 / dist;
		dx *= dist_recip;
		dy *= dist_recip;

		otherSpecies = species[j];

		force = attraction[selfSpecies][otherSpecies];
		fx += dx * force;
		fy += dy * force;

		// Distance optimization with help from Claude
		repulsion = 0.0f;
		if (dist < 2.8f && otherSpecies != selfSpecies) repulsion = 6.5f;
		else if (dist < 7.0f) repulsion = 2.5f;

		fx -= dx * repulsion;
		fy -= dy * repulsion;
	}

	// Update velocity
	p.z += fx * dt;
	p.w += fy * dt;

	// Dampen velocity
	p.z *= 0.89;
	p.w *= 0.89;

	p.z = clamp(p.z, -maxVel, maxVel);
	p.w = clamp(p.w, -maxVel, maxVel);

	// Update position
	p.x += p.z * 15.0f * dt;
	p.y += p.w * 15.0f * dt;

	// Clamp and add bounce to the positions & velocities
	if (p.x < margin || p.x > width-margin) {
		p.z *= -0.76f;
		p.x = clamp(p.x, margin, width-margin);
	}
	if (p.y < margin || p.y > height-margin) {
		p.w *= -0.76f;
		p.y = clamp(p.y, margin, height-margin);
	}

	particles[i] = p;
}

__kernel void update_particles(__global float4* particles,
								__global int* species,
								int width, int height,
								float dt, int N) {
	int workItem = get_global_id(0);
	const float effectDist = 30.0f;

	int childWorkItem0 = workItem * 2;
	int childWorkItem1 = workItem * 2 + 1;

	if (childWorkItem0 < N) computeParticle(particles, species, childWorkItem0, effectDist, width, height, dt, N);
	if (childWorkItem1 < N) computeParticle(particles, species, childWorkItem1, effectDist, width, height, dt, N);
}
