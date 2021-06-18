#include "FlexSystem.h"

void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line)
{
	printf("Flex: %s - %s:%d\n", msg, file, line);
}

void VMesh::GetBounds(Vector3& outMinExtents, Vector3& outMaxExtents) const
{
	Point3 minExtents(FLT_MAX);
	Point3 maxExtents(-FLT_MAX);

	// calculate face bounds
	for (uint32_t i = 0; i < m_positions.size(); ++i)
	{
		const Point3& a = Point3(m_positions[i].x, m_positions[i].y, m_positions[i].z);

		minExtents = Min(a, minExtents);
		maxExtents = Max(a, maxExtents);
	}

	outMinExtents = Vector3(minExtents);
	outMaxExtents = Vector3(maxExtents);
}

SimBuffers::SimBuffers(NvFlexLibrary* l) :
	positions(l), positionsGpu(l, 0, eNvFlexBufferDevice), restPositions(l), velocities(l), phases(l), densities(l),
	anisotropy1(l, 0, eNvFlexBufferDevice), anisotropy2(l, 0, eNvFlexBufferDevice), anisotropy3(l, 0, eNvFlexBufferDevice), normals(l), normalsGpu(l, 0, eNvFlexBufferDevice), smoothPositions(l, 0, eNvFlexBufferDevice),
	diffusePositions(l), diffuseVelocities(l), diffuseCount(l), activeIndices(l),
	shapeGeometry(l), shapePositions(l), shapeRotations(l), shapePrevPositions(l),
	shapePrevRotations(l), shapeFlags(l), inflatableTriOffsets(l),
	inflatableTriCounts(l), inflatableVolumes(l), inflatableCoefficients(l),
	inflatablePressures(l), springIndices(l), springLengths(l),
	springStiffness(l), triangles(l), triangleNormals(l), uvs(l), uvsGpu(l, 0, eNvFlexBufferDevice),
	contactPlanes(l), contactVelocities(l), contactIndices(l), contactCounts(l), trianglesGpu(l, 0, eNvFlexBufferDevice), triangleNormalsGpu(l, 0, eNvFlexBufferDevice)
{

}

SimBuffers::~SimBuffers() 
{
	// particles
	positions.destroy();
	positionsGpu.destroy();
	restPositions.destroy();
	velocities.destroy();
	phases.destroy();
	densities.destroy();
	anisotropy1.destroy();
	anisotropy2.destroy();
	anisotropy3.destroy();
	normals.destroy();
	normalsGpu.destroy();
	diffusePositions.destroy();
	diffuseVelocities.destroy();
	diffuseCount.destroy();
	smoothPositions.destroy();
	activeIndices.destroy();

	// convexes
	shapeGeometry.destroy();
	shapePositions.destroy();
	shapeRotations.destroy();
	shapePrevPositions.destroy();
	shapePrevRotations.destroy();
	shapeFlags.destroy();

	// springs
	springIndices.destroy();
	springLengths.destroy();
	springStiffness.destroy();

	// inflatables
	inflatableTriOffsets.destroy();
	inflatableTriCounts.destroy();
	inflatableVolumes.destroy();
	inflatableCoefficients.destroy();
	inflatablePressures.destroy();

	// triangles
	triangles.destroy();
	trianglesGpu.destroy();
	triangleNormals.destroy();
	triangleNormalsGpu.destroy();
	uvs.destroy();
	uvsGpu.destroy();
}

void SimBuffers::MapBuffers() 
{
	// particles
	positions.map();
	positionsGpu.map();
	restPositions.map();
	velocities.map();
	phases.map();
	densities.map();
	anisotropy1.map();
	anisotropy2.map();
	anisotropy3.map();
	normals.map();
	normalsGpu.map();
	diffusePositions.map();
	diffuseVelocities.map();
	diffuseCount.map();
	smoothPositions.map();
	activeIndices.map();

	// convexes
	shapeGeometry.map();
	shapePositions.map();
	shapeRotations.map();
	shapePrevPositions.map();
	shapePrevRotations.map();
	shapeFlags.map();

	springIndices.map();
	springLengths.map();
	springStiffness.map();

	// inflatables
	inflatableTriOffsets.map();
	inflatableTriCounts.map();
	inflatableVolumes.map();
	inflatableCoefficients.map();
	inflatablePressures.map();

	// triangles
	triangles.map();
	trianglesGpu.map();
	triangleNormals.map();
	triangleNormalsGpu.map();
	uvs.map();
	uvsGpu.map();

	// contacts
	contactPlanes.map();
	contactVelocities.map();
	contactIndices.map();
	contactCounts.map();
}

void SimBuffers::UnmapBuffers() 
{
	// particles
	positions.unmap();
	positionsGpu.unmap();
	restPositions.unmap();
	velocities.unmap();
	phases.unmap();
	densities.unmap();
	anisotropy1.unmap();
	anisotropy2.unmap();
	anisotropy3.unmap();
	normals.unmap();
	normalsGpu.unmap();
	diffusePositions.unmap();
	diffuseVelocities.unmap();
	diffuseCount.unmap();
	smoothPositions.unmap();
	activeIndices.unmap();

	// convexes
	shapeGeometry.unmap();
	shapePositions.unmap();
	shapeRotations.unmap();
	shapePrevPositions.unmap();
	shapePrevRotations.unmap();
	shapeFlags.unmap();

	// springs
	springIndices.unmap();
	springLengths.unmap();
	springStiffness.unmap();

	// inflatables
	inflatableTriOffsets.unmap();
	inflatableTriCounts.unmap();
	inflatableVolumes.unmap();
	inflatableCoefficients.unmap();
	inflatablePressures.unmap();

	// triangles
	triangles.unmap();
	trianglesGpu.unmap();
	triangleNormals.unmap();
	triangleNormalsGpu.unmap();
	uvs.unmap();
	uvsGpu.unmap();

	//contacts
	contactPlanes.unmap();
	contactVelocities.unmap();
	contactIndices.unmap();
	contactCounts.unmap();
}

