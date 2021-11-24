#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
//layout(location = 2) in vec3 a_Normal;

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;
//layout(location = 2) out vec3 v_Normal;

layout(location = 3) out vec2 v_FragPos;

layout(push_constant) uniform Push {
    float iTime;
} push;

void main() {
    v_Color    = vec3(1.0, 0.0, 0.0);
    v_TexCoord = a_TexCoord;
    //v_Normal   = mat3(transpose(inverse(ubo.model))) * a_Normal;

    //v_FragPos = vec3(pushConsts.model * ubo.model * vec4(a_Position, 1.0));
    //localPos = pushConsts.model * vec4(localPos, 1.0);
    v_FragPos = a_Position * vec2(1280, 720);

    gl_Position = vec4(a_Position * -2.0 + 1.0, 0.0, 1.0);
}