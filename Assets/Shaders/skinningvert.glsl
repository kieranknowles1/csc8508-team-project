#version 400 //No core?

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec2 texCoord;
in vec4 jointWeights;
in ivec4 jointIndices;

uniform mat4 joints[128];//not sure if this is the right number atm

out Vertex {
    vec2 texCoord;
    } OUT;

void main(void) { 
     vec4 localPos = vec4(position, 1.0f);
     vec4 skelPos  = vec4(0, 0, 0, 0);

     for (int i = 0; i < 4; ++i) { //not currently sure why 4 specifically. Maybe not right in this case
         int jointIndex = jointIndices[i];
         float jointWeight = jointWeights[i];

         skelPos += joints[jointIndex] * localPos * jointWeight;
         }
    mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    gl_Position = mvp * vec4(skelPos.xyz, 1.0);
    OUT.texCoord = texCoord;
}
//if possible might be nice to add all the necessary stuff from scene.vert to allow the model to be lit



