#include "stdafx.h"

#include "Terrain.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"

struct TerrainVertex
{
	Vector3 pos;
	Vector3 normal;
	float shadowingValue;
};

Terrain::Terrain(unsigned int Resolution, float TextureIteration, float SnowHeight, float Interval, const Vector3& LightDirection) :
	mResolution(Resolution),

	mTextureIterationFactor(TextureIteration / mResolution),
	mSnowHeight(SnowHeight),
	mSnowTexture(0),
	mGrassTexture(0),
	mRockTexture(0),
	mSandTexture(0),

	mSnowBump(0),
	mGrassBump(0),
	mRockBump(0),
	mSandBump(0)
{
	// --------------------------------
	Log::get() << "Creating Terrain heights...\n";
	// Height generation - modified Diamond Square (we call it "Island Diamond Square")
    vertices.reset(new TerrainVertex[mResolution*mResolution]);

	for(int z = 0; z < mResolution; z++)
	{      
        for(int x = 0; x < mResolution; x++)
		{
			float xPos = static_cast<float>(x);
			float zPos = static_cast<float>(z);
			if(x == 0)
				xPos -= 2000.0f;
			else if(x == mResolution-1)
				xPos += 2000.0f;
			if(z == 0)
				zPos -= 2000.0f;
			else if(z == mResolution-1)
				zPos += 2000.0f;
			SetPosition(x,z, Vector3(xPos, 0.0f, zPos));
		}
	}

	Generate(0, 0, mResolution-1, mResolution-1, Interval);

	// Precompute Normals - border normals AND border's border normals are up!
	Log::get() << "Computing Terrain normals & per vertex shadowing...\n";
	for(int z = 0; z < mResolution; z++)
	{   
        for(int x = 0; x < mResolution; x++)
		{
			unsigned int Index = GetArrayIndex(x,z);
			vertices[Index].shadowingValue = 1.0f;	// assume no shadow

			if(x<2 || x>(mResolution-3) || z<2 || z>(mResolution-3))
			{
				vertices[Index].normal = Vector3(0,1,0);
			}
			else
			{
				// old approximation
				vertices[Index].normal = Vector3(GetHeight(x - 1, z) - GetHeight(x + 1, z), 2, GetHeight(x, z - 1) - GetHeight(x, z + 1));

				// sum of the normals of the nearest planes
					// gather directions
				/*float MidHeight = GetHeight(x,z);
				Vector3 Left	(1,  GetHeight(x+1,z) - MidHeight, 0);
				Vector3 Right	(-1, GetHeight(x-1,z) - MidHeight, 0);
				Vector3 Up		(0,  GetHeight(x,z-1) - MidHeight, 1);
				Vector3 Down	(0,  GetHeight(x,z+1) - MidHeight, -1);
					// 4 normals
				vertices[Index].normal += cross(Up, Left).normalize();
				vertices[Index].normal += cross(Left, Down).normalize();
				vertices[Index].normal += cross(Down, Right).normalize();
				vertices[Index].normal += cross(Right, Up).normalize();
				vertices[Index].normal *= 0.25;*/

				vertices[Index].normal.normalize();

				// compute per Vertex shadow Approximation
				Vector3 RayPos(static_cast<float>(x), GetHeightInterp(static_cast<float>(x), static_cast<float>(z)) + 0.025f, static_cast<float>(z));
				do
				{
					RayPos += LightDirection;
					if(GetHeightInterp(RayPos.x, RayPos.z) > RayPos.y) // GetHeightInterp too slow?
					{
						vertices[Index].shadowingValue = 0.0f;
						break;
					}
				} while(!IsBorder((int)RayPos.x, (int)RayPos.z));
			}	
		}
	}

	// Smoothing shadow weighted 5x5 filter
	Log::get() << "Smoothing per vertex shadowing...\n";
	for(int z = 2; z < mResolution-2; z++)
	{   
        for(int x = 2; x < mResolution-2; x++)
		{
			unsigned int Index = GetArrayIndex(x,z);

			for(int SampleX=-2; SampleX<=2; ++SampleX)
			{
				for(int SampleZ=-2; SampleZ<=2; ++SampleZ)
				{
					float Weight = static_cast<float>(abs(SampleX + SampleZ));
					if(Weight != 0)
						vertices[Index].shadowingValue += vertices[GetArrayIndex(SampleX+x, SampleZ+z)].shadowingValue / Weight;
				}
			}

			// Weighting (1/x)
			// 4 3 2 3 4
			// 3 2 1 2 3
			// 2 1 1 1 2
			// 3 2 1 2 3
			// 4 3 2 3 4
			// sum is: about 12.6666666

			vertices[Index].shadowingValue /= 12.66666667f;
		}
	}

	// buffer
	// vbo
	glGenBuffers(1, &terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainVertex) * mResolution*mResolution, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError("terrainVBO");

	// ibo
	unsigned int numIndices = (mResolution - 1) * (mResolution - 1) * 6;
	std::unique_ptr<unsigned int[]> indices(new unsigned int[numIndices]);
	unsigned int* index = indices.get();
	for(int y = 0; y < mResolution - 1; y++)
	{
		for(int x = 0; x < mResolution - 1; x++)
		{
			*index = GetArrayIndex(x, y);			++index;
			*index = GetArrayIndex(x + 1, y);		++index;
			*index = GetArrayIndex(x, y + 1);		++index;

			*index = GetArrayIndex(x, y + 1);		++index;
			*index = GetArrayIndex(x + 1, y);		++index;
			*index = GetArrayIndex(x + 1, y + 1);	++index;
		}
	}

	glGenBuffers(1, &terrainIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices.get(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	checkGLError("terrainIBO");

	// generate vao for cloud particles (2 for ping/pong)
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(sizeof(Vector3)));
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), BUFFER_OFFSET(sizeof(Vector3)*2));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	checkGLError("terrainVAO");



	Log::get() << "Loading Terrain shader & Textures ...\n";
	// --------------------------------
	// Load textures
	mSnowTexture = loadTextureWithMipMaps("texture/snow.bmp");
	mRockTexture = loadTextureWithMipMaps("texture/rock.bmp");
	mGrassTexture = loadTextureWithMipMaps("texture/grass.bmp");
	mSandTexture = loadTextureWithMipMaps("texture/sand.jpg");
	mSnowBump = loadTextureWithMipMaps("texture/snowbump.bmp");
	mRockBump = loadTextureWithMipMaps("texture/rockbump.bmp");
	mGrassBump = loadTextureWithMipMaps("texture/grassbump.bmp");
	mSandBump = loadTextureWithMipMaps("texture/sandbump.jpg");
	
	// load shader
	shader.reset(new ShaderObject("Shader\\terrain.vert", "Shader\\terrain.frag"));
	GLuint blockIndex = glGetUniformBlockIndex(shader->getProgram(), "View"); 
	glUniformBlockBinding(shader->getProgram(), blockIndex, 1);	// View binding=1

	Log::get() << "Terrain is ready!\n";
}

