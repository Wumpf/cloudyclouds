#pragma once

class Vector3;

class Terrain
{
public:
    Terrain(unsigned int Resolution, float TextureIteration, float SnowHeight, float Interval, const Vector3& LightDirection);
    ~Terrain();

    void Draw(float WaterHeight, const Vector3& LightDirection);

    inline float GetHeight(int x, int z) const;
	float GetHeightInterp(float x, float z) const;	// bilinear interpolated
	float GetHeightSafe(int x, int z) const;
    
	float GetSnowHeight() const { return mSnowHeight; }

	// Returns the vertex normal
	const Vector3& GetNormal(int x, int z) const;

private:
	inline const Vector3& GetPosition(int GridX, int GridZ);
	inline void SetPosition(int x, int z, const Vector3& v);
	inline void SetNormal(int x, int z, const Vector3& v);
	void SetHeight_OnlyZero(int x, int z, float Height);
	unsigned int GetArrayIndex(int x, int z) const { return x*mResolution+z; }

	void Generate(int x1, int z1, int x2, int z2, float Interval);
	bool IsBorder(int x, int z);

	const int   mResolution;  // number of points in x and z direction
	GLuint terrainVBO;
	GLuint terrainIBO;
	GLuint terrainVAO;

	const float mTextureIterationFactor;
	const float mSnowHeight;
	GLuint mSnowTexture;
	GLuint mGrassTexture;
	GLuint mRockTexture;
	GLuint mSandTexture;

	GLuint mSnowBump;
	GLuint mGrassBump;
	GLuint mRockBump;
	GLuint mSandBump;

	std::unique_ptr<class ShaderObject> shader;
	std::unique_ptr<struct TerrainVertex[]> vertices;
};