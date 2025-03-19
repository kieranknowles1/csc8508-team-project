#version 330 core

in vec2 texCoord;
in vec4 Color;

out vec4 fragColor;

uniform sampler2D mainTex; //UI texture
uniform bool hasTexture; //Check to see if the element has a texture

void main() {
	if (hasTexture) {
		vec4 texColor = texture(mainTex, texCoord);
		fragColor = texColor * Color;
		//fragColor = vec4(texCoord ,0.0 ,1.0);
		}
	else {
		fragColor = Color;
		//fragColor = vec4(1,1,1,1);
	}
}