Terrain::~Terrain()
{
	glDeleteTextures(1, &mGrassTexture);
	glDeleteTextures(1, &mRockTexture);
	glDeleteTextures(1, &mSnowTexture);
	glDeleteTextures(1, &mSandTexture);

	glDeleteTextures(1, &mGrassBump);
	glDeleteTextures(1, &mRockBump);
	glDeleteTextures(1, &mSnowBump);
	glDeleteTextures(1, &mSandBump);

	glDeleteBuffers(1, &terrainVBO);
	glDeleteBuffers(1, &terrainIBO);
	glDeleteBuffers(1, &terrainVAO);
}


void Terrain::Draw(float WaterHeight, const Vector3& LightDirection)
{
	// set our vertex/fragment programobject
	shader->useProgram();

	// set textures
	glEnable(GL_TEXTURE_2D);
		// 0 grass
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGrassTexture);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "GrassTexture"), 0);
		// 1 rock
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mRockTexture);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "RockTexture"), 1);
		// 2 snow
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mSnowTexture);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "SnowTexture"), 2);
		// 3 Sand
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mSandTexture);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "SandTexture"), 3);

	// set Bumpmaps

		// 0 grass
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mGrassBump);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "GrassBump"), 4);
		// 1 rock
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mRockBump);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "RockBump"), 5);
		// 2 snow
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, mSnowBump);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "SnowBump"), 6);
		// 3 Sand
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, mSandBump);
	glUniform1i(glGetUniformLocation(shader->getProgram(), "SandBump"), 7);
	
	// set other program constants
	glUniform1f(glGetUniformLocation(shader->getProgram(), "SandHeight"), WaterHeight + 3.0f);
	glUniform1f(glGetUniformLocation(shader->getProgram(), "SnowHeight"), mSnowHeight);
	glUniform1f(glGetUniformLocation(shader->getProgram(), "TextureIteration"), mTextureIterationFactor);
	glUniform3fv(glGetUniformLocation(shader->getProgram(), "LightDirectionObjSpace"), 1, LightDirection);

	// Draw
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
	glBindVertexArray(terrainVBO);
	glDrawElements(GL_TRIANGLES, (mResolution - 1) * (mResolution - 1) * 2, GL_UNSIGNED_INT, 0);


	// --------------------------------
	// Cleanups...
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// deactive programobject
	glUseProgram(0);

	// disable texturestages and activate stage 0
	for(int i=0; i<7; ++i)
	{
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
}

