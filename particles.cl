// gpu-code/particles.cl

__kernel void update_particles(__global float4* particles,
                               __global float* species,
                               int width, int height,
                               float dt, int N) {
	int i = get_global_id(0);

	float4 p = particles[i];
	float fx = 0.0f;
	float fy = 0.0f;

	for (int j = 0; j < N; j++) {
		if (i == j) continue;

		float4 other = particles[j];
		float dx = other.x - p.x;
		float dy = other.y - p.y;
		float dist = sqrt(dx*dx + dy*dy) + 1e-5f;
		if (dist > 64) continue;

		float selfSpecies = species[i];
		float otherSpecies = species[j];

		switch ((int)selfSpecies) {
			case 0: // Red particle (cytoplasm/mover)
				if (otherSpecies == 0) { 
					fx += (dx / dist) * 2.5f; // strong attraction to red
					fy += (dy / dist) * 2.5f;
				} else {
					fx += (dx / dist) * 0.2f; // weak attraction to others
					fy += (dy / dist) * 0.2f;
				}
				break;

			case 1: // Yellow particle (cytoplasm/mover)
				if (otherSpecies == 1) {
					fx += (dx / dist) * 2.2f; // strong cohesion
					fy += (dy / dist) * 2.2f;
				} else {
					fx += (dx / dist) * 0.15f;
					fy += (dy / dist) * 0.15f;
				}
				break;

			case 2: // Blue particle (cell wall)
				if (otherSpecies == 2) {
					fx += (dx / dist) * 3.0f; // very cohesive wall
					fy += (dy / dist) * 3.0f;
				} else {
					fx += (dx / dist) * 0.1f; // minimal attraction to others
					fy += (dy / dist) * 0.1f;
				}
				break;

			case 3: // Green particle (nucleus)
				if (otherSpecies == 3) {
					fx += (dx / dist) * 1.5f; // nucleus cohesion
					fy += (dy / dist) * 1.5f;
				} else {
					fx += (dx / dist) * 0.05f; // very weak interaction
					fy += (dy / dist) * 0.05f;
				}
				break;

			default: break;
		}

		if (dist < 2.0f) { // strong short-range repulsion for all
			fx -= (dx / dist) * 1.0f;
			fy -= (dy / dist) * 1.0f;
		}

		else if (dist < 5.0f && otherSpecies != selfSpecies) { // strong short-range repulsion for all
			fx -= (dx / dist) * 7.5f;
			fy -= (dy / dist) * 7.5f;
		}
	}
	
	// Update velocity
	p.z += fx * dt;
	p.w += fy * dt;

	p.z *= 0.99f;
	p.w *= 0.99f;

	// Update position
	p.x += p.z * 20.0f * dt;
	p.y += p.w * 20.0f * dt;

	p.x = fmod(p.x + width, width);
	p.y = fmod(p.y + height, height);

	particles[i] = p;
}