void SimBuffers::InitBuffers() 
{
	// particles
	positions.resize(0);
	positionsGpu.resize(0);
	restPositions.resize(0);
	velocities.resize(0);
	phases.resize(0);
	diffusePositions.resize(0);
	diffuseVelocities.resize(0);
	diffuseCount.resize(0);

	normals.resize(0);
	normalsGpu.resize(0);

	// springs
	springIndices.resize(0);
	springLengths.resize(0);
	springStiffness.resize(0);
	
	// triangles
	triangles.resize(0);
	triangleNormals.resize(0);
	uvs.resize(0);
	uvsGpu.resize(0);

    // shapes
	shapeGeometry.resize(0);
	shapePositions.resize(0);
	shapeRotations.resize(0);
	shapePrevPositions.resize(0);
	shapePrevRotations.resize(0);
	shapeFlags.resize(0);

	//contacts
	contactPlanes.resize(0);
	contactVelocities.resize(0);
	contactIndices.resize(0);
	contactCounts.resize(0);
}

FlexSystem::FlexSystem()
{	
	g_profile = false;
	nEmitter = 0;
	nVolumeBoxes = 0;
	maxParticles = 9;
	g_maxDiffuseParticles = 60;
	numDiffuse = 64;
	g_solver = nullptr;
	cursor = 0;
	nFields = 0;
	clothMesh0 = nullptr;
	g_triangleCollisionMesh = nullptr;
	memset(&g_timers, 0, sizeof(g_timers));
}

FlexSystem::~FlexSystem()
{
	if (g_solver)
	{
		if (g_buffers) 
		{
			delete g_buffers;
		}
		
		NvFlexDestroySolver(g_solver);
		NvFlexShutdown(g_flexLib);
	}

	NvFlexDeviceDestroyCudaContext();
}

void FlexSystem::getSimTimers() 
{
	if (g_profile) 
	{
		memset(&g_timers, 0, sizeof(g_timers));
		NvFlexGetTimers(g_solver, &g_timers);
		simLatency = g_timers.total;
	}
	else 
	{
		simLatency = NvFlexGetDeviceLatency(g_solver, &g_GpuTimers.computeBegin, &g_GpuTimers.computeEnd, &g_GpuTimers.computeFreq);
	}	
}

void FlexSystem::initSystem() 
{
	//int g_device = -1;
	g_device = NvFlexDeviceGetSuggestedOrdinal();

	// Create an optimized CUDA context for Flex and set it on the 
	// calling thread. This is an optional call, it is fine to use 
	// a regular CUDA context, although creating one through this API
	// is recommended for best performance.
	bool success = NvFlexDeviceCreateCudaContext(g_device);

	if (!success)
	{
		printf("Error creating CUDA context.\n");
	}

	NvFlexInitDesc desc;
	desc.deviceIndex = g_device;
	desc.enableExtensions = true;
	desc.renderDevice = 0;
	desc.renderContext = 0;
	desc.computeType = eNvFlexCUDA;

	g_flexLib = NvFlexInit(NV_FLEX_VERSION, ErrorCallback, &desc);
}

void FlexSystem::initParams() 
{
	g_params.gravity[0] = 0.0f;
	g_params.gravity[1] = -9.8f;
	g_params.gravity[2] = 0.0f;

	g_params.wind[0] = 0.0f;
	g_params.wind[1] = 0.0f;
	g_params.wind[2] = 0.0f;
	g_params.drag = 0.0f;
	g_params.lift = 0.0f;

	g_params.radius = 0.15f;
	g_params.viscosity = 0.0f;
	g_params.dynamicFriction = 0.0f;
	g_params.staticFriction = 0.0f;
	g_params.particleFriction = 0.0f; // scale friction between particles by default
	g_params.freeSurfaceDrag = 0.0f;
	g_params.drag = 0.0f;
	g_params.lift = 0.0f;
	g_params.numIterations = 3;
	g_params.fluidRestDistance = 0.0f;
	g_params.solidRestDistance = 0.0f;

	g_params.anisotropyScale = 1.0f;
	g_params.anisotropyMin = 0.1f;
	g_params.anisotropyMax = 2.0f;
	g_params.smoothing = 1.0f;

	g_params.dissipation = 0.0f;
	g_params.damping = 0.0f;
	g_params.particleCollisionMargin = 0.0f;
	g_params.shapeCollisionMargin = 0.0f;
	g_params.collisionDistance = 0.0f;
	g_params.sleepThreshold = 0.0f;
	g_params.shockPropagation = 0.0f;
	g_params.restitution = 0.001f;

	g_params.maxSpeed = FLT_MAX;
	g_params.maxAcceleration = 100.0f;	// approximately 10x gravity

	g_params.relaxationMode = eNvFlexRelaxationLocal;
	g_params.relaxationFactor = 1.0f;
	g_params.solidPressure = 1.0f;
	g_params.adhesion = 0.0f;
	g_params.cohesion = 0.1f;
	g_params.surfaceTension = 0.0f;
	g_params.vorticityConfinement = 80.0f;
	g_params.buoyancy = 1.0f;
	g_params.diffuseThreshold = 100.0f;
	g_params.diffuseBuoyancy = 1.0f;
	g_params.diffuseDrag = 0.8f;
	g_params.diffuseBallistic = 16;
	g_params.diffuseLifetime = 2.0f;

	g_params.numPlanes = 0;

	callback = nullptr;
}