bool Terrain::IsBorder(int x, int z)
{
	return x<=0 || x>=(mResolution-1) || z<=0 || z>=(mResolution-1);
}

void Terrain::Generate(int x1, int z1, int x2, int z2, float Interval)
{
    float EdgeHeight, centerHeight;
	int midX = (x1 + x2) / 2;
	int midY = (z1 + z2) / 2;

	if ((x2 - x1 < 2) && (z2 - z1 < 2))
	    return;
     
    centerHeight = (GetHeight(x1,z1) + GetHeight(x1,z2) + 
                          GetHeight(x2,z1) + GetHeight(x2,z2)) / 4.0f + 
                          random(Interval);
	centerHeight = fabs(centerHeight);
    SetHeight_OnlyZero(midX, midY, centerHeight);
    
	if(!IsBorder(x1, midY))
	{
		EdgeHeight = (GetHeight(x1,z1) + GetHeight(x1,z2)) / 2.0f + random(Interval);
		SetHeight_OnlyZero(x1, midY, fabs(EdgeHeight));
	}
	if(!IsBorder(x2, midY))
	{
		EdgeHeight = (GetHeight(x2,z1) + GetHeight(x2,z2)) / 2.0f + random(Interval);
		SetHeight_OnlyZero(x2, midY, fabs(EdgeHeight));
	}
	if(!IsBorder(midX, z1))
	{
		EdgeHeight = (GetHeight(x1,z1) + GetHeight(x2,z1)) / 2.0f + random(Interval);
		SetHeight_OnlyZero(midX, z1, fabs(EdgeHeight));
	}
	if(!IsBorder(midX, z2))
	{
		EdgeHeight = (GetHeight(x1,z2) + GetHeight(x2,z2)) / 2.0f + random(Interval);
		SetHeight_OnlyZero(midX, z2, fabs(EdgeHeight));
	}
    
	Interval /= 2;
	
	Generate(x1, z1, midX, midY, Interval);
	Generate(midX, z1, x2, midY, Interval);
	Generate(x1, midY, midX, z2, Interval);
	Generate(midX, midY, x2, z2, Interval);
}

void Terrain::SetHeight_OnlyZero(int x, int z, float height)
{
    if(GetHeight(x, z) == 0)
		vertices[GetArrayIndex(x,z)].pos.y = height;
}


void Terrain::SetPosition(int x, int z, const Vector3& v)
{
	vertices[GetArrayIndex(x,z)].pos = v;
}

const Vector3& Terrain::GetPosition(int GridX, int GridZ)
{
	return vertices[GridZ*mResolution+GridX].pos;
}

void Terrain::SetNormal(int x, int z, const Vector3& v)
{
	vertices[GetArrayIndex(x,z)].normal = v;
}
	
float Terrain::GetHeight(int x, int z) const
{
	return vertices[GetArrayIndex(x,z)].pos.y;
}

float Terrain::GetHeightInterp(float x, float z) const
{
	int x0 = static_cast<int>(floorf(x));
	int z0 = static_cast<int>(floorf(z));
	int x1 = static_cast<int>(ceilf(x));
	int z1 = static_cast<int>(ceilf(z));
	float height00 = GetHeightSafe(x0, z0);
	float height10 = GetHeightSafe(x1, z0);
	float height01 = GetHeightSafe(x0, z1);
	float height11 = GetHeightSafe(x1, z1);
	float tx = x - x0;
	float tz = z - z0;

	float val0 = interpolateLinear(height00, height10, tx);
	float val1 = interpolateLinear(height01, height11, tx);

	return interpolateLinear(val0, val1, tz);
}

float Terrain::GetHeightSafe(int x, int z) const
{
	if(x < 0)
		x = 0;
	if(x > mResolution-1)
		x = mResolution-1;
	if(z < 0)
		z = 0;
	if(z > mResolution-1)
		z = mResolution-1;
	return vertices[GetArrayIndex(x,z)].pos.y; 
}

