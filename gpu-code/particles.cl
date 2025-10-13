// gpu-code/particles.cl
//
// CellularClusters - OpenCL update step for a simple particle life simulation.
//
// Overview
// --------
// This file contains a single kernel (update_particles) and a helper function
// (computeParticle) that advance positions and velocities of N particles based on
// pairwise attractions/repulsions between 5 species. Each particle is stored as
// a float4 where:
//   x, y = position in pixels (or world units mapped to screen pixels)
//   z, w = velocity components (vx, vy)
//
// Data layout and arguments
// -------------------------
// - particles: __global float4[N] with elements {x, y, vx, vy}
// - species:   __global int[N] species indices in [0,4]
// - width, height: simulation bounds used for boundary collision/bounce
// - dt: time step (seconds-like unit; forces are scaled by dt and an additional 15x factor)
// - N: number of particles
//
// Tuning knobs
// ------------
// - attraction[][]: 5x5 matrix of signed interaction strengths (species-to-species)
// - effectDist: interaction cutoff distance
// - repulsion logic: short-range repulsion to avoid clustering artifacts
// - margin: boundary margin where collisions/bounces occur
// - maxVel: clamp for velocity magnitude per component
//
// Notes
// -----
// - The kernel launches half as many work-items as particles and processes two
//   particles per work-item to increase arithmetic intensity for light loads.
// - Forces are accumulated using a simple distance-normalized model with an
//   early AABB distance test (fabs(dx), fabs(dy)) to skip far pairs cheaply.
// - All math is single-precision to match OpenCL device defaults.

// Species attraction/repulsion table (rows: self species, columns: other species).
// Positive = attraction, Negative = repulsion. Effect table courtesy of Claude.
__constant float attraction[5][5] = { // Effect table courtesy of Claude
// | Red   | Yel    | Blu    | Grn  | Part |
   {2.5f,   -2.5f,  2.789f,  0.3f,  0.1f   },   // Red attractions
   {2.15f,  1.2f,   1.89f,   0.15f, 1.05f  },  // Yellow attractions
   {2.7f,   3.1f,  2.3f,    -0.3f, 2.4f   },   // Blue attractions
   {3.05f,  -0.25f, 1.05f,   1.98f, -0.76f }, // Green attractions
   {0.01f,  3.6f,   1.0f,    -1.6f, 1.7f   }   // Abiotic particle attractions
};

// Boundary handling parameters
__constant float margin = 10.0f;  // Margin from each boundary where collisions are enforced
__constant float maxVel = 1.0f;   // Per-axis velocity clamp

// computeParticle
// ---------------
// Advance a single particle i by computing net force from all other particles,
// then integrating velocity and position with simple damping and boundary bounce.
//
// Parameters:
//   particles  - global buffer of float4 {x, y, vx, vy}
//   species    - global buffer of species indices [0..4]
//   i          - index of the particle to update
//   effectDist - cutoff distance for interaction
//   width      - domain width in pixels/units
//   height     - domain height in pixels/units
//   dt         - time step
//   N          - total particle count
void computeParticle(__global float4* particles,
						__global int* species,
						int i, float effectDist,
						int width, int height,
						float dt, int N) {
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

	// Dampen velocity
	p.z *= 0.89;
	p.w *= 0.89;

	p.z = fmin(p.z, maxVel);
	p.w = fmin(p.w, maxVel);

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

// update_particles kernel
// -----------------------
// Entry point that updates all particles. Each work-item is responsible for up to
// two particles to reduce launch overhead and improve cache locality:
//   childWorkItem0 = gid * 2
//   childWorkItem1 = gid * 2 + 1
// If either index is >= N it is skipped.
//
// Parameters:
//   particles - global buffer of float4 {x, y, vx, vy}
//   species   - global buffer of species indices [0..4]
//   width     - domain width
//   height    - domain height
//   dt        - time step
//   N         - total particle count
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
