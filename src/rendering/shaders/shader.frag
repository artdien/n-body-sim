R"(

#version 460 core

in vec2 position;
in vec4 color;

out vec4 fragment_output;

void main() {
    if (dot(position, position) > 1.0) {
        discard;
    }
    fragment_output = color;
}

)"