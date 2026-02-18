#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;
layout (location = 3) in uint inOffset;

uniform mat4 mvp;
uniform mat4 model;

out vec3 normal;
out vec3 fragPosition;

uniform vec3 cameraPosition;

uniform int voxelSpaceSize;

void main() {
	uint n = uint(voxelSpaceSize);
	uint zDirection = inOffset % n;
	uint yDirection = (inOffset / n) % n;
	uint xDirection = inOffset / (n * n); 

	gl_Position = mvp * vec4(inPos.x + float(xDirection), inPos.y + float(yDirection), inPos.z + float(zDirection), 1.0);

	normal = inNormal;
	fragPosition = vec3(model * vec4(inPos, 1.0));
}
