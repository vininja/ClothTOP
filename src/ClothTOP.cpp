#include "ClothTOP.h"

#include <assert.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <string.h>
#endif
#include <cstdio>

extern "C"
{
	DLLEXPORT
		void
		FillTOPPluginInfo(TOP_PluginInfo *info)
	{
		// Always set this to CHOPCPlusPlusAPIVersion.
		info->apiVersion = TOPCPlusPlusAPIVersion;

		// Change this to change the executeMode behavior of this plugin.
		info->executeMode = TOP_ExecuteMode::OpenGL_FBO;
		// The opType is the unique name for this CHOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Clothtop");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("Cloth TOP");

		// Will be turned into a 3 letter icon on the nodes
		info->customOPInfo.opIcon->setString("CLT");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("Vinicius Ginja");
		info->customOPInfo.authorEmail->setString("https://github.com/vininja");

		// This TOP works with 0 inputs
		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 0;
	}

	DLLEXPORT
	TOP_CPlusPlusBase*
	CreateTOPInstance(const OP_NodeInfo* info, TOP_Context* context)
	{
		return new ClothTOP(info, context);
	}

	DLLEXPORT
	void
	DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context* context)
	{
		context->beginGLCommands();
		delete (ClothTOP*)instance;
		context->endGLCommands();
	}
};

ClothTOP::ClothTOP(const OP_NodeInfo* info, TOP_Context* context)
	: myNodeInfo(info), myExecuteCount(0), myError(nullptr),
	activeCount(0), cuArray_outColorBuffer1(nullptr), cuArray_outColorBuffer2(nullptr),
	cuGlResource_outColorBuffer1(nullptr), cuGlResource_outColorBuffer2(nullptr),
	timer(0.0f)
{
	myExecuteCount = 0;

	std::cout << "[CLT] INIT" << std::endl;
	FlexSys = &FlexSys->getInstance();
	FlexSys->initSystem();

	maxParticles = 0;
	activeIndicesSize = 0;
	inactiveIndicesSize = 0;
}

ClothTOP::~ClothTOP()
{
	
}

void
ClothTOP::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	ginfo->cookEveryFrameIfAsked = true;
	ginfo->clearBuffers = true;
}

bool
ClothTOP::getOutputFormat(TOP_OutputFormat* format, const OP_Inputs* inputs, void* reserved1)
{
	return false;
}

/////////////////////////////// CUDA FUNCTIONS /////////////////////////////////

extern void launch_PosNormKernel(int outputmode, dim3 grid, dim3 block, int npoints, int ntris, void** mapped_g_buff, cudaArray* output1, cudaArray* output2, int imgw, int imgh);

////////////////////////////////////////////////////////////////////////////////

