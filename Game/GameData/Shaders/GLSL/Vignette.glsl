#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "ScreenPos.glsl"
#include "Transform.glsl"

varying vec2 vScreenPos;

#ifdef COMPILEPS
    uniform float cVignetteRadius = 0.75;
    uniform float cVignetteSoftness = 0.45;
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
    // Исходный пиксель вьюпорта.
    vec3 color = texture2D(sDiffMap, vScreenPos.xy).rgb;
    
    // Расстояние до центра экрана. В центре экрана dist = 0, в углу экрана dist = 0.71.
    float dist = distance(vScreenPos.xy, vec2(0.5, 0.5));

    // Размытый овал. Результат в диапазоне [немного больше 0, 1].
    // Эрмитова интерполяция smoothstep по спецификации требует, чтобы левое значение
    // было меньше правого, но тут все работает.
    // Когда dist >= cVignetteRadius (dist >= 0.75), результат равен нулю. Черный цвет далеко от центра, в углах экрана.
    // Когда dist <= cVignetteRadius - cVignetteSoftness (dist <= 0.3), результат равен 1. Белый цвет в центре экрана.
    // Когда dist между этими значениями (0.75 > dist > 0.3), то результат плавно интерполируется между 0 и 1.
    float vignette = smoothstep(cVignetteRadius, cVignetteRadius - cVignetteSoftness, dist);

    // Прозрачность виньетки = 50%.
    color = mix(color, color * vignette, 0.5);

    gl_FragColor = vec4(color, 1.0);
}
