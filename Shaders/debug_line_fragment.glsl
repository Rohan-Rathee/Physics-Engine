#version 450 core

in VS_OUT {
    vec3 vertexColor;
} fs_in;

out vec4 FragColor;

void main()
{
    FragColor = vec4(fs_in.vertexColor, 1.0);
}
