//--------------------------------------------------------------------------------------
// File: MeshLoader10.cpp
//
// Wrapper class for I3DX10Mesh interface. Handles loading mesh data from an .obj file, including resource manadement for material textures.
// - will include additional formats in the future (FBX, Collada)
// 
// This is based on the DirectX SDK MeshFromOBJ sample
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#include "SDKmisc.h"

#include "MeshLoader10.h"
#include <fstream>

using namespace std;

//Define the input layout
// hardcoded offsets? really? shouldn't we use something different? like
const D3D10_INPUT_ELEMENT_DESC layout_MeshLoader10[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA, 0},
};
UINT numElements_layout_MeshLoader10 = sizeof( layout_MeshLoader10 ) / sizeof( layout_MeshLoader10[0] );

//----------------------------------
// Constructor
MeshLoader10::MeshLoader10(void)
{
	m_pd3dDevice = NULL;
	m_pMesh = NULL;

	m_NumAttribTableEntries = 0;
	m_pAttribTable = NULL;

	ZeroMemory( m_strMediaDir, sizeof( m_strMediaDir ) );
}

//----------------------------------
// Destructor EXTERMINATE!
MeshLoader10::~MeshLoader10(void)
{
	Destroy();
}

//----------------------------------
// DESTROY ALL HU-
void MeshLoader10::Destroy()
{
	for ( int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial)
	{
		Material *pMaterial = m_Materials.GetAt( iMaterial );

		if( pMaterial->pTextureRV10 && !IsErrorResource(pMaterial->pTextureRV10))
		{
			ID3D10Resource* pRes = NULL;

			pMaterial->pTextureRV10->GetResource( &pRes );
			SAFE_RELEASE( pRes );
			SAFE_RELEASE( pRes ); // do it twice, GetResource adds another reference

			SAFE_RELEASE( pMaterial->pTextureRV10);
		}

		SAFE_DELETE( pMaterial );
	}

	m_Materials.RemoveAll();
	m_Vertices.RemoveAll();
	m_Indices.RemoveAll();
	m_Atrributes.RemoveAll();

	SAFE_DELETE_ARRAY( m_pAttribTable );
	m_NumAttribTableEntries = 0;

	SAFE_RELEASE( m_pMesh );
	m_pd3dDevice = NULL;
}

//---------------------------------
// Create the Mesh
HRESULT MeshLoader10::Create( ID3D10Device* pd3dDevice, const WCHAR* strFileName)
{
	HRESULT hr;
	WCHAR str[MAX_PATH] = {0};

	// clean up before we create
	Destroy();

	// Store the device pointer
	m_pd3dDevice = pd3dDevice;

	// Load the vertex buffer, index buffer and subset information from a file. 
	// for now we'll use obj, more to come later. (with automatic choosing?)
	V_RETURN( LoadGeometryFromOBJ( strFileName ) );

	//Set the current directory based on where the mesh was found
	WCHAR wstrOldDir[MAX_PATH] = {0};
	GetCurrentDirectory( MAX_PATH, wstrOldDir);
	SetCurrentDirectory( m_strMediaDir);

	// Load material textures
	for (int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial )
	{
		Material *pMaterial = m_Materials.GetAt(iMaterial );
		if( pMaterial->strTexture[0] )
		{
			pMaterial->pTextureRV10 = ( ID3D10ShaderResourceView*)ERROR_RESOURCE_VALUE;

			if( SUCCEEDED(DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strTexture) ) )
			{
				DXUTGetGlobalResourceCache().CreateTextureFromFile( pd3dDevice, str, &pMaterial->pTextureRV10, false);
			}
		}
	}

	// Restore the original current directory
	SetCurrentDirectory( wstrOldDir );

	// Create the encapsulated mesh
	ID3DX10Mesh *pMesh = NULL;

	V_RETURN( D3DX10CreateMesh( pd3dDevice, 
								layout_MeshLoader10, 
								numElements_layout_MeshLoader10, 
								layout_MeshLoader10[0].SemanticName,
								m_Vertices.GetSize(),
								m_Indices.GetSize() / 3,
								D3DX10_MESH_32_BIT,
								&pMesh ) );

	// Set the vertex data
	pMesh->SetVertexData( 0, (void*)m_Vertices.GetData() );
	m_Vertices.RemoveAll();

	//set the index data
	pMesh->SetIndexData( (void*) m_Indices.GetData(), m_Indices.GetSize() );
	m_Indices.RemoveAll();

	//set the attribute data
	pMesh->SetAttributeData( ( UINT*)m_Atrributes.GetData() );
	m_Atrributes.RemoveAll();

	// reorder and optimize for this card
	V( pMesh->GenerateAdjacencyAndPointReps( 1e-6f) ); // TODO: look up this function!
	V( pMesh->Optimize( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, NULL, NULL ) ); // TODO: this one as well

	pMesh->GetAttributeTable(NULL, &m_NumAttribTableEntries);
	m_pAttribTable = new D3DX10_ATTRIBUTE_RANGE[m_NumAttribTableEntries];
	pMesh->GetAttributeTable( m_pAttribTable, &m_NumAttribTableEntries );

	V( pMesh->CommitToDevice() );

	m_pMesh = pMesh;

	return S_OK;
}

