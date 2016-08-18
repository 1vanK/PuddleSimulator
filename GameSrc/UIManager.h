/*
Так как к UI-элементам нельзя прикрепить компоненты, то все элементы
контролируются в этой подсистеме.
*/

#pragma once
#include <Urho3D/Urho3DAll.h>
#include "MyButton.h"

#define UI_MANAGER GetSubsystem<UIManager>()

class UIManager : public Object
{
    URHO3D_OBJECT(UIManager, Object);

public:
    // Индекс выбранного цвета.
    int selectedColor_ = 0;
    // Дискета, информирующая о сохранении.
    BorderImage* floppyImage_;

    UIManager(Context* context);

    // Возвращает элемент интерфейса под курсором мыши.
    UIElement* GetHoveredElement();
    
private:
    // Цветовые кнопки.
    MyButton* redButton_;
    MyButton* orangeButton_;
    MyButton* yellowButton_;
    MyButton* greenButton_;
    MyButton* blueButton_;
    MyButton* indigoButton_;
    MyButton* violetButton_;

    // Ходы.
    Text* turnsText_;
    MyButton* decreaseTurnsButton_;
    MyButton* increaseTurnsButton_;

    // Кнопки смены уровней.
    MyButton* prevButton_;
    MyButton* replayButton_;
    MyButton* nextButton_;

    // Проигрывание звуков.
    void PlayFail();
    void PlayClick();

    // Возвращает позицию на экране, в которой должна находиться цветовая кнопка
    // в зависимости от выбранного цвета.
    IntVector2 GetColorButtonTargetPos(int color);
    // Сохраняет текущие координаты кнопки в переменную элемента с типом Vector2.
    // Вещественный тип необходим при анимации движения.
    void SavePositionToVar(UIElement* element);
    // Может ли игрок перейти на следующий уровень.
    bool IsAvailableNextLevel();
    // Может ли игрок перейти на предыдущий уровень.
    bool IsAvailablePrevLevel();
    // Обновляет видимость элементов.
    void UpdateElementsVisibility();

    // Плавное перемещение кнопок в целевую позицию.
    void UpdateColorButtonPos(MyButton* button, float timeStep);
    void UpdateNextLevelButtonPos(float timeStep);

    // Остаток ходов turnsRemain_ изменяется в Update, поэтому используем PostUpdate для
    // обновления соответствующего ему элемента интерфейса turnsText_.
    // Заодно остальная UI-логика тоже тут.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    
    void HandleColorButtonClick(StringHash eventType, VariantMap& eventData);
    void HandleIncreaseTurnsButtonClick(StringHash eventType, VariantMap& eventData);
    void HandleDecreaseTurnsButtonClick(StringHash eventType, VariantMap& eventData);
    void HandlePrevButtonClick(StringHash eventType, VariantMap& eventData);
    void HandleReplayButtonClick(StringHash eventType, VariantMap& eventData);
    void HandleNextButtonClick(StringHash eventType, VariantMap& eventData);
};
