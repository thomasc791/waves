#version 440 core

struct Wave {
    float uPrev;
    float u;
    float uNext;
    float _padding;
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
layout(binding = 3, std430) buffer WaveArray {
    Wave data[];
};
layout(location = 1) uniform uvec2 texSize;

void main() {
    uint curr = gl_GlobalInvocationID.x;

    data[curr].uPrev = data[curr].u;
    data[curr].u = data[curr].uNext;
}