HRESULT MeshLoader10::LoadGeometryFromOBJ( const WCHAR* strFileName)
{

	WCHAR strMaterialFileName[MAX_PATH] ={0};
	WCHAR wstr[MAX_PATH];
	char str[MAX_PATH];
	HRESULT hr;

	// Find the file
	V_RETURN( DXUTFindDXSDKMediaFileCch( wstr, MAX_PATH, strFileName));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, MAX_PATH, NULL, NULL);

	// Store the directory where the mesh was found
	wcscpy_s( m_strMediaDir, MAX_PATH -1, wstr);
	WCHAR* pch = wcsrchr(m_strMediaDir, L'\\');
	if(pch)
		*pch = NULL;

	// Create temporary storage for all the input data. Once the data has been loaded
	// into a good format, we can create a D3DXMesh object with it.
	CGrowableArray <D3DXVECTOR3> Positions;
	CGrowableArray <D3DXVECTOR2> TexCoords;
	CGrowableArray <D3DXVECTOR3> Normals;

	// The first subset uses the default material
	Material* pMaterial = new Material();
	if( pMaterial == NULL)
		return E_OUTOFMEMORY;

	InitMaterial(pMaterial);
	wcscpy_s(pMaterial->strName, MAX_PATH-1, L"default");
	m_Materials.Add(pMaterial);

	DWORD dwCurSubset = 0;

	//File input
	WCHAR strCommand[256] = {0};
	wifstream InFile(str);
	if(!InFile)
		return DXTRACE_ERR(L"wifstream::open", E_FAIL);

	#pragma region read all the data
