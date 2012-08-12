#version 330

layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	mat4 InverseViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
	vec3 CameraDir;
};

uniform float TextureIteration;
uniform vec3 LightDirectionObjSpace;
uniform mat4 WorldViewProjection;

in vec3 in_position;
in vec3 in_normal;
in float in_shadowing;

out vec3 normal;
out vec2 texture_coordinate; 
out float height;
out float vertexShadowing;
out vec3 LightDirectionTangentSpace;

void main()
{
	// passing the heigh trough
	height = in_position.y;

	// Transforming The Vertex
	gl_Position = ViewProjection * vec4(in_position, 1.0);

	// Transforming the normal is not needed, because we don't stretch or rotate our terrain!
	normal = in_normal; 

	// transform Lightdirection into tangentspace if necessary - we know the direction of the texture, so we can compute it on the fly
	vec3 Tangent = cross(in_normal, vec3(1, 0, 0));
	vec3 BiNormal = cross(Tangent, in_normal);
	mat3 rotmat = mat3(Tangent, BiNormal, in_normal);
	LightDirectionTangentSpace = rotmat * normalize(-LightDirectionObjSpace);

	// generate texture cordinates
	texture_coordinate = in_position.xz * TextureIteration;

	// pass vertexshadowing through
	vertexShadowing = in_shadowing;
}