#version 330 core //Basically like scene.frag but without the lighting calculations

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

in Vertex {
    vec4 colour;
	vec2 texCoord;
	//vec4 shadowProj; temporarily removing shadows
	vec3 normal;
	vec3 worldPos;
	//Added for normal mapping:
	vec3 tangent;
	vec3 binormal;
} IN;

out vec4 fragColour[2];

void main(void)   { //MAY NEED TO ADD THE BOOL UNIFORMS LATER FOR CONDITIONALS LIKE IF NORMAL MAP
    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = texture2D(normalTex, IN.texCoord).rgb * 2.0 - 1.0;
	normal      = normalize(TBN * normalize(normal));

	fragColour[0] = texture2D(diffuseTex, IN.texCoord);
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, 1.0);
	}