for(;;)
	{
		InFile >> strCommand;
		if (!InFile)
			break;

		if(0 == wcscmp(strCommand, L"#"))
		{

			// just a comment, don't bother parsing.
		}
		else if(0 == wcscmp(strCommand, L"v"))
		{
			// we have a VERTEX POSITION!
			float x, y, z;
			InFile >> x >> y >> z;
			Positions.Add(D3DXVECTOR3(x,y,z));			
		}
		else if(wcscmp(strCommand, L"vt"))
		{
			// Oh my! A texture Coordinate!
			float u, v;
			InFile >> u >> v;
			TexCoords.Add(D3DXVECTOR2(u,v));
		}
		else if(wcscmp(strCommand, L"vn"))
		{

			//AAAAAAaand... a Vertex Normal!
			float x,y,z;
			InFile >> x >> y >> z;
			Normals.Add(D3DXVECTOR3(x,y,z));
		}
		else if (0 == wcscmp(strCommand, L"f"))
		{
			// a Face, this is going to be a biiit more complex
			// we shall match up all the positions, normals and texture coordinates
			// EXTREME GRAPHICS SPEED DATING! MANDATORY!
			UINT iPosition, iTexCoord, iNormal;
			VERTEX vertex;

			for (UINT iFace = 0; iFace < 3; iFace++)
			{
				ZeroMemory(&vertex, sizeof(VERTEX));

				//OBJ format uses 1-based arrays. silly buggers
				InFile >> iPosition;
				vertex.position = Positions[iPosition - 1];
			

			if( '/' == InFile.peek())
			{

				InFile.ignore();

				if ('/' != InFile.peek())
				{
					// optional texture coordinate
					InFile >> iTexCoord;
					vertex.texcoord = TexCoords[iTexCoord - 1];

				}

				if( '/' == InFile.peek())
				{
					//optional vertex normal.
					InFile >> iNormal;
					vertex.normal = Normals[ iNormal - 1];
					
				}
			}

			// check for duplicate vertices. if there is none, add it to the list. 
			// also store index in indices array. 
			// this will someday become a full grown Vertex and Index Buffer. or rather, next frame.
			DWORD index = AddVertex( iPosition, &vertex);
			if (index == (DWORD) - 1 )
				return E_OUTOFMEMORY;

			m_Indices.Add(index);
			}

			m_Atrributes.Add(dwCurSubset);
		}// ENDIF face ("f")
		else if (0 == wcscmp(strCommand, L"mtllib"))
		{
			// oh look, a Material Library
			InFile >> strMaterialFileName;

		}
		else if( 0 == wcscmp(strCommand, L"usemtl"))
		{
			//Material file. mtl, stuff. u know.
			WCHAR strName[MAX_PATH] = {0};
			InFile >> strName;

			bool bFound = false;
			for (int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
			{
				Material* pCurMaterial = m_Materials.GetAt(iMaterial);
				if(0 ==wcscmp(pCurMaterial->strName, strName))
				{
					bFound = true;
					dwCurSubset = iMaterial;
					break;
				}
			}

			if(!bFound)
			{
				pMaterial = new Material();
				if(pMaterial == NULL)
					return E_OUTOFMEMORY;

				dwCurSubset = m_Materials.GetSize();

				InitMaterial( pMaterial );
				wcscpy_s(pMaterial->strName, MAX_PATH - 1, strName);

				m_Materials.Add(pMaterial);
			}
		}
		else
		{
			// huh? what? 
			// unrecognized command
			// probably a curve-based model.. :)
			// TODO: add a message box or some notification?
		}

		InFile.ignore( 1000, '\n');
	}
#pragma endregion read all the data

	// Cleanup
	InFile.close();
	DeleteCache();

	//if an associated material file was found, read that as well. 
	// not like we wanted to get anything done today!
	if( strMaterialFileName[0])
	{
		V_RETURN( LoadMaterialsFromMTL(strMaterialFileName));
	}

	return S_OK;
}

DWORD MeshLoader10::AddVertex(UINT hash, VERTEX* pVertex )
{
	// if this vertex doesnÄt already exist in the Vertices list, create a new entry.
	// Add the index of the vertex to the Indices list.
	bool bFoundInList = false;
	DWORD index = 0;

	// we'll use a hashtable to speed up the process a bit.
	if ((UINT)m_VertexCache.GetSize() > hash)
	{
		CacheEntry* pEntry = m_VertexCache.GetAt(hash);
		while(pEntry != NULL)
		{
			VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

			// identical Vertex already exists? just point the index buffer in the right direction
			if(0 == memcmp(pVertex, pCacheVertex, sizeof(VERTEX)))
			{
				bFoundInList = true;
				index = pEntry->index;
				break;
			}

			pEntry = pEntry->pNext;
		}
	}

	// Whoo we have a new Vertex! Create a new cozy entry for it. 
	// within the Vertices List and the hashtable (yes, both!)
	if ( !bFoundInList)
	{
		//Add to Vertices LIst
		index = m_Vertices.GetSize();
		m_Vertices.Add(*pVertex);

		//And to the hashtable as well
		CacheEntry* pNewEntry = new CacheEntry;
		if( pNewEntry == NULL )
			return (DWORD) -1;

		pNewEntry->index = index;
		pNewEntry->pNext = NULL;

		// Grow the cache if needed
		while((UINT)m_VertexCache.GetSize() <= hash)
		{
			m_VertexCache.Add(NULL);
		}

		// Add it to the end of the linked list
		CacheEntry* pCurEntry = m_VertexCache.GetAt(hash);
		if(pCurEntry == NULL)
		{
			// head element!
			m_VertexCache.SetAt(hash, pNewEntry);
		}
		else
		{
			//find the tail
			while( pCurEntry->pNext != NULL)
			{
				pCurEntry = pCurEntry->pNext;
			}

			pCurEntry->pNext = pNewEntry;
		}
	}

	return index;
}