void FlexSystem::initScene() 
{
	RandInit();
	cursor = 0;

	if (g_solver)
	{
		if (g_buffers) 
		{
			delete g_buffers;
		}

		NvFlexDestroySolver(g_solver);
		g_solver = nullptr;
	}

	g_buffers = new SimBuffers(g_flexLib);

	g_buffers->MapBuffers();
	g_buffers->InitBuffers();

	g_rectEmitters.resize(0);
	g_volumeBoxes.resize(0);

	initParams();

	g_numSubsteps = 2;
	g_sceneLower = FLT_MAX;
	g_sceneUpper = -FLT_MAX;

	NvFlexSetSolverDescDefaults(&g_solverDesc);
	ClearShapes();

	maxParticles = 64;
	g_maxDiffuseParticles = 0;
	g_maxNeighborsPerParticle = 96;
	g_maxContactsPerParticle = 6;
}

void FlexSystem::postInitScene()
{
	g_solverDesc.featureMode = eNvFlexFeatureModeSimpleSolids;// eNvFlexFeatureModeSimpleFluids;
	g_params.fluidRestDistance = g_params.radius * 0.65f;

	if (g_params.solidRestDistance == 0.0f) 
	{
		g_params.solidRestDistance = g_params.radius;
	}

	if (g_params.fluidRestDistance > 0.0f) 
	{
		g_params.solidRestDistance = g_params.fluidRestDistance;
	}

	if (g_params.collisionDistance == 0.0f) 
	{
		g_params.collisionDistance = Max(g_params.solidRestDistance, g_params.fluidRestDistance) * 0.5f;
	}

	if (g_params.particleFriction == 0.0f) 
	{
		g_params.particleFriction = g_params.dynamicFriction * 0.1f;
	}

	if (g_params.shapeCollisionMargin == 0.0f) 
	{
		g_params.shapeCollisionMargin = g_params.collisionDistance * 0.5f;
	}

	g_maxDiffuseParticles = 5000;

	for (int i = 0; i < nVolumeBoxes; i++) 
	{
		CreateCenteredParticleGrid(Point3(g_volumeBoxes[i].mPos.x, g_volumeBoxes[i].mPos.y, g_volumeBoxes[i].mPos.z), g_volumeBoxes[i].mRot, Point3(g_volumeBoxes[i].mSize.x, g_volumeBoxes[i].mSize.y, g_volumeBoxes[i].mSize.z), g_params.fluidRestDistance, Vec3(0.0f), 1, false, NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid), g_params.fluidRestDistance * 0.01f);
	}

	g_params.anisotropyScale = 1.0f;

	if (g_triangleCollisionMesh) 
	{
		triangleCollisionMeshId = CreateTriangleMesh(g_triangleCollisionMesh);
	}

	uint32_t numParticles = g_buffers->positions.size(); //non zero if init volume boxes

	if (g_buffers->positions.size())
	{
		g_buffers->activeIndices.resize(numParticles);
		for (size_t i = 0; i < g_buffers->activeIndices.size(); ++i)
		{
			g_buffers->activeIndices[i] = i;
		}
	}

	g_inactiveIndices.resize(size_t(maxParticles) - numParticles);

	for (size_t i = 0; i < g_inactiveIndices.size(); ++i) 
	{
		g_inactiveIndices[i] = i + numParticles;
	}
	
	g_buffers->smoothPositions.resize(maxParticles);
	g_buffers->normals.resize(0);
	g_buffers->normals.resize(maxParticles);
	g_buffers->normalsGpu.resize(0);
	g_buffers->normalsGpu.resize(maxParticles);
	g_buffers->positions.resize(maxParticles);
	g_buffers->positionsGpu.resize(maxParticles);
	g_buffers->velocities.resize(maxParticles);
	g_buffers->phases.resize(maxParticles);
	g_buffers->densities.resize(maxParticles);
	g_buffers->anisotropy1.resize(maxParticles);
	g_buffers->anisotropy2.resize(maxParticles);
	g_buffers->anisotropy3.resize(maxParticles);

	// diffuse
	g_buffers->diffusePositions.resize(g_maxDiffuseParticles);
	g_buffers->diffuseVelocities.resize(g_maxDiffuseParticles);
	g_buffers->diffuseCount.resize(g_maxDiffuseParticles);

	// triangles
	g_buffers->triangles.resize(maxParticles);
	g_buffers->trianglesGpu.resize(maxParticles);
	g_buffers->triangleNormals.resize(maxParticles);
	g_buffers->triangleNormalsGpu.resize(maxParticles);
	g_buffers->uvs.resize(maxParticles);
	g_buffers->uvsGpu.resize(maxParticles);

	// contacts
	const int maxContactsPerParticle = 6;
	g_buffers->contactPlanes.resize(maxParticles * maxContactsPerParticle);
	g_buffers->contactVelocities.resize(maxParticles * maxContactsPerParticle);
	g_buffers->contactIndices.resize(maxParticles);
	g_buffers->contactCounts.resize(maxParticles);

	// rest positions
	g_buffers->restPositions.resize(g_buffers->positions.size());
	for (size_t j = 0; j < g_buffers->positions.size(); ++j)
	{
		g_buffers->restPositions[j] = g_buffers->positions[j];
	}

	g_solverDesc.maxParticles = maxParticles;
	g_solverDesc.maxDiffuseParticles = g_maxDiffuseParticles;
	g_solverDesc.maxNeighborsPerParticle = g_maxNeighborsPerParticle;
	g_solverDesc.maxContactsPerParticle = g_maxContactsPerParticle;

	g_solver = NvFlexCreateSolver(g_flexLib, &g_solverDesc);

	g_buffers->UnmapBuffers();

	if (g_buffers->activeIndices.size()) 
	{
		NvFlexSetActive(g_solver, g_buffers->activeIndices.buffer, NULL);
	}
	
	if (g_buffers->positions.size()) 
	{
		NvFlexSetParticles(g_solver, g_buffers->positions.buffer, NULL);
		NvFlexSetVelocities(g_solver, g_buffers->velocities.buffer, NULL);
		NvFlexSetPhases(g_solver, g_buffers->phases.buffer, NULL);
		NvFlexSetNormals(g_solver, g_buffers->normals.buffer, NULL);
	}
	
	if (g_buffers->triangles.size()) 
	{
		NvFlexSetDynamicTriangles(g_solver, g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);
	}

	if (g_buffers->springIndices.size())
	{
		assert((g_buffers->springIndices.size() & 1) == 0);
		assert((g_buffers->springIndices.size() / 2) == g_buffers->springLengths.size());
		NvFlexSetSprings(g_solver, g_buffers->springIndices.buffer, g_buffers->springLengths.buffer, g_buffers->springStiffness.buffer, g_buffers->springLengths.size());
	}

	if (g_buffers->inflatableTriOffsets.size())
	{
		NvFlexSetInflatables(g_solver, g_buffers->inflatableTriOffsets.buffer, g_buffers->inflatableTriCounts.buffer, g_buffers->inflatableVolumes.buffer, g_buffers->inflatablePressures.buffer, g_buffers->inflatableCoefficients.buffer, g_buffers->inflatableTriOffsets.size());
	}

	setShapes();

	NvFlexSetParams(g_solver, &g_params);
	NvFlexUpdateSolver(g_solver, g_dt, g_numSubsteps, g_profile);

	if (callback != nullptr)
	{
		NvFlexExtDestroyForceFieldCallback(callback);
	}

	callback = NvFlexExtCreateForceFieldCallback(g_solver);
}

