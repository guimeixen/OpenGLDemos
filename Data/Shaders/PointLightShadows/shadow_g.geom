#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

layout(location = 0) out vec3 worldPos;

uniform mat4 lightTransform[6];

void main()
{
	for (int face = 0; face < 6; ++face)
	{		
		for (int i = 0; i < 3; ++i)
		{
			worldPos = gl_in[i].gl_Position.xyz;
			gl_Position = lightTransform[face] * gl_in[i].gl_Position;
			gl_Layer = face;
			EmitVertex();			
		}
		EndPrimitive();
	}
}