//---------------------------------
void MeshLoader10::DeleteCache()
{
	// iterate through all elements in cache and subsequent lists
	for( int i=0; i < m_VertexCache.GetSize(); i++)
	{
		CacheEntry* pEntry = m_VertexCache.GetAt(i);
		while(pEntry != NULL)
		{
			CacheEntry* pNext = pEntry->pNext;
			SAFE_DELETE(pEntry);
			pEntry = pNext;
		}
	}

	m_VertexCache.RemoveAll();
}

HRESULT MeshLoader10::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
	HRESULT hr;

	// Set the current directory based on where the mesh was found
	WCHAR wstrOldDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, wstrOldDir);
	SetCurrentDirectory(m_strMediaDir);

	//Find the File
	WCHAR strPath[MAX_PATH];
	char cstrPath[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(strPath, MAX_PATH, strFileName));
	WideCharToMultiByte(CP_ACP, 0, strPath, -1, cstrPath, MAX_PATH, NULL, NULL);

	//File Input
	WCHAR strCommand[256] = {0};
	wifstream InFile(cstrPath);
	if (!InFile)
		return DXTRACE_ERR(L"wifstream::open", E_FAIL);

	// Restore the original directory
	SetCurrentDirectory( wstrOldDir);

	Material* pMaterial = NULL;

	for (;;)
	{
		InFile >> strCommand;
		if(!InFile)
			break;

		if(0 == wcscmp(strCommand, L"newmtl"))
		{
			// swith the active materials (since we have a new one)
			WCHAR strName[MAX_PATH] = {0};
			InFile >> strName;

			pMaterial = NULL;
			for(int i = 0; i < m_Materials.GetSize(); i++)
			{
				Material* pCurMaterial = m_Materials.GetAt(i);
				if(0 == wcscmp(pCurMaterial->strName, strName))
				{
					pMaterial = pCurMaterial;
					break;
				}
			}
		}

		// the rest of the commands rely on an active material, so check if we have one
		if(pMaterial == NULL)
			continue;

		if( 0 == wcscmp(strCommand, L"#"))
		{
			// don't people ever get bored of commenting? (I don't)
		}
		else if(0 == wcscmp(strCommand, L"Ka"))
		{
			// Ambient color
			float r, g, b;
			InFile >> r >> g >> b;
			pMaterial->vAmbient = D3DXVECTOR3(r, g, b);
		}
		else if(0 == wcscmp(strCommand, L"Kd"))
		{
			// Diffuse color
			float r, g, b;
			InFile >> r >> g >> b;
			pMaterial->vDiffuse = D3DXVECTOR3(r, g, b);
		}
		else if(0 == wcscmp(strCommand, L"Ks"))
		{
			// Specular color
			float r, g, b;
			InFile >> r >> g >>b;
			pMaterial->vSpecular = D3DXVECTOR3(r, g, b);
		}
		else if(0 == wcscmp(strCommand, L"d") || 0 == wcscmp(strCommand, L"Tr"))
		{
			// alpha (transparency
			InFile >> pMaterial->fAlpha;
		}
		else if (0 == wcscmp(strCommand, L"Ns"))
		{
			// Shininess
			int nShininess;
			InFile >> nShininess;
			pMaterial->nShininess;
		}
		else if(0 == wcscmp(strCommand, L"illum"))
		{
			// Specular on /off
			int illumination;
			InFile >> illumination;
			pMaterial->bSpecular = (illumination == 2);
		}
		else if( 0 == wcscmp(strCommand, L"map_Kd"))
		{
			// Texture
			InFile >> pMaterial->strTexture;
		}
		else
		{
			// Huh? not a command I know, or implemented yet
		}

		InFile.ignore(1000, '\n');
	}

	InFile.close();

	return S_OK;
}

void MeshLoader10::InitMaterial( Material* pMaterial )
{
	ZeroMemory( pMaterial, sizeof( Material));

	pMaterial->vAmbient = D3DXVECTOR3(0.2f, 0.2f, 0.2f);
	pMaterial->vDiffuse = D3DXVECTOR3(0.8f, 0.8f, 0.8f);
	pMaterial->vSpecular = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	pMaterial->nShininess = 0;
	pMaterial->fAlpha = 1.0f;
	pMaterial->bSpecular = false;
	pMaterial->pTextureRV10 = NULL;
}