void ClothTOP::execute(TOP_OutputFormatSpecs* outputFormat,
	const OP_Inputs* inputs,
	TOP_Context* context,
	void* reserved1)
{
	myExecuteCount++;

	int simulate = inputs->getParInt("Simulate");
	int reset = inputs->getParInt("Reset");

	if (reset == 1) 
	{
		std::cout << "[CLT] RESET" << std::endl;

		FlexSys->initScene();
		FlexSys->initClothMesh(inputs);
		FlexSys->initTriangleMesh(inputs);

		if (inputs->getParCHOP("Forceschop") && (FlexSys->forcefieldRadial != nullptr))
		{
			delete FlexSys->forcefieldRadial;
		}

		cudaGraphicsGLRegisterImage(&cuGlResource_outColorBuffer1, outputFormat->colorBufferRB[1], GL_RENDERBUFFER, cudaGraphicsRegisterFlagsSurfaceLoadStore);
		cudaGraphicsGLRegisterImage(&cuGlResource_outColorBuffer2, outputFormat->colorBufferRB[2], GL_RENDERBUFFER, cudaGraphicsRegisterFlagsSurfaceLoadStore);
	} 

	FlexSys->updateParams(inputs);
	
	if (FlexSys->g_solver) 
	{
		FlexSys->g_buffers->MapBuffers();
		FlexSys->getSimTimers();

		cudaGraphicsMapResources(1, &cuGlResource_outColorBuffer1, 0);
		cudaGraphicsMapResources(1, &cuGlResource_outColorBuffer2, 0);
		
		cudaGraphicsSubResourceGetMappedArray(&cuArray_outColorBuffer1, cuGlResource_outColorBuffer1, 0, 0);
		cudaGraphicsSubResourceGetMappedArray(&cuArray_outColorBuffer2, cuGlResource_outColorBuffer2, 0, 0);

		int nThreads = 16;
		int width = outputFormat->width;
		int height = outputFormat->height;

		dim3 block(nThreads, nThreads, 1);
		dim3 grid;

		grid.x = width / nThreads + (!(width % nThreads) ? 0 : 1); 
		grid.y = height / nThreads + (!(height % nThreads) ? 0 : 1);

		void* mapped_g_buff[4] = {	FlexSys->g_buffers->positionsGpu.mappedPtr,
									FlexSys->g_buffers->trianglesGpu.mappedPtr,
									FlexSys->g_buffers->normalsGpu.mappedPtr,
									FlexSys->g_buffers->uvsGpu.mappedPtr };

		launch_PosNormKernel(inputs->getParInt("Outputmode"),grid, block, FlexSys->g_buffers->positionsGpu.size(), FlexSys->g_buffers->trianglesGpu.size() / 3, mapped_g_buff, cuArray_outColorBuffer1, cuArray_outColorBuffer2, outputFormat->width, outputFormat->height);

		FlexSys->updatePlanes(inputs);

		float pressure = (float)inputs->getParDouble("Pressure");

		if (FlexSys->g_buffers->inflatablePressures.mappedPtr != nullptr) 
		{
			for (int i = 0; i < int(FlexSys->g_buffers->inflatablePressures.size()); ++i) 
			{
				FlexSys->g_buffers->inflatablePressures[i] = pressure;
			}
		}

		FlexSys->ClearShapes();

		FlexSys->updateCloths(inputs);
		FlexSys->updateSpheresCols(inputs);
		FlexSys->updateBoxesCols(inputs);

		if (FlexSys->g_triangleCollisionMesh) 
		{
			FlexSys->updateTriangleMesh(inputs);

			FlexSys->previousMeshTrans = FlexSys->curMeshTrans;
			FlexSys->previousMeshRot = FlexSys->curMeshRot; 

			double meshTrans[3];
			inputs->getParDouble3("Meshtranslation", meshTrans[0], meshTrans[1], meshTrans[2]);

			double meshRot[3];
			inputs->getParDouble3("Meshrotation", meshRot[0], meshRot[1], meshRot[2]);

			FlexSys->curMeshTrans = Vec3((float)meshTrans[0], (float)meshTrans[1], (float)meshTrans[2]);
			Vec3 rot = Vec3((float)meshRot[0], (float)meshRot[1], (float)meshRot[2]);

			Quat qx = QuatFromAxisAngle(Vec3(1, 0, 0), DegToRad(rot.x));
			Quat qy = QuatFromAxisAngle(Vec3(0, 1, 0), DegToRad(rot.y));
			Quat qz = QuatFromAxisAngle(Vec3(0, 0, 1), DegToRad(rot.z));

			FlexSys->curMeshRot = qz * qy * qx;
			FlexSys->AddTriangleMesh(FlexSys->triangleCollisionMeshId, FlexSys->curMeshTrans, FlexSys->curMeshRot, FlexSys->previousMeshTrans, FlexSys->previousMeshRot, 1.0f);
		}
	}

	// Radial force field
	if (inputs->getParCHOP("Forceschop"))
	{
		const OP_CHOPInput* radialFieldInput = inputs->getParCHOP("Forceschop");
		if (radialFieldInput->numChannels == 5)
		{
			FlexSys->forcefieldRadial = (NvFlexExtForceField*)malloc(radialFieldInput->numSamples * sizeof(NvFlexExtForceField));

			FlexSys->nFields = radialFieldInput->numSamples;

			for (int i = 0; i < radialFieldInput->numSamples; i++)
			{
				if (FlexSys->forcefieldRadial != nullptr)
				{
					FlexSys->forcefieldRadial[i].mPosition[0] = radialFieldInput->getChannelData(0)[i];
					FlexSys->forcefieldRadial[i].mPosition[1] = radialFieldInput->getChannelData(1)[i];
					FlexSys->forcefieldRadial[i].mPosition[2] = radialFieldInput->getChannelData(2)[i];
					FlexSys->forcefieldRadial[i].mRadius = radialFieldInput->getChannelData(3)[i];
					FlexSys->forcefieldRadial[i].mStrength = radialFieldInput->getChannelData(4)[i];
					FlexSys->forcefieldRadial[i].mMode = eNvFlexExtModeForce;
					FlexSys->forcefieldRadial[i].mLinearFalloff = true;
				}
			}
		}
	}

	if (reset == 1) 
	{
		FlexSys->postInitScene();
		//FlexSys->update();

		FlexSys->g_buffers->uvs.map();
		FlexSys->g_buffers->uvsGpu.map();
		cudaMemcpy(FlexSys->g_buffers->uvsGpu.mappedPtr, FlexSys->g_buffers->uvs.mappedPtr, FlexSys->g_buffers->uvs.count * sizeof(Vec3), cudaMemcpyHostToDevice);

		FlexSys->update();
	}
	else if (FlexSys->g_solver) 
	{
		if (simulate) 
		{
			FlexSys->update();
		}

		activeCount = FlexSys->activeParticles;
		maxParticles = FlexSys->maxParticles;
		activeIndicesSize = FlexSys->g_buffers->activeIndices.size();
		inactiveIndicesSize = (int)FlexSys->g_inactiveIndices.size();
		
		cudaGraphicsUnmapResources(1, &cuGlResource_outColorBuffer1, 0);
		cudaGraphicsUnmapResources(1, &cuGlResource_outColorBuffer2, 0);
	}
}

void ClothTOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		
	}
}

int32_t ClothTOP::getNumInfoCHOPChans(void *reserved1)
{
	return 4;
}

void ClothTOP::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan, void* reserved1)
{
	switch (index) 
	{
		case 0:
			chan->name->setString("executeCount");
			chan->value = (float)myExecuteCount;
			break;
		case 1:
			chan->name->setString("flexTime");
			chan->value = FlexSys->simLatency;
			break;

		case 2:
			chan->name->setString("solveVelocities");
			chan->value = FlexSys->g_timers.solveVelocities;
			break;
		case 3:
			chan->name->setString("maxVel");
			chan->value = FlexSys->g_params.maxSpeed;
			break;
	}
}

bool ClothTOP::getInfoDATSize(OP_InfoDATSize* infoSize, void *reserved1)
{
	infoSize->rows = 7;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void ClothTOP::getInfoDATEntries(int32_t index, int32_t nEntries,
							OP_InfoDATEntries* entries,
							void *reserved1)
{
	
	static char tempBuffer1[4096];
	static char tempBuffer2[4096];

	switch (index) 
	{
		case 0:
			strcpy(tempBuffer1, "maxParticles");
			sprintf(tempBuffer2, "%d", maxParticles);
			break;
		case 1:
			strcpy(tempBuffer1, "activeIndicesSize");
			sprintf(tempBuffer2, "%d", activeIndicesSize);
			break;
		case 2:
			strcpy(tempBuffer1, "inactiveIndicesSize");
			sprintf(tempBuffer2, "%d", inactiveIndicesSize);
			break;
		case 3:
			strcpy(tempBuffer1, "maxVel");
			sprintf(tempBuffer2, "%f", FlexSys->g_params.maxSpeed);
			break;
		case 4:
			strcpy(tempBuffer1, "cursor");
			sprintf(tempBuffer2, "%d", FlexSys->cursor);
			break;
	}

	entries->values[0]->setString(tempBuffer1);
	entries->values[1]->setString(tempBuffer2);
}

void ClothTOP::getErrorString(OP_String* error, void* reserved1)
{
	error->setString(myError);
}

void ClothTOP::setupParamsCustom(OP_ParameterManager* manager) 
{
	// Reset
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		np.defaultValues[0] = 0.0;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Simulate
	{
		OP_NumericParameter	np;

		np.name = "Simulate";
		np.label = "Simulate";
		np.defaultValues[0] = 1.0;

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Output mode
	{
		OP_StringParameter sp;

		sp.name = "Outputmode";
		sp.label = "Output Mode";
		sp.page = "Render";
		sp.defaultValue = "Linearmode";

		const char* names[2] = { "Linearmode", "Uvcoordmode" };
		const char* labels[2] = { "Linear Mode", "UV Mode" };

		OP_ParAppendResult res = manager->appendMenu(sp, 2, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// Radius
	{
		OP_NumericParameter	np;

		np.name = "Radius";
		np.label = "Radius";
		np.defaultValues[0] = 0.05;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Maxspeed
	{
		OP_NumericParameter	np;

		np.name = "Maxspeed";
		np.label = "Max Speed";
		np.defaultValues[0] = 100;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Gravity
	{
		OP_NumericParameter	np;

		np.name = "Gravity";
		np.label = "Gravity";

		np.defaultValues[0] = 0.0;
		np.defaultValues[1] = -3.0;
		np.defaultValues[2] = 0.0;


		OP_ParAppendResult res = manager->appendXYZ(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void ClothTOP::setupParamsSolver(OP_ParameterManager* manager) {
	///////////SOLVER

	// Fps
	{
		OP_NumericParameter	np;

		np.name = "Fps";
		np.label = "FPS";
		np.page = "Solver";
		np.defaultValues[0] = 30;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Numsubsteps
	{
		OP_NumericParameter	np;

		np.name = "Numsubsteps";
		np.label = "Num Substeps";
		np.page = "Solver";
		np.defaultValues[0] = 3;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Numiterations
	{
		OP_NumericParameter	np;

		np.name = "Numiterations";
		np.label = "Num Iterations";
		np.page = "Solver";
		np.defaultValues[0] = 3;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Maxparticles
	{
		OP_NumericParameter	np;

		np.name = "Maxparticles";
		np.label = "Max Number of Particles";
		np.page = "Solver";
		np.defaultValues[0] = 160000;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Maxdiffuseparticles
	{
		OP_NumericParameter	np;

		np.name = "Maxdiffuseparticles";
		np.label = "Max Number of Diffuse Particles";
		np.page = "Solver";
		np.defaultValues[0] = 2000;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Forceschop
	{
		OP_StringParameter sp;

		sp.name = "Forceschop";
		sp.label = "Forcefields CHOP";
		sp.page = "Solver";

		OP_ParAppendResult res = manager->appendCHOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}
}

void ClothTOP::setupParamsParts(OP_ParameterManager* manager) {
	// Damping
	{
		OP_NumericParameter	np;

		np.name = "Damping";
		np.label = "Damping";
		np.defaultValues[0] = 0;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Dynamicfriction
	{
		OP_NumericParameter	np;

		np.name = "Dynamicfriction";
		np.label = "Dynamic Friction";
		np.defaultValues[0] = 0;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Restitution
	{
		OP_NumericParameter	np;

		np.name = "Restitution";
		np.label = "Restitution";
		np.defaultValues[0] = 0.001f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Adhesion
	{
		OP_NumericParameter	np;

		np.name = "Adhesion";
		np.label = "Adhesion";
		np.defaultValues[0] = 0.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Dissipation
	{
		OP_NumericParameter	np;

		np.name = "Dissipation";
		np.label = "Dissipation";
		np.defaultValues[0] = 0.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Cohesion
	{
		OP_NumericParameter	np;

		np.name = "Cohesion";
		np.label = "Cohesion";
		np.defaultValues[0] = 0.1f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Surfacetension
	{
		OP_NumericParameter	np;

		np.name = "Surfacetension";
		np.label = "Surface Tension";
		np.defaultValues[0] = 0.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Viscosity
	{
		OP_NumericParameter	np;

		np.name = "Viscosity";
		np.label = "Viscosity";
		np.defaultValues[0] = 0.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Vorticityconfinement
	{
		OP_NumericParameter	np;

		np.name = "Vorticityconfinement";
		np.label = "Vorticity Confinement";
		np.defaultValues[0] = 80.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Smoothingstrength
	{
		OP_NumericParameter	np;

		np.name = "Smoothingstrength";
		np.label = "Smoothing Strength";
		np.defaultValues[0] = 1.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Anisotropyscale
	{
		OP_NumericParameter	np;

		np.name = "Anisotropyscale";
		np.label = "Anisotropy Scale";
		np.defaultValues[0] = 1.0f;
		np.page = "PartsParams";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void ClothTOP::setupParamsCollisions(OP_ParameterManager* manager) {
	//Colplaneschop
	{
		OP_StringParameter sp;

		sp.name = "Colplaneschop";
		sp.label = "Collision Planes CHOP";
		sp.page = "Collisions";

		OP_ParAppendResult res = manager->appendCHOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//Colspheres
	{
		OP_StringParameter sp;

		sp.name = "Colsphereschop";
		sp.label = "Collision Spheres CHOP";
		sp.page = "Collisions";

		OP_ParAppendResult res = manager->appendCHOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//Colboxes
	{
		OP_StringParameter sp;

		sp.name = "Colboxeschop";
		sp.label = "Collision Boxes CHOP";
		sp.page = "Collisions";

		OP_ParAppendResult res = manager->appendCHOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//Trianglespolysop
	{
		OP_StringParameter	sp;

		sp.name = "Trianglespolysop";
		sp.label = "Triangles Polygons SOP";
		sp.page = "Collisions";

		OP_ParAppendResult res = manager->appendSOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	// Deformingmesh
	{
		OP_NumericParameter	np;

		np.name = "Deformingmesh";
		np.label = "Deforming Mesh";
		np.page = "Collisions";
		np.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Meshtranslation
	{
		OP_NumericParameter	np;

		np.name = "Meshtranslation";
		np.label = "Mesh Translation";
		np.page = "Collisions";
		np.defaultValues[0] = 0;
		np.defaultValues[1] = 0;
		np.defaultValues[2] = 0;

		OP_ParAppendResult res = manager->appendXYZ(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Meshrotation
	{
		OP_NumericParameter	np;

		np.name = "Meshrotation";
		np.label = "Mesh Rotation";
		np.page = "Collisions";
		np.defaultValues[0] = 0;
		np.defaultValues[1] = 0;
		np.defaultValues[2] = 0;

		OP_ParAppendResult res = manager->appendXYZ(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Collisiondistance
	{
		OP_NumericParameter	np;

		np.name = "Collisiondistance";
		np.label = "Collision Distance";
		np.defaultValues[0] = 0.0f; // 0.05f;
		np.page = "Collisions";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Shapecollisionmargin
	{
		OP_NumericParameter	np;

		np.name = "Shapecollisionmargin";
		np.label = "Shape Collision Margin";
		np.defaultValues[0] = 0.0f; //0.00001f
		np.page = "Collisions";

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void ClothTOP::setupParamsCloths(OP_ParameterManager* manager) {
	//// Instance transforms
	//{
	//	OP_StringParameter sp;

	//	sp.name = "Instanceschop";
	//	sp.label = "Instance Transforms CHOP";
	//	sp.page = "Cloths";

	//	OP_ParAppendResult res = manager->appendCHOP(sp);
	//	assert(res == OP_ParAppendResult::Success);
	//}
	//Cloth mesh
	{
		OP_StringParameter sp;

		sp.name = "Flexcloth0";
		sp.label = "Mesh 0";
		sp.page = "Cloths";

		OP_ParAppendResult res = manager->appendSOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//Cloth anchors
	{
		OP_StringParameter sp;

		sp.name = "Anchorscloth0";
		sp.label = "Anchors 0";
		sp.page = "Cloths";

		OP_ParAppendResult res = manager->appendSOP(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//Stretch Stiffness
	{
		OP_NumericParameter	np;

		np.name = "Strechcloth";
		np.label = "Stretch Stiffness";
		np.page = "Cloths";
		np.defaultValues[0] = 1.0f;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Bend Stiffness
	{
		OP_NumericParameter	np;

		np.name = "Bendcloth";
		np.label = "Bend Stiffness";
		np.page = "Cloths";
		np.defaultValues[0] = 1.0f;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Pressure (inflatable)
	{
		OP_NumericParameter	np;

		np.name = "Pressure";
		np.label = "Pressure";
		np.page = "Cloths";
		np.defaultValues[0] = 0.0f;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Wind
	{
		OP_NumericParameter	np;

		np.name = "Wind";
		np.label = "Wind";
		np.page = "Cloths";

		np.defaultValues[0] = 0.0;
		np.defaultValues[1] = 0.0;
		np.defaultValues[2] = 0.0;

		OP_ParAppendResult res = manager->appendXYZ(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//Drag cloth
	{
		OP_NumericParameter	np;

		np.name = "Dragcloth";
		np.label = "Drag";
		np.page = "Cloths";

		np.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Lift
	{
		OP_NumericParameter	np;

		np.name = "Lift";
		np.label = "Lift";
		np.page = "Cloths";

		np.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void ClothTOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
	setupParamsCustom(manager);
	setupParamsSolver(manager);
	setupParamsCloths(manager);
	setupParamsParts(manager);
	setupParamsCollisions(manager);
}