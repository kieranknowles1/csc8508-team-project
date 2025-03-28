#version 400 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position; 

layout(location = 1) in vec4 colour; 
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 5) in vec4 jointWeights; //location 5 to match enum in struct in mesh.h
layout(location = 6) in ivec4 jointIndices; 

uniform vec4 objectColour; //added to fit with scene.vert

uniform mat4 joints[128];

out Vertex { 
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 tangent;
    vec3 binormal;
    } OUT;

void main(void) { 
     vec4 localPos = vec4(position, 1.0f);
     vec4 skelPos  = vec4(0, 0, 0, 0);

     for (int i = 0; i < 4; ++i) { 
         int jointIndex = jointIndices[i];
         float jointWeight = jointWeights[i];

         skelPos += joints[jointIndex] * localPos * jointWeight;
         }
    mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    gl_Position = mvp * vec4(skelPos.xyz, 1.0);
    OUT.texCoord = texCoord;

    //calulating the rest as in scenevert:
    mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));
    OUT.worldPos 	= ( modelMatrix * vec4 ( position ,1)). xyz ;
	OUT.normal 		= normalize ( normalMatrix * normalize ( normal ));
	OUT.tangent     = normalize ( normalMatrix * normalize ( tangent.xyz)); //calculate tangent for normal mapping
	OUT.binormal    = cross ( OUT.tangent, OUT.normal) * tangent.w; //calculate the binormal for normal mapping
	OUT.colour		= objectColour;
}




