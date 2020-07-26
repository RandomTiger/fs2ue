 
#if defined(FS2_UE)
FMatrix gTempMat;
FVector gTempVec;
#endif

struct Submodel
{
	static const int kNoIndexSet = -1;

	Submodel(const int lTexture, const int lIndexStart, const int lSubIndex)
		: mTexture(lTexture)
		, mIndexCount(0)
		, mIndexStart(0)
		, mSubIndex(lSubIndex)
	{}

	int mTexture;
	int mIndexCount;
	int mIndexStart;

	int mSubIndex;

	std::vector<int> mIndexList;
#if defined(FS2_UE)
	FMatrix mMatrix;
	FVector mPos;
#endif
};

std::vector<Submodel> g_SubModels;

vector gMins;
vector gMaxs;

const int stride = 8;
const int iMaxVertexCount = 6000;
const int iMaxIndexCount = iMaxVertexCount;
const int iFloatCount = iMaxVertexCount * stride;
int iCurrentVertex = 0;

float VertexArray[iFloatCount];

bool g_modelCacheInProgress = false;

int gCurrentSubmodel = -1;
int gSubmodelIndex = Submodel::kNoIndexSet;

void AddVertex(VertexNorm *vert)
{
	assert((iCurrentVertex * stride + 8) < iFloatCount);

	VertexArray[(iCurrentVertex * stride) + 0] = vert->sx;
	VertexArray[(iCurrentVertex * stride) + 1] = vert->sy;
	VertexArray[(iCurrentVertex * stride) + 2] = vert->sz;

	VertexArray[(iCurrentVertex * stride) + 3] = vert->nx;
	VertexArray[(iCurrentVertex * stride) + 4] = vert->ny;
	VertexArray[(iCurrentVertex * stride) + 5] = vert->nz;

	VertexArray[(iCurrentVertex * stride) + 6] = vert->tu;
	VertexArray[(iCurrentVertex * stride) + 7] = vert->tv;

	const int lAvaliableSubmodels = (int)g_SubModels.size();
	assert(gCurrentSubmodel >= 0);
	assert(gCurrentSubmodel < lAvaliableSubmodels);
	g_SubModels[gCurrentSubmodel].mIndexList.push_back(iCurrentVertex);
	iCurrentVertex++;

}

void SetModelCacheInProgress(const bool state) { g_modelCacheInProgress = state; }

bool isModelCacheInProgress()
{
	return g_modelCacheInProgress;
}

void model_cache_set_matrix(vector *pos,matrix *orient)
{
#if defined(FS2_UE)
	gTempMat = orient->Get();
	gTempVec = pos->Get();
#endif
}

void model_cache_set_texture(int texture)
{
	if(isModelCacheInProgress())
	{
		const int lSize = g_SubModels.size();
		for(int i = 0; i < lSize; i++)
		{
			if(texture == g_SubModels[i].mTexture)
			{
				if(gSubmodelIndex == Submodel::kNoIndexSet || gSubmodelIndex == g_SubModels[i].mSubIndex)
				{
					gCurrentSubmodel = i;
					return;
				}
			}
		}

		gCurrentSubmodel = lSize;
		g_SubModels.push_back(Submodel(texture,iCurrentVertex, gSubmodelIndex));

#if defined(FS2_UE)
		g_SubModels[gCurrentSubmodel].mMatrix = gTempMat;
		g_SubModels[gCurrentSubmodel].mPos = gTempVec;
#endif

	}
}

void model_cache_force_subset(const int lSubmodelIndex)
{
	gSubmodelIndex = lSubmodelIndex;
}
	
void model_cache_add_vertex(VertexNorm *src_v)
{
	if(isModelCacheInProgress())
	{
		const int submodel = g_SubModels.size() - 1;
		assert(submodel != -1);

		AddVertex(src_v);

		if(src_v->sx < gMins.x)
		{
			gMins.x = src_v->sx;
		}
		if(src_v->sy < gMins.y)
		{
			gMins.y = src_v->sy;
		}
		if(src_v->sz < gMins.z)
		{
			gMins.z = src_v->sz;
		}
		if(src_v->sx > gMaxs.x)
		{
			gMaxs.x = src_v->sx;
		}
		if(src_v->sy > gMaxs.y)
		{
			gMaxs.y = src_v->sy;
		}
		if(src_v->sz > gMaxs.z)
		{
			gMaxs.z = src_v->sz;
		}
	}
}

void model_start_cache()
{
	gMins = vector(0,0,0);
	gMaxs = vector(0,0,0);

	g_SubModels.clear();
	SetModelCacheInProgress(true);
	iCurrentVertex = 0;
	gCurrentSubmodel = 0;
	gSubmodelIndex = Submodel::kNoIndexSet;
}
		
void model_stop_cache()
{
	SetModelCacheInProgress(false);
	gCurrentSubmodel = -1;
}

float GetBoundSphere(const vector &min, const vector &max)
{
	// Reevaluate this
	return sqrt(pow(max.x-min.x, 2.0f) + pow(max.y-min.y, 2.0f) + pow(max.z-min.z, 2.0f));
}

void createOgreMesh(const char * const lName, polymodel * pm, const int lSubmodelIndex, const bool lSubObj)
{
#if defined(FS2_UE)
	// The rendering code that we hijack needs this setup
	init_free_points();

	model_start_cache();

	Interp_flags = MR_NO_LIGHTING | MR_NO_CULL;
	Interp_tmap_flags = TMAP_FLAG_TEXTURED;

	// Main Hull
	bsp_info *lSubmodel = &pm->submodel[lSubmodelIndex];
	model_interp_sub( (ubyte *) lSubmodel->bsp_data, pm, lSubmodel, 0 );
	model_stop_cache();
		
	if(iCurrentVertex == 0)
	{
		return;
	}

	/// Create the mesh via the MeshManager
	extern const std::string lGroupName;
	std::string lMeshName = lName;
	if(lSubObj)
	{
		const int lBufferSize = 8;
		char lBuffer[lBufferSize];
		sprintf_s(lBuffer, lBufferSize, "_%d", lSubmodelIndex);

		lMeshName.append(lBuffer);
	}

	for (int i = 0; i < iCurrentVertex; i++)
	{
		FVector pos(VertexArray[(i * stride) + 2], VertexArray[(i * stride) + 0], VertexArray[(i * stride) + 1]);
		FVector norm(VertexArray[(i * stride) + 5], VertexArray[(i * stride) + 3], VertexArray[(i * stride) + 4]);
		FVector2D tcoord(VertexArray[(i * stride) + 6], VertexArray[(i * stride) + 7]);

		lSubmodel->ueVertices.Add(pos);
		lSubmodel->ueNormals.Add(norm);
		lSubmodel->ueColor.Add(FColor::White);
		lSubmodel->ueTexCoords.Add(tcoord);
	}

	int lIndexCount = 0;
	const unsigned int lModelSize = g_SubModels.size();
	for (unsigned int m = 0; m < lModelSize; m++)
	{
		g_SubModels[m].mIndexStart = lIndexCount;

		const unsigned int lIndexSize = g_SubModels[m].mIndexList.size();

		for (int i = lIndexSize - 1; i >= 0; i--)
		{
			assert(lIndexCount < iCurrentVertex);

			lSubmodel->ueTriangles.Add(g_SubModels[m].mIndexList[i]);
			lIndexCount++;
		}

		g_SubModels[m].mIndexCount = lIndexCount - g_SubModels[m].mIndexStart;
	}

#endif
}
