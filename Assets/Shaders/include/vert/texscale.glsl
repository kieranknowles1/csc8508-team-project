#ifndef TEXSCALE_H
#define TEXSCALE_H

bool nearZero(float val) {
    return abs(val) < 0.001;
}

vec2 scaleUv(vec2 input, vec3 scale, vec3 normal, bool repeats) {
    if (!repeats) return input;

    vec2 output = input;
	//before applying mvp matrix, vertices are in local space and therefore axis aligned
	//We check to see what axis the plane in question is facing before determining how to scale the texCoords
	//Scaling currently works well for cuboids but not too well for curved surfaces like capsules and spheres
	vec3 direction = normalize(normal);
	if (!nearZero(direction.x) && nearZero(direction.y) && nearZero(direction.z)) {//face is perpendicular to x plane => on y-z plane
	    output.x *= scale.z;
	    output.y *= scale.y;
    } else if (nearZero(direction.x) && !nearZero(direction.y) && nearZero(direction.z)) {//face is perpendicular to y plane => on x-z plane

	    output.x *= scale.x;
	    output.y *= scale.z;
    } else if (nearZero(direction.x) && nearZero(direction.y) && !nearZero(direction.z)) {//face is perpendicular to z plane => on x-y plane
	    output.x *= scale.x;
	    output.y *= scale.y;
    }
    return output;
}

#endif
