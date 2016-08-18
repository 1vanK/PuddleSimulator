#include "Utils.h"

float ToTarget(float currentValue, float targetValue, float speed, float timeStep)
{
    if (currentValue < targetValue)
    {
        float result = currentValue + speed * timeStep;
        
        // Проскочили целевое значение.
        if (result > targetValue)
            result = targetValue;

        return result;
    }
    else if (currentValue > targetValue)
    {
        float result = currentValue - speed * timeStep;

        if (result < targetValue)
            result = targetValue;

        return result;
    }
    else // currentValue == targetValue
    {
        return targetValue;
    }
}

Vector2 ToTarget(const Vector2& currentPosition, const Vector2& targetPosition, float speed, float timeStep)
{
    if (currentPosition.Equals(targetPosition))
        return targetPosition;

    Vector2 direction = (targetPosition - currentPosition).Normalized();
    Vector2 newPosition = currentPosition + direction * timeStep * speed;

    // Проверка перед попыткой определения нового направления, так как нормализация
    // вектора с нулевой длиной - плохая идея.
    if (newPosition.Equals(targetPosition))
        return targetPosition;

    // Если направление от новой позиции до целевой точки изменилось на противоположное,
    // значит мы проскочили пункт назначения.
    Vector2 newDirection = (targetPosition - newPosition).Normalized();
    if (newDirection.DotProduct(direction) < 0.0f)
        return targetPosition;
    
    return newPosition;
}
