// gpu-code/particles.cl

__constant float attraction[5][5] = { // Effect table courtesy of Claude
 // | Red   | Yel    | Blu    | Grn  | Part |
    {2.5f,   -2.5f,  2.789f,  0.3f,  0.1f   },   // Red attractions
    {2.15f,  1.2f,   1.89f,   0.15f, 1.05f  }, // Yellow attractions  
    {2.2f,   1.76f,  3.0f,    -0.3f, 2.4f   },   // Blue attractions
    {3.05f,  -0.25f, 1.05f,   1.98f, -0.76f }, // Green attractions
    {0.01f,  3.6f,   1.0f,    -1.6f, 1.7f   }   // Abiotic particle attractions
};

__constant float margin = 3.0f;

__kernel void update_particles(__global float4* particles,
								__global int* species,
								int width, int height,
								float dt, int N) {
	int i = get_global_id(0);
	const float effectDist = 30.0f;

	float4 p = particles[i];
	float fx = 0.0f;
	float fy = 0.0f;

	int selfSpecies = species[i];

	for (int j = 0; j < N; j++) {
		if (i == j) continue;

		float4 other = particles[j];
		float dx = other.x - p.x;
		float dy = other.y - p.y;
		if (fabs(dx) > effectDist || fabs(dy) > effectDist) continue;
		float dist = sqrt(dx*dx + dy*dy + 1e-5f);

		int otherSpecies = species[j];

		float force = attraction[selfSpecies][otherSpecies];
		fx += (dx / dist) * force;
		fy += (dy / dist) * force;

		// Distance optimization with help from Claude
		float repulsion = 0.0f;
		if (dist < 2.8f && otherSpecies != selfSpecies) repulsion = 6.5f;
		else if (dist < 7.0f) repulsion = 2.5f;

		fx -= (dx / dist) * repulsion;
		fy -= (dy / dist) * repulsion;
	}

	// Update velocity
	p.z += fx * dt;
	p.w += fy * dt;

	p.z *= 0.89f;
	p.w *= 0.89f;

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
