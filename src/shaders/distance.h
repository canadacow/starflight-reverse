float calculateDistance(vec2 point1, vec2 point2) {
    float xDist = point2.x - point1.x;
    float yDist = 0.60f * (point2.y - point1.y);
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

vec2 calculateNormalizedUVIcon(vec2 point1, vec2 point2, float width) {
    vec2 uv = vec2((point2.x - point1.x) / width, 0.60f * (point2.y - point1.y) / width) + vec2(0.5, 0.5);
    return uv;
}