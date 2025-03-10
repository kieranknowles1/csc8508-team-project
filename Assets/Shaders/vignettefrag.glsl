#version 330 core

#include "include/post/vingette.glsl"

uniform sampler2D diffuseTex;
uniform vec2 windowSize;
uniform vec3 effectColour;
uniform float intensity = 1.0f;
uniform float time;
uniform bool vignetteOn = false;

in Vertex {
   vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void) {
  if (!vignetteOn) { //if vignette not enabled, simply return the scene without performing any post processing
    fragColor.rgb = texColour;
    fragColor.a = 1;
    return;
  }

  vec3 original = texture(diffuseTex, IN.texCoord).rgb;
  vec2 ndcPos = gl_FragCoord.xy / windowSize;
  fragColor.rgb = applyVingette(original, ndcPos, intensity, effectColour, time);
  fragColor.a = 1.0f;
}
