 
#if 1
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

#if defined(FS2_UE)

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
#if 0
	lSubmodel->mOgreInfo = new bsp_info::cOgreInfo();
	Ogre::MeshPtr &msh = lSubmodel->mOgreInfo->mMesh = Ogre::MeshManager::getSingleton().createManual(lMeshName, lGroupName);

	msh->sharedVertexData = new Ogre::VertexData();
	msh->sharedVertexData->vertexCount = iCurrentVertex;
	msh->sharedVertexData->vertexStart = 0;

	/// Create declaration (memory format) of vertex data
	Ogre::VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
	size_t offset = 0;
	// 1st buffer
	decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

	/// Allocate vertex buffer of the requested number of vertices (vertexCount) 
	/// and bytes per vertex (offset)
	Ogre::HardwareVertexBufferSharedPtr vbuf = 
		Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, msh->sharedVertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	/// Upload the vertex data to the card

	vbuf->writeData(0, vbuf->getSizeInBytes(), VertexArray, true);

	/// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
	Ogre::VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding; 
	bind->setBinding(0, vbuf);

	/// Upload the index data to the card
	Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, iCurrentVertex, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	int lIndexCount = 0;
	unsigned short *data = (unsigned short *) ibuf->lock(0, ibuf->getSizeInBytes(), Ogre::HardwareBuffer::HBL_DISCARD);
	const unsigned int lModelSize = g_SubModels.size();
	for(unsigned int m = 0; m < lModelSize; m++)
	{
		g_SubModels[m].mIndexStart = lIndexCount;

		const unsigned int lIndexSize = g_SubModels[m].mIndexList.size();
		for(unsigned int i = 0; i < lIndexSize; i++)
		{
			assert(lIndexCount < iCurrentVertex);
			data[lIndexCount] = g_SubModels[m].mIndexList[i];
			lIndexCount++;
		}

		g_SubModels[m].mIndexCount = lIndexCount - g_SubModels[m].mIndexStart;
	}
	ibuf->unlock();


	for(unsigned int m = 0; m < g_SubModels.size(); m++)
	{
		Ogre::SubMesh* sub = msh->createSubMesh();

		// Set parameters of the submesh
		sub->useSharedVertices = true;
		sub->vertexData = 0;

		sub->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		sub->indexData->indexBuffer = ibuf;
		sub->indexData->indexCount = g_SubModels[m].mIndexCount;
		sub->indexData->indexStart = g_SubModels[m].mIndexStart;
		sub->indexData->optimiseVertexCacheTriList();

		extern Texture * GetTexture(int bitmap);
		Texture *t = GetTexture(g_SubModels[m].mTexture);

		Ogre::String lTextureName = t->mOgreTexture->getName();
		mprintf(( "Loading texture: %s", lTextureName.c_str()));

		const int lMaterialNameLen = 48;
		char lMaterialName[lMaterialNameLen];
		sprintf_s(lMaterialName,lMaterialNameLen,"Mat_%d_%s", m, lName);
		Ogre::MaterialPtr lMaterial = Ogre::MaterialManager::getSingleton().create( lMaterialName, lGroupName );

		Ogre::Pass *pass = lMaterial->getTechnique( 0 )->getPass( 0 );
		pass->setLightingEnabled( true );
		pass->setDepthCheckEnabled( true );
		pass->setDepthWriteEnabled( true );
		pass->setCullingMode(Ogre::CULL_ANTICLOCKWISE);
		//pass->setPolygonMode(Ogre::PM_WIREFRAME);

		Ogre::TextureUnitState *tex = pass->createTextureUnitState( "Custom", 0 );

		tex->setTextureName( lTextureName );
		sub->setMaterialName(lMaterialName);

		if(Cmdline_savemesh)
		{
			std::string lExportName = "Meshes\\";
			lExportName += lMaterialName;

			Ogre::MaterialSerializer lMaterialSave;
			lMaterialSave.exportMaterial(lMaterial, lExportName);
		}

		//g_SubModels[gSubmodelIndex].mMatrix = orient->Get();
		//g_SubModels[gSubmodelIndex].mPos = pos->Get();
	}
	

	Ogre::AxisAlignedBox lFullObjectBoundingBox(pm->mins.Get(), pm->maxs.Get());
	msh->_setBounds(lFullObjectBoundingBox);

	/// Set bounding information (for culling)
//	msh->_setBounds(Ogre::AxisAlignedBox(gMins.x,gMins.y,gMins.z,gMaxs.x,gMaxs.y,gMaxs.z));
	msh->_setBoundingSphereRadius(GetBoundSphere(gMins,gMaxs));
	// why is it calling createvertexposekeyframe ??

	/// Notify -Mesh object that it has been loaded
	msh->load();
#endif
#endif
}
#endif