void FlexSystem::updateParams(const OP_Inputs* inputs)
{
	double gravity[3];
	double wind[3];

	g_params.maxSpeed = (float)inputs->getParDouble("Maxspeed");
	g_numSubsteps = inputs->getParInt("Numsubsteps");
	g_params.numIterations = inputs->getParInt("Numiterations");

	g_dt = (float) 1.0 / inputs->getParDouble("Fps");
	g_params.maxSpeed = 0.5f * g_params.radius * g_numSubsteps / g_dt;

	inputs->getParDouble3("Gravity", gravity[0], gravity[1], gravity[2]);
	g_params.gravity[0] = (float)gravity[0];
	g_params.gravity[1] = (float)gravity[1];
	g_params.gravity[2] = (float)gravity[2];

	inputs->getParDouble3("Wind", wind[0], wind[1], wind[2]);
	g_params.wind[0] = (float)wind[0];
	g_params.wind[1] = (float)wind[1];
	g_params.wind[2] = (float)wind[2];

	g_params.drag = (float)inputs->getParDouble("Dragcloth");
	g_params.lift = (float)inputs->getParDouble("Lift");
	g_params.dynamicFriction = (float)inputs->getParDouble("Dynamicfriction");
	g_params.restitution = (float)inputs->getParDouble("Restitution");
	g_params.adhesion = (float)inputs->getParDouble("Adhesion");
	g_params.dissipation = (float)inputs->getParDouble("Dissipation");
	g_params.cohesion = (float)inputs->getParDouble("Cohesion");
	g_params.surfaceTension = (float)inputs->getParDouble("Surfacetension");
	g_params.viscosity = (float)inputs->getParDouble("Viscosity");
	g_params.vorticityConfinement = (float)inputs->getParDouble("Vorticityconfinement");
	g_params.damping = (float)inputs->getParDouble("Damping");
	g_params.radius = (float)inputs->getParDouble("Radius");
	maxParticles = inputs->getParInt("Maxparticles");
	g_maxDiffuseParticles = inputs->getParInt("Maxdiffuseparticles");
}

void FlexSystem::updateTriangleMesh(const OP_Inputs* inputs)
{
	const OP_SOPInput* sopInput = inputs->getParSOP("Trianglespolysop");

	if (deformingMesh && sopInput && sopInput->getNumPrimitives() > 0 &&
		g_triangleCollisionMesh->GetNumVertices() == sopInput->getNumPoints())
	{

		Point3 minExt(FLT_MAX);
		Point3 maxExt(-FLT_MAX);

		for (int i = 0; i < sopInput->getNumPoints(); i++)
		{
			Position curPos = sopInput->getPointPositions()[i];
			const Point3& a = Point3(curPos.x, curPos.y, curPos.z);

			g_triangleCollisionMesh->m_positions[i] = Vec4(curPos.x, curPos.y, curPos.z, 0.0);

			minExt = Min(a, minExt);
			maxExt = Max(a, maxExt);
		}

		g_triangleCollisionMesh->minExtents = Vector3(minExt);
		g_triangleCollisionMesh->maxExtents = Vector3(maxExt);
		UpdateTriangleMesh(g_triangleCollisionMesh, triangleCollisionMeshId);
	}
}

