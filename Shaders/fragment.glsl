#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 vFragPos;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

void main()
{  
    // 1. Sample and blend the two textures
    vec4 texColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);

    // 2. Calculate fog factor
    float distance  = length(vFragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);

    // 3. Apply fog on top of the texture color
    if (fogFactor <= 0.0) discard;
    vec3 finalColor = mix(fogColor, texColor.rgb, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}