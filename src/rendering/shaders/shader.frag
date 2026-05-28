R"(

#version 460 core

in vec2 position;
in vec3 color;

out vec4 fragment_output;

void main() {
    if (dot(position, position) > 1.0) {
        discard;
    }
    fragment_output = vec4(color, 1.0);
}

)"