void FlexSystem::initTriangleMesh(const OP_Inputs* inputs) {
	const OP_SOPInput* sopInput = inputs->getParSOP("Trianglespolysop");

	if (sopInput && (sopInput->getNumPrimitives() > 0))
	{
		if (g_triangleCollisionMesh) 
		{
			delete g_triangleCollisionMesh;
		}

		g_triangleCollisionMesh = new VMesh();
		deformingMesh = inputs->getParInt("Deformingmesh");

		Point3 minExt(FLT_MAX);
		Point3 maxExt(-FLT_MAX);

		for (int i = 0; i < sopInput->getNumPoints(); i++)
		{
			Position curPos = sopInput->getPointPositions()[i];
			const Point3& a = Point3(curPos.x, curPos.y, curPos.z);

			g_triangleCollisionMesh->m_positions.push_back(Vec4(curPos.x, curPos.y, curPos.z, 0.0));

			minExt = Min(a, minExt);
			maxExt = Max(a, maxExt);
		}

		g_triangleCollisionMesh->minExtents = Vector3(minExt);
		g_triangleCollisionMesh->maxExtents = Vector3(maxExt);

		for (int i = 0; i < sopInput->getNumPrimitives(); i++)
		{
			SOP_PrimitiveInfo curPrim = sopInput->getPrimitive(i);
			g_triangleCollisionMesh->m_indices.push_back(curPrim.pointIndices[2]);
			g_triangleCollisionMesh->m_indices.push_back(curPrim.pointIndices[1]);
			g_triangleCollisionMesh->m_indices.push_back(curPrim.pointIndices[0]);
		}

		double meshTrans[3];
		inputs->getParDouble3("Meshtranslation", meshTrans[0], meshTrans[1], meshTrans[2]);

		double meshRot[3];
		inputs->getParDouble3("Meshrotation", meshRot[0], meshRot[1], meshRot[2]);

		curMeshTrans = Vec3((float)meshTrans[0], (float)meshTrans[1], (float)meshTrans[2]);

		Vec3 rot = Vec3((float)meshRot[0], (float)meshRot[1], (float)meshRot[2]);
		Quat qx = QuatFromAxisAngle(Vec3(1, 0, 0), DegToRad(rot.x));
		Quat qy = QuatFromAxisAngle(Vec3(0, 1, 0), DegToRad(rot.y));
		Quat qz = QuatFromAxisAngle(Vec3(0, 0, 1), DegToRad(rot.z));

		curMeshRot = qz * qy * qx;
		previousMeshTrans = curMeshTrans;
		previousMeshRot = curMeshRot;
	}
}

void FlexSystem::initClothMesh(const OP_Inputs* inputs) {
	const OP_SOPInput* sopInput = inputs->getParSOP("Flexcloth0");
	const OP_CHOPInput* instInput = inputs->getParCHOP("Instanceschop");

	if ((sopInput != nullptr) && (sopInput->getNumPrimitives() > 0))
	{
		std::vector<Vec4> m_positions;
		std::vector<int> m_indices;
		const Vector* norms = nullptr;
		const TexCoord* textures = nullptr;
		int32_t numTextures = 0;

		if (clothMesh0 != nullptr) 
		{
			delete clothMesh0;
		}

		if (sopInput->hasNormals()) 
		{
			norms = sopInput->getNormals()->normals;
		}

		if (sopInput->getTextures()->numTextureLayers)
		{
			textures = sopInput->getTextures()->textures;
			numTextures = sopInput->getTextures()->numTextureLayers;
		}

		if ((instInput != nullptr) && (instInput->numChannels == 9))
		{
			float tx, ty, tz;
			float rx, ry, rz;
			float sx, sy, sz;

			for (int i = 0; i < instInput->numSamples; i++)
			{
				tx = instInput->getChannelData(0)[i];
				ty = instInput->getChannelData(1)[i];
				tz = instInput->getChannelData(2)[i];
				rx = instInput->getChannelData(3)[i];
				ry = instInput->getChannelData(4)[i];
				rz = instInput->getChannelData(5)[i];
				sx = instInput->getChannelData(6)[i];
				sy = instInput->getChannelData(7)[i];
				sz = instInput->getChannelData(8)[i];

			}
		}
		else
		{
			// Particle positions
			for (int i = 0; i < sopInput->getNumPoints(); i++)
			{
				Position curPos = sopInput->getPointPositions()[i];

				m_positions.push_back(Vec4(curPos.x, curPos.y, curPos.z, 0.0));

				g_buffers->positions.push_back(Vec4(curPos.x, curPos.y, curPos.z, 1.0f));
				g_buffers->velocities.push_back(Vec3(0.0f, 0.0f, 0.0f));
				g_buffers->phases.push_back(NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter));
				if (norms) g_buffers->normals.push_back(Vec4(norms[i].x, norms[i].y, norms[i].z, 0.0f));
				if (textures) g_buffers->uvs.push_back(Vec3(textures[i].u, textures[i].v, textures[i].w));
			}

			int baseIndex = 0;

			// Triangle indices
			for (int i = 0; i < sopInput->getNumPrimitives(); i++)
			{
				SOP_PrimitiveInfo curPrim = sopInput->getPrimitive(i);
				m_indices.push_back(curPrim.pointIndices[2]);
				m_indices.push_back(curPrim.pointIndices[1]);
				m_indices.push_back(curPrim.pointIndices[0]);
			}

			int triOffset = g_buffers->triangles.size();
			int triCount = m_indices.size() / 3;

			g_buffers->triangles.assign((int*)&m_indices[0], m_indices.size());

			float stretchStiffness = (float)inputs->getParDouble("Strechcloth");
			float bendStiffness = (float)inputs->getParDouble("Bendcloth");

			clothMesh0 = new ClothMesh(&m_positions[0], m_positions.size(), &m_indices[0], m_indices.size(), stretchStiffness, bendStiffness, true);

			const int numSprings = int(clothMesh0->mConstraintCoefficients.size());

			g_buffers->springIndices.resize(numSprings * 2);
			g_buffers->springLengths.resize(numSprings);
			g_buffers->springStiffness.resize(numSprings);

			// Spring constraints
			for (int i = 0; i < numSprings; ++i)
			{
				g_buffers->springIndices[i * 2 + 0] = clothMesh0->mConstraintIndices[i * 2 + 0];
				g_buffers->springIndices[i * 2 + 1] = clothMesh0->mConstraintIndices[i * 2 + 1];
				g_buffers->springLengths[i] = clothMesh0->mConstraintRestLengths[i];
				g_buffers->springStiffness[i] = clothMesh0->mConstraintCoefficients[i];
			}

			float pressure = inputs->getParDouble("Pressure");

			// Inflatable constraints (optional)
			if (pressure > 0.0f)
			{
				g_buffers->inflatableTriOffsets.push_back(triOffset / 3);
				g_buffers->inflatableTriCounts.push_back(triCount);
				g_buffers->inflatablePressures.push_back(pressure);
				g_buffers->inflatableVolumes.push_back(clothMesh0->mRestVolume);
				g_buffers->inflatableCoefficients.push_back(clothMesh0->mConstraintScale);
			}
		}

		
	}
}

