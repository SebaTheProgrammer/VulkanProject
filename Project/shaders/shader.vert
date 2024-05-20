#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push
{
    mat4 transform; //projection*view*model
    mat4 modelMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 3.0, -1.0));

const float AMBIENT_INTENSITY = 0.02;

void main() 
{
    gl_Position = push.transform * vec4(position, 1.0);

    vec3 normalWordSpace = normalize(mat3(push.modelMatrix) * normal);

    float lightIntensity = AMBIENT_INTENSITY + max(dot(normalWordSpace, DIRECTION_TO_LIGHT),0)/2;

    fragColor=lightIntensity*color;
}