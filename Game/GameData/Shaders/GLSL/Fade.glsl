#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;

#ifdef COMPILEPS
    uniform float cFadeValue = 0.5;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
    vec3 color = texture2D(sDiffMap, vScreenPos.xy).rgb;
    color = mix(color, vec3(0.0), cFadeValue);
    gl_FragColor = vec4(color, 1.0);
}