void FlexSystem::updatePlanes(const OP_Inputs* inputs)
{
	const OP_CHOPInput* colPlanesInput = inputs->getParCHOP("Colplaneschop");

	if (colPlanesInput)
	{
		if (colPlanesInput->numChannels == 4)
		{
			int nPlanes = colPlanesInput->numSamples;
			g_params.numPlanes = nPlanes;

			for (int i = 0; i < nPlanes; i++)
			{
				(Vec4&)g_params.planes[i] = Vec4(colPlanesInput->getChannelData(0)[i],
					colPlanesInput->getChannelData(1)[i],
					colPlanesInput->getChannelData(2)[i],
					colPlanesInput->getChannelData(3)[i]);
			}
		}
	}
}

void FlexSystem::updateSpheresCols(const OP_Inputs* inputs)
{
	const OP_CHOPInput* spheresInput = inputs->getParCHOP("Colsphereschop");

	if (spheresInput && (spheresInput->numChannels == 4))
	{
		for (int i = 0; i < spheresInput->numSamples; i++)
		{
			Vec3 spherePos = Vec3(spheresInput->getChannelData(0)[i],
				spheresInput->getChannelData(1)[i],
				spheresInput->getChannelData(2)[i]);

			float sphereRadius = spheresInput->getChannelData(3)[i];
			AddSphere(sphereRadius, spherePos, Quat());
		}
	}
}

void FlexSystem::updateBoxesCols(const OP_Inputs* inputs)
{
	const OP_CHOPInput* boxesInput = inputs->getParCHOP("Colboxeschop");

	if (boxesInput && boxesInput->numChannels == 9)
	{
		for (int i = 0; i < boxesInput->numSamples; i++)
		{
			Vec3 boxPos = Vec3(boxesInput->getChannelData(0)[i],
				boxesInput->getChannelData(1)[i],
				boxesInput->getChannelData(2)[i]);

			Vec3 boxSize = Vec3(boxesInput->getChannelData(3)[i],
				boxesInput->getChannelData(4)[i],
				boxesInput->getChannelData(5)[i]);

			Vec3 boxRot = Vec3(boxesInput->getChannelData(6)[i],
				boxesInput->getChannelData(7)[i],
				boxesInput->getChannelData(8)[i]);

			Quat qx = QuatFromAxisAngle(Vec3(1, 0, 0), DegToRad(boxRot.x));
			Quat qy = QuatFromAxisAngle(Vec3(0, 1, 0), DegToRad(boxRot.y));
			Quat qz = QuatFromAxisAngle(Vec3(0, 0, 1), DegToRad(boxRot.z));

			AddBox(boxSize, boxPos, qz * qy * qx);
		}
	}
}

void FlexSystem::updateCloths(const OP_Inputs* inputs)
{
	const OP_SOPInput* sopInput = inputs->getParSOP("Anchorscloth0");

	// update anchor positions
	if (sopInput && (sopInput->getNumPoints() > 0))
	{
		const SOP_CustomAttribData* idx = sopInput->getCustomAttribute("idx");

		for (int i = 0; i < sopInput->getNumPoints(); i++)
		{
			Position curPos = sopInput->getPointPositions()[i];
			g_buffers->positions[(int)idx->floatData[i]] = Vec4(curPos.x, curPos.y, curPos.z, 0.0f);
		}
	}
}

void FlexSystem::AddBox(Vec3 halfEdge, Vec3 center, Quat quat, bool dynamic)
{
	g_buffers->shapePositions.push_back(Vec4(center.x, center.y, center.z, 0.0f));
	g_buffers->shapeRotations.push_back(quat);

	g_buffers->shapePrevPositions.push_back(g_buffers->shapePositions.back());
	g_buffers->shapePrevRotations.push_back(g_buffers->shapeRotations.back());

	NvFlexCollisionGeometry geo;
	geo.box.halfExtents[0] = halfEdge.x;
	geo.box.halfExtents[1] = halfEdge.y;
	geo.box.halfExtents[2] = halfEdge.z;

	g_buffers->shapeGeometry.push_back(geo);
	g_buffers->shapeFlags.push_back(NvFlexMakeShapeFlags(eNvFlexShapeBox, dynamic));
}

void FlexSystem::AddSphere(float radius, Vec3 position, Quat rotation)
{
	NvFlexCollisionGeometry geo;
	geo.sphere.radius = radius;
	g_buffers->shapeGeometry.push_back(geo);

	g_buffers->shapePositions.push_back(Vec4(position, 0.0f));
	g_buffers->shapeRotations.push_back(rotation);

	g_buffers->shapePrevPositions.push_back(g_buffers->shapePositions.back());
	g_buffers->shapePrevRotations.push_back(g_buffers->shapeRotations.back());

	int flags = NvFlexMakeShapeFlags(eNvFlexShapeSphere, false);
	g_buffers->shapeFlags.push_back(flags);
}

