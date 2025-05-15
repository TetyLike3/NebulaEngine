#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 viewportResolution;
layout(location = 3) in float time;

layout(location = 0) out vec4 outColor;

void main() {
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragTexCoord/viewportResolution.xy;

    float mdf = 0.4; // increase for noise amount 
    float noise = (fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453));
    vec4 tex = texture(iChannel0, uv);
    
    mdf *= sin(time) + .1; // animate the effect's strength
    
    outColor = tex - noise * mdf;
}