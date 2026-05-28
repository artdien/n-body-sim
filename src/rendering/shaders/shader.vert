R"(

#version 460 core

struct Body {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
};

layout(binding = 0, std430) readonly buffer SSBO {
    Body bodies[];
} ssbo;

out vec2 position;
out vec3 color;

uniform float body_radius;
uniform vec3 body_color;
uniform mat4 projection;

const vec2[6] quad = {
    {-1.0, +1.0}, {+1.0, -1.0}, {+1.0, +1.0}, // Triangle 1
    {-1.0, +1.0}, {-1.0, -1.0}, {+1.0, -1.0}, // Triangle 2
};

void main() {
    Body body = ssbo.bodies[gl_InstanceID];

    position = quad[gl_VertexID];
    color = body_color;

    gl_Position = projection * vec4(body_radius * position + body.position.xy, 0.0, 1.0);
}

)"