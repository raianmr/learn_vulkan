#version 460

vec2 points[3] = { { 0.0, -0.5 }, { 0.5, -0.5 }, { 0.0, 0.0 } };

void main() {
    gl_Position = vec4(points[gl_VertexIndex], 0.0, 1.0);
}