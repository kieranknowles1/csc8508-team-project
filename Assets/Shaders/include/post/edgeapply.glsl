vec4 doEdgeDetect(vec4 colour, vec2 texCoord,vec2 invWinSize) {
    vec2 offsets[25] = vec2[] (
        vec2(-2.0,  2.0), vec2(-1.0,  2.0), vec2(0.0,  2.0), vec2(1.0,  2.0), vec2(2.0,  2.0),
        vec2(-2.0,  1.0),      vec2(-1.0,  1.0),      vec2(0.0,  1.0),      vec2(1.0,  1.0),      vec2(2.0,  1.0),
        vec2(-2.0,  0.0),           vec2(-1.0,  0.0),           vec2(0.0,  0.0),           vec2(1.0,  0.0),           vec2(2.0,  0.0),
        vec2(-2.0, -1.0),      vec2(-1.0, -1.0),      vec2(0.0, -1.0),      vec2(1.0, -1.0),      vec2(2.0, -1.0),
        vec2(-2.0, -2.0), vec2(-1.0, -2.0), vec2(0.0, -2.0), vec2(1.0, -2.0), vec2(2.0, -2.0)
    );

   vec3 normalWorld = normalSample(texCoord);
   for (int i = 0; i < 25; i++) {
        vec2 offset = offsets[i] * invWinSize;
       vec3 normalWorld2 =normalSample(texCoord+offset); // check surrounding normals
       if(distance(normalWorld,normalWorld2) >= 0.8f){ // normals are substantially different
           colour = vec4(0,0,0,1);
           break;
        }
    }
   return colour;
 }