NvFlexTriangleMeshId FlexSystem::CreateTriangleMesh(VMesh* m)
{
	if (m == nullptr)
	{
		return 0;
	}

	Vec3 lower, upper;

	lower = m->minExtents;
	upper = m->maxExtents;

	NvFlexVector<Vec4> positions(g_flexLib);
	NvFlexVector<int> indices(g_flexLib);

	positions.assign((Vec4*)&m->m_positions[0], (int)m->m_positions.size());
	indices.assign((int*)&m->m_indices[0], (int)m->m_indices.size());

	positions.unmap();
	indices.unmap();

	NvFlexTriangleMeshId flexMesh = NvFlexCreateTriangleMesh(g_flexLib);
	NvFlexUpdateTriangleMesh(g_flexLib, flexMesh, positions.buffer, indices.buffer, m->GetNumVertices(), m->GetNumFaces(), (float*)&lower, (float*)&upper);

	return flexMesh;
}

void FlexSystem::UpdateTriangleMesh(VMesh* m, NvFlexTriangleMeshId flexMeshId)
{
	if (m == nullptr)
	{
		return;
	}

	Vec3 lower, upper;

	lower = m->minExtents;
	upper = m->maxExtents;

	NvFlexVector<Vec4> positions(g_flexLib);
	NvFlexVector<int> indices(g_flexLib);

	positions.assign((Vec4*)&m->m_positions[0], (int)m->m_positions.size());
	indices.assign((int*)&m->m_indices[0], (int)m->m_indices.size());

	positions.unmap();
	indices.unmap();

	NvFlexUpdateTriangleMesh(g_flexLib, flexMeshId, positions.buffer, indices.buffer, m->GetNumVertices(), m->GetNumFaces(), (float*)&lower, (float*)&upper);
}

void FlexSystem::AddTriangleMesh(NvFlexTriangleMeshId mesh, Vec3 translation, Quat rotation, Vec3 prevTrans, Quat prevRot, Vec3 scale)
{
	NvFlexCollisionGeometry geo;
	geo.triMesh.mesh = mesh;
	geo.triMesh.scale[0] = scale.x;
	geo.triMesh.scale[1] = scale.y;
	geo.triMesh.scale[2] = scale.z;

	g_buffers->shapePositions.push_back(Vec4(translation, 0.0f));
	g_buffers->shapeRotations.push_back(Quat(rotation));
	g_buffers->shapePrevPositions.push_back(Vec4(prevTrans, 0.0f));
	g_buffers->shapePrevRotations.push_back(Quat(prevRot));
	g_buffers->shapeGeometry.push_back((NvFlexCollisionGeometry&)geo);
	g_buffers->shapeFlags.push_back(NvFlexMakeShapeFlags(eNvFlexShapeTriangleMesh, false));
}

void FlexSystem::GetParticleBounds(Vec3& lower, Vec3& upper)
{
	lower = Vec3(FLT_MAX);
	upper = Vec3(-FLT_MAX);

	for (int i=0; i < g_buffers->positions.size(); ++i)
	{
		lower = Min(Vec3(g_buffers->positions[i]), lower);
		upper = Max(Vec3(g_buffers->positions[i]), upper);
	}
}

void FlexSystem::CreateParticleGrid(Vec3 lower, int dimx, int dimy, int dimz, float radius, Vec3 velocity, float invMass, bool rigid, float rigidStiffness, int phase, float jitter=0.005f)
{
	for (int x = 0; x < dimx; ++x) 
	{
		for (int y = 0; y < dimy; ++y) 
		{
			for (int z = 0; z < dimz; ++z)
			{
				Vec3 position = lower + Vec3(float(x), float(y), float(z)) * radius + RandomUnitVector() * jitter;

				g_buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
				g_buffers->velocities.push_back(velocity);
				g_buffers->phases.push_back(phase);
			}
		}
	}
}

void FlexSystem::CreateCenteredParticleGrid(Point3 center, Vec3 rotation, Point3 size, float restDistance, Vec3 velocity, float invMass, bool rigid, int phase, float jitter)
{
	long dx = int(ceilf(size.x / restDistance));
	long dy = int(ceilf(size.y / restDistance));
	long dz = int(ceilf(size.z / restDistance));

	for (int x = 0; x < dx; ++x)
	{
		for (int y = 0; y < dy; ++y) 
		{
			for (int z = 0; z < dz; ++z)
			{
				Point3 position = restDistance * Point3((float)x - 0.5 * ((float)dx - 1.0f), (float)y - 0.5 * ((float)dy - 1.0f), (float)z - 0.5 * ((float)dz - 1.0f)) + RandomUnitVector() * jitter;
				position = TranslationMatrix(center) * TransformMatrix(Rotation(rotation.y, rotation.z, rotation.x), Point3(0.f)) * position;

				if (cursor < (maxParticles - 1))
				{
					g_buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
					g_buffers->velocities.push_back(velocity);
					g_buffers->phases.push_back(phase);
					cursor++;
				}
			}
		}
	}
}

void FlexSystem::CreateSpring(int i, int j, float stiffness, float give = 0.0f)
{
	g_buffers->springIndices.push_back(i);
	g_buffers->springIndices.push_back(j);
	g_buffers->springLengths.push_back((1.0f + give) * Length(Vec3(g_buffers->positions[i]) - Vec3(g_buffers->positions[j])));
	g_buffers->springStiffness.push_back(stiffness);
}

inline int GridIndex(int x, int y, int dx) { return y * dx + x; }

