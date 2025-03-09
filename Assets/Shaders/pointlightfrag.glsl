#version 400 core

uniform sampler2D depthTex; //need the depth texture to get world space position of fragment light covers
uniform sampler2D normTex; 

uniform sampler2D diffuseTexLight; 
uniform sampler2D specularTexLight; 

uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

in Vertex {
   vec2 texCoord;
   }IN;

void main(void) {
    vec2  texCoord   = IN.texCoord;
    float depth      = texture(depthTex, texCoord.xy).r;
    vec3  ndcPos     = vec3(texCoord, depth) * 2.0 - 1.0;
    vec4  invClipPos = inverseProjView * vec4(ndcPos, 1.0);
    vec3  worldPos   = invClipPos.xyz / invClipPos.w;

    float dist       = length(lightPos - worldPos);
    float atten      = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
    vec4 diffuseOut;
    vec4 specularOut;
    if (atten == 0.0) {
      diffuseOut = texture(diffuseTexLight,texCoord.xy);
      specularOut = texture(specularTexLight,texCoord.xy);
    
    }else{

    vec3 normal   = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
    vec3 incident = normalize(lightPos - worldPos);
    vec3 viewDir  = normalize(cameraPos - worldPos);
    vec3 halfDir  = normalize(incident + viewDir);

    float lambert    = clamp(dot(incident, normal), 0.0, 1.0); 
    float rFactor    = clamp(dot(halfDir, normal), 0.0, 1.0);
    float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
    specFactor       = pow(specFactor, 80.0); 
    vec3 attenuated  = lightColour.xyz * atten;

    vec4 diffuseCalculated =  vec4(attenuated * lambert, 1.0);
    vec4 specularCalculated =  vec4(attenuated * specFactor * 0.33, 1.0);
    diffuseOut = texture(diffuseTexLight,texCoord.xy) + diffuseCalculated;
    specularOut = texture(specularTexLight,texCoord.xy) +  specularCalculated;
    
    }
    diffuseOut = min(diffuseOut,vec4(1,1,1,1));
    specularOut = min(specularOut,vec4(1,1,1,1));
    diffuseOutput    = diffuseOut;
    specularOutput   = specularOut;
}