#version 440 core

struct Wave {
    float uPrev;
    float u;
    float uNext;
    float _padding;
};

layout(local_size_x = 128, local_size_y = 8, local_size_z = 1) in;
layout(binding = 3, std430) buffer WaveArray {
    Wave data[];
};
layout(location = 1) uniform uvec2 texSize;

void main() {
    uint i = gl_GlobalInvocationID.x + 1;
    uint j = gl_GlobalInvocationID.y + 1;
    uint curr = j * texSize.x + i;

    data[curr].uPrev = data[curr].u;
    data[curr].u = data[curr].uNext;
}
