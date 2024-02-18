float calculateDistance(vec2 point1, vec2 point2) {
    float xDist = point2.x - point1.x;
    float yDist = 0.60f * (point2.y - point1.y);
    return sqrt((xDist * xDist) + (yDist * yDist));
}

float calculateDistanceWorldAspect(vec2 point1, vec2 point2) {
    float xDist = 0.925f * (point2.x - point1.x);
    float yDist = point2.y - point1.y;
    return sqrt((xDist * xDist) + (yDist * yDist));
}

float calculateBoundingBoxDistance(vec2 point1, vec2 point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = abs(point2.y - point1.y);
    return max(xDist, yDist);
}

vec2 calculateNormalizedUV(vec2 point1, vec2 point2, float width) {
    vec2 uv = vec2((point2.x - point1.x) / width, (point2.y - point1.y) / width) + vec2(0.5, 0.5);
    return uv;
}

float calculateBoundingBoxDistanceIcon(vec2 point1, vec2 point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = 0.60f * abs(point2.y - point1.y);
    return max(xDist, yDist);
}

vec2 calculateNormalizedUVWorldAspect(vec2 point1, vec2 point2, float width) {
    vec2 uv = vec2(0.925f * (point2.x - point1.x) / width, (point2.y - point1.y) / width) + vec2(0.5, 0.5);
    return uv;
}


vec2 calculateNormalizedUVIcon(vec2 point1, vec2 point2, float width) {
    vec2 uv = vec2((point2.x - point1.x) / width, 0.60f * (point2.y - point1.y) / width) + vec2(0.5, 0.5);
    return uv;
}

vec2 calculateNormalizedUVOrrery(vec2 point1, vec2 point2, float width) {
    vec2 uv = vec2(1.5f * (point2.x - point1.x) / width, 0.60f * (point2.y - point1.y) / width) + vec2(0.5, 0.5);
    return uv;
}

vec3 slerp(vec3 start, vec3 end, float t) {
    // Dot product - the cosine of the angle between the vectors
    float dot = max(min(dot(start, end), 1.0), -1.0); // Clamp to avoid floating point errors
    float theta = acos(dot) * t; // Angle between start and end, scaled by t

    // Relative vector from start to end with start's influence removed
    vec3 relativeVec = normalize(end - start * dot);
    // Spherical interpolation formula
    return start * cos(theta) + relativeVec * sin(theta);
}
