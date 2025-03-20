#ifndef VINGETTE_H
#define VINGETTE_H

vec3 applyVingette(vec3 original, vec2 ndcPos, float intensity, vec3 effectColor, float time) {
    vec2 centrePos = vec2(0.5, 0.5);
    float softness = 1.7; // Controls the smooth transiti
    //add vignette:
    //need to change fragment colour (brightness and colour) based on distance from screen centre
    //distance from centre. Make sure the fragment coordinate is converted to ndc first
    float dist = length(ndcPos - centrePos);
    //intensity pulsates in time
    float intenseFactor = intensity * (1.0 + (0.25 * sin(time * 3.0)));

     float vignetteFactor = smoothstep(1.0, 1.0 - softness, dist * intenseFactor);
     vignetteFactor = 1.0 - intensity + (intensity * vignetteFactor); 

    //last parameter is a weighting, higher weighting = lower influence of vignette
    vec3 outColor = mix(effectColor, original, vignetteFactor);
    //we want to make sure the weighting is clamped between 0 and 1.
    return outColor;
}

#endif
