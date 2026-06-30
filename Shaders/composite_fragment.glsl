#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D hdrScene;
uniform sampler2D bloomBlur;
uniform float     bloomStrength;
uniform float     exposure;

const float PI = 3.14159265359;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec3 hdr   = texture(hdrScene,  TexCoords).rgb;
    vec3 bloom = texture(bloomBlur, TexCoords).rgb;


    vec3 result = hdr + bloom * bloomStrength;


    result = ACESFilm(result * exposure);
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}