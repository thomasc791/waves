#version 440 core

struct Point {
    vec2 pos;
    float angle;
    float radius;
    vec3 c;
    float hasFood;
};

struct Wave {
    float uPrev;
    float u;
    float uNext;
    float _padding;
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f) uniform image2D imgOutput;
layout(binding = 3, std430) buffer WaveArray {
    Wave data[];
};
layout(location = 0) uniform float t;
layout(location = 1) uniform uvec2 texSize;

void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    uint j = uint(floor(gl_GlobalInvocationID.x / texSize.x));
    uint i = gl_GlobalInvocationID.x % texSize.x;
    ivec2 texelCoord = ivec2(i, j);
    uint curr = gl_GlobalInvocationID.x;

    data[curr].uNext = 0.9999 * (2 * data[curr].u - data[curr].uPrev + 0.25 * (
                    data[curr - 1].u + data[curr + 1].u + data[curr - texSize.x].u + data[curr + texSize.x].u - 4 * data[curr].u));

    value.x = mix(1.0, 0.0, int(data[curr].uNext >= 0) * -100 * (data[curr].uNext) + data[curr].u);
    value.y = mix(1.0, 0.0, 10 * abs(data[curr].uNext));
    value.z = mix(1.0, 0.0, int(data[curr].uNext >= 0) * 100 * data[curr].uNext + data[curr].u);

    imageStore(imgOutput, texelCoord, value);
}