void FlexSystem::CreateSpringGrid(Vec3 lower, int dx, int dy, int dz, float radius, int phase, float stretchStiffness, float bendStiffness, float shearStiffness, Vec3 velocity, float invMass)
{
	int baseIndex = int(g_buffers->positions.size());

	for (int z = 0; z < dz; ++z) 
	{
		for (int y = 0; y < dy; ++y) 
		{
			for (int x = 0; x < dx; ++x)
			{
				Vec3 position = lower + radius * Vec3(float(x), float(z), float(y));

				g_buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
				g_buffers->velocities.push_back(velocity);
				g_buffers->phases.push_back(phase);

				if (x > 0 && y > 0)
				{
					g_buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y - 1, dx));
					g_buffers->triangles.push_back(baseIndex + GridIndex(x, y - 1, dx));
					g_buffers->triangles.push_back(baseIndex + GridIndex(x, y, dx));

					g_buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y - 1, dx));
					g_buffers->triangles.push_back(baseIndex + GridIndex(x, y, dx));
					g_buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y, dx));

					g_buffers->triangleNormals.push_back(Vec3(0.0f, 1.0f, 0.0f));
					g_buffers->triangleNormals.push_back(Vec3(0.0f, 1.0f, 0.0f));
				}
			}
		}
	}

	// horizontal
	for (int y = 0; y < dy; ++y)
	{
		for (int x = 0; x < dx; ++x)
		{
			int index0 = y * dx + x;

			if (x > 0)
			{
				int index1 = y * dx + x - 1;
				CreateSpring(baseIndex + index0, baseIndex + index1, stretchStiffness);
			}
			if (x > 1)
			{
				int index2 = y * dx + x - 2;
				CreateSpring(baseIndex + index0, baseIndex + index2, bendStiffness);
			}

			if (y > 0 && x < dx - 1)
			{
				int indexDiag = (y - 1) * dx + x + 1;
				CreateSpring(baseIndex + index0, baseIndex + indexDiag, shearStiffness);
			}

			if (y > 0 && x > 0)
			{
				int indexDiag = (y - 1) * dx + x - 1;
				CreateSpring(baseIndex + index0, baseIndex + indexDiag, shearStiffness);
			}
		}
	}

	// vertical
	for (int x = 0; x < dx; ++x)
	{
		for (int y = 0; y < dy; ++y)
		{
			int index0 = y * dx + x;

			if (y > 0)
			{
				int index1 = (y - 1) * dx + x;
				CreateSpring(baseIndex + index0, baseIndex + index1, stretchStiffness);
			}

			if (y > 1)
			{
				int index2 = (y - 2) * dx + x;
				CreateSpring(baseIndex + index0, baseIndex + index2, bendStiffness);
			}
		}
	}
}

void FlexSystem::ClearShapes()
{
	g_buffers->shapeGeometry.resize(0);
	g_buffers->shapePositions.resize(0);
	g_buffers->shapeRotations.resize(0);
	g_buffers->shapePrevPositions.resize(0);
	g_buffers->shapePrevRotations.resize(0);
	g_buffers->shapeFlags.resize(0);
}

void FlexSystem::setShapes()
{
	if(g_buffers->shapeFlags.size())
	{
		NvFlexSetShapes(
			g_solver,
			g_buffers->shapeGeometry.buffer,
			g_buffers->shapePositions.buffer,
			g_buffers->shapeRotations.buffer,
			g_buffers->shapePrevPositions.buffer,
			g_buffers->shapePrevRotations.buffer,
			g_buffers->shapeFlags.buffer,
			g_buffers->shapeFlags.size());
	}
}

void FlexSystem::update()
{
	activeParticles = NvFlexGetActiveCount(g_solver);

	g_buffers->UnmapBuffers();

	NvFlexCopyDesc copyDesc;
	copyDesc.dstOffset = 0;
	copyDesc.srcOffset = 0;

	// Active particles
	if (g_buffers->activeIndices.size())
	{
		copyDesc.elementCount = g_buffers->activeIndices.size();
		NvFlexSetActive(g_solver, g_buffers->activeIndices.buffer, &copyDesc);
		NvFlexSetActiveCount(g_solver, g_buffers->activeIndices.size());
	}

	// Particles
	if (g_buffers->positions.size())
	{
		copyDesc.elementCount = g_buffers->positions.size();
		NvFlexSetParticles(g_solver, g_buffers->positions.buffer, &copyDesc);
		NvFlexSetVelocities(g_solver, g_buffers->velocities.buffer, &copyDesc);
		NvFlexSetPhases(g_solver, g_buffers->phases.buffer, &copyDesc);
		NvFlexSetRestParticles(g_solver, g_buffers->restPositions.buffer, &copyDesc);
	}

	// Dynamic triangles
	if (g_buffers->triangles.size())
	{
		NvFlexSetDynamicTriangles(g_solver, g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);
	}

	// Inflatables
	if (g_buffers->inflatableTriOffsets.size())
	{
		NvFlexSetInflatables(g_solver, g_buffers->inflatableTriOffsets.buffer, g_buffers->inflatableTriCounts.buffer, g_buffers->inflatableVolumes.buffer, g_buffers->inflatablePressures.buffer, g_buffers->inflatableCoefficients.buffer, g_buffers->inflatableTriOffsets.size());
	}

	setShapes();
	NvFlexSetParams(g_solver, &g_params);
	NvFlexUpdateSolver(g_solver, g_dt, g_numSubsteps, g_profile);

	if (g_buffers->positions.size())
	{
		copyDesc.elementCount = g_buffers->positions.size();
		NvFlexGetParticles(g_solver, g_buffers->positions.buffer, &copyDesc);
		NvFlexGetParticles(g_solver, g_buffers->positionsGpu.buffer, &copyDesc);
		NvFlexGetVelocities(g_solver, g_buffers->velocities.buffer, &copyDesc);
		NvFlexGetNormals(g_solver, g_buffers->normalsGpu.buffer, NULL);
	}

	if (g_buffers->triangles.size()) 
	{
		NvFlexGetDynamicTriangles(g_solver, g_buffers->trianglesGpu.buffer, g_buffers->triangleNormalsGpu.buffer, g_buffers->triangles.size() / 3);
	}

	activeParticles = NvFlexGetActiveCount(g_solver);

	// Forcefields
	if ((forcefieldRadial != nullptr) && (callback != nullptr) && (nFields > 0))
	{
		NvFlexExtSetForceFields(callback, forcefieldRadial, nFields);
	}
}