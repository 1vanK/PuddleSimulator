#pragma once
#include <Urho3D/Urho3DAll.h>

// Плавное изменение числа в сторону целевого значения с определенной скоростью.
float ToTarget(float currentValue, float targetValue, float speed, float timeStep);

// Плавное изменение позиции в сторону пункта назначения с определенной скоростью.
Vector2 ToTarget(const Vector2& currentPosition, const Vector2& targetPosition, float speed, float timeStep);
