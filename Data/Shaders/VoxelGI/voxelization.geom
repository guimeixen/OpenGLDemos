#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 uvGeom[];
in vec3 normalGeom[];
in vec3 worldPosGeom[];
in vec4 lightSpacePosGeom[];

out vec3 normal;
out vec2 uv;
out vec4 lightSpacePos;
flat out int axis;

uniform mat4 orthoProjX;
uniform mat4 orthoProjY;
uniform mat4 orthoProjZ;

void main()
{
	// Calculate normal
	const vec3 faceN = cross(worldPosGeom[1] - worldPosGeom[0], worldPosGeom[2] - worldPosGeom[0]);
	const float NdotX = abs(faceN.x);			// NdotX = N.x * X.x + N.y * X.y + N.z * X.z = N.x * 1.0 + N.y * 0.0 + N.z *0.0 = N.x
	const float NdotY = abs(faceN.y);
	const float NdotZ = abs(faceN.z);

	mat4 proj;
		
	if (NdotZ > NdotX && NdotZ > NdotY)
	{
		axis = 3;
		proj = orthoProjZ;	
	}
	else if (NdotX > NdotY && NdotX > NdotZ)
	{
		axis = 1;
		proj = orthoProjX;
	}
	else
	{
		axis = 2;
		proj = orthoProjY;
	}
	
	gl_Position = proj * gl_in[0].gl_Position;
	normal = normalGeom[0];
	uv = uvGeom[0];
	lightSpacePos = lightSpacePosGeom[0];
	
	EmitVertex();
	
	gl_Position = proj * gl_in[1].gl_Position;
	normal = normalGeom[1];
	uv = uvGeom[1];
	lightSpacePos = lightSpacePosGeom[1];
	
	EmitVertex();
	
	gl_Position = proj * gl_in[2].gl_Position;
	normal = normalGeom[2];
	uv = uvGeom[2];
	lightSpacePos = lightSpacePosGeom[2];
	
	EmitVertex();
	
	EndPrimitive();
}