//----------------------------
// File: MeshLoader10.h
//
// Wrapper class for I3DX10Mesh interface. Handles loading mesh data from an .obj file, including resource manadement for material textures.
// - will include additional formats in the future (FBX, Collada)
// 
// This is based on the DirectX SDK MeshFromOBJ sample
//--------------------------------


#pragma once

#define ERROR_RESOURCE_VALUE 1

// TODO: find out what this is for.
// I don't really get what this error_resource template is for?
// do we check if the mesh is an error_resource?
template<typename TYPE> BOOL IsErrorResource( TYPE data )
{
	if( ( TYPE )ERROR_RESOURCE_VALUE == data )
		return TRUE;
	return FALSE;
}

//Vertex Format
struct VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
};

// hashtable entry, used for the vertex cache when creating mesh from obj file
struct CacheEntry
{
	UINT index;
	CacheEntry* pNext;
};

//Material properties per mesh subset
struct Material
{
	WCHAR strName[MAX_PATH];

	D3DXVECTOR3 vAmbient;
	D3DXVECTOR3 vDiffuse;
	D3DXVECTOR3 vSpecular;

	int nShininess;
	float fAlpha;

	bool bSpecular;

	WCHAR strTexture[MAX_PATH];
	ID3D10ShaderResourceView* pTextureRV10;
	ID3D10EffectTechnique* pTechnique;
};

class MeshLoader10
{
public:
	MeshLoader10(void);
	~MeshLoader10(void);

	HRESULT Create( ID3D10Device* pd3dDevice, const WCHAR* strFilename );
	void Destroy();

	UINT GetNumMaterials() const
	{
		return m_Materials.GetSize();
	}
	Material* GetMaterial( UINT iMaterial )
	{
		return m_Materials.GetAt( iMaterial );
	}

	UINT GetNumSubsets()
	{
		return m_NumAttribTableEntries;
	}
	Material* GetSubsetMaterial( UINT iSubset )
	{
		return m_Materials.GetAt( m_pAttribTable[iSubset].AttribId );
	}

	ID3DX10Mesh* GetMesh()
	{
		return m_pMesh;
	}
	WCHAR* GetMediaDirectory()
	{
		return m_strMediaDir;
	}

private:
	HRESULT LoadGeometryFromOBJ( const WCHAR* strFileName );
	HRESULT LoadMaterialsFromMTL( const WCHAR* strFileName );
	void InitMaterial( Material* pMaterial );

	DWORD AddVertex(UINT hash, VERTEX* pVertex );
	void DeleteCache();

	ID3D10Device* m_pd3dDevice; //Direct3D Device object associated with this mesh
	ID3DX10Mesh* m_pMesh; //Encapsulated D3DX Mesh

	CGrowableArray <CacheEntry*>	m_VertexCache;	// Hashtable cache for locating duplicate vertices
	CGrowableArray <VERTEX>			m_Vertices;		// Filled and copied to the Vertex Buffer
	CGrowableArray <DWORD>			m_Indices;		//				- " -       Index Buffer
	CGrowableArray <DWORD>			m_Atrributes;	//				- " -		attribute buffer
	CGrowableArray <Material*>		m_Materials;	// Holds the material properties per subset

	UINT m_NumAttribTableEntries;
	D3DX10_ATTRIBUTE_RANGE *m_pAttribTable;

	WCHAR m_strMediaDir[MAX_PATH]; // Directory where the mesh was found
 };

