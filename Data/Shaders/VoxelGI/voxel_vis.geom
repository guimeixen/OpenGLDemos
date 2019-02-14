#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 uvGeom[];
in vec3 normalGeom[];
in vec3 worldPosGeom[];

out vec3 worldPos;
out vec3 normal;

void main()
{
	// Calculate normal
	const vec3 p1 = worldPosGeom[1] - worldPosGeom[0];
	const vec3 p2 = worldPosGeom[2] - worldPosGeom[0];
	const vec3 p = abs(cross(p1, p2));
	
	for (uint i = 0; i < 3; i++)
	{
		worldPos = worldPosGeom[i];
		normal = worldPosGeom[i];
		
		if (p.z > p.x && p.z > p.y)
		{
			gl_Position = vec4(worldPos.x, worldPos.y, 0.0, 1.0);
		}
		else if (p.x > p.y && p.x > p.z)
		{
			gl_Position = vec4(worldPos.y, worldPos.z, 0.0, 1.0);
		}
		else
		{
			gl_Position = vec4(worldPos.x, worldPos.z, 0.0, 1.0);
		}
		EmitVertex();
	}
	EndPrimitive();
}