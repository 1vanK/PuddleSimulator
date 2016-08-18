#include "UIManager.h"
#include "Urho3DAliases.h"
#include "Utils.h"
#include "ContainerLogic.h"
#include "Config.h"

#define NEXT_BUTTON_NORMAL_POS IntVector2(-250, 310)

UIManager::UIManager(Context* context) : Object(context)
{
    INPUT->SetMouseVisible(true);

    // Создаем отладочный худ.
    XMLFile* defaultStyle = GET_XML_FILE("UI/DefaultStyle.xml");
    DebugHud* debugHud = ENGINE->CreateDebugHud();
    debugHud->SetDefaultStyle(defaultStyle);

    // Остальные элементы используют кастомный стиль.
    XMLFile* style = GET_XML_FILE("UI/Style.xml");
    UI_ROOT->SetDefaultStyle(style);

    // Красная кнопка.
    redButton_ = UI_ROOT->CreateChild<MyButton>();
    redButton_->SetStyle("ColorButton");
    redButton_->SetNormalColor(Color(0.75f, 0.0f, 0.0f));
    redButton_->SetHoverColor(Color(1.0f, 0.0f, 0.0f));
    redButton_->SetPressedColor(Color(1.0f, 0.0f, 0.0f));
    redButton_->SetPosition(GetColorButtonTargetPos(0));
    redButton_->SetVar("Color", 0); // Номер цвета сохраняется в переменной элемента.
    SavePositionToVar(redButton_);
    SubscribeToEvent(redButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Оранжевая кнопка.
    orangeButton_ = UI_ROOT->CreateChild<MyButton>();
    orangeButton_->SetStyle("ColorButton");
    orangeButton_->SetNormalColor(Color(0.75f, 0.375f, 0.0f));
    orangeButton_->SetHoverColor(Color(1.0f, 0.5f, 0.0f));
    orangeButton_->SetPressedColor(Color(1.0f, 0.5f, 0.0f));
    orangeButton_->SetPosition(GetColorButtonTargetPos(1));
    orangeButton_->SetVar("Color", 1);
    SavePositionToVar(orangeButton_);
    SubscribeToEvent(orangeButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Жёлтая кнопка.
    yellowButton_ = UI_ROOT->CreateChild<MyButton>();
    yellowButton_->SetStyle("ColorButton");
    yellowButton_->SetNormalColor(Color(0.75f, 0.75f, 0.0f));
    yellowButton_->SetHoverColor(Color(1.0f, 1.0f, 0.0f));
    yellowButton_->SetPressedColor(Color(1.0f, 1.0f, 0.0f));
    yellowButton_->SetPosition(GetColorButtonTargetPos(2));
    yellowButton_->SetVar("Color", 2);
    SavePositionToVar(yellowButton_);
    SubscribeToEvent(yellowButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Зелёная кнопка.
    greenButton_ = UI_ROOT->CreateChild<MyButton>();
    greenButton_->SetStyle("ColorButton");
    greenButton_->SetNormalColor(Color(0.0f, 0.75f, 0.0f));
    greenButton_->SetHoverColor(Color(0.0f, 1.0f, 0.0f));
    greenButton_->SetPressedColor(Color(0.0f, 1.0f, 0.0f));
    greenButton_->SetPosition(GetColorButtonTargetPos(3));
    greenButton_->SetVar("Color", 3);
    SavePositionToVar(greenButton_);
    SubscribeToEvent(greenButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Голубая кнопка.
    blueButton_ = UI_ROOT->CreateChild<MyButton>();
    blueButton_->SetStyle("ColorButton");
    blueButton_->SetNormalColor(Color(0.0f, 0.75f, 0.75f));
    blueButton_->SetHoverColor(Color(0.0f, 1.0f, 1.0f));
    blueButton_->SetPressedColor(Color(0.0f, 1.0f, 1.0f));
    blueButton_->SetPosition(GetColorButtonTargetPos(4));
    blueButton_->SetVar("Color", 4);
    SavePositionToVar(blueButton_);
    SubscribeToEvent(blueButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Синяя кнопка.
    indigoButton_ = UI_ROOT->CreateChild<MyButton>();
    indigoButton_->SetStyle("ColorButton");
    indigoButton_->SetNormalColor(Color(0.0f, 0.0f, 0.75f));
    indigoButton_->SetHoverColor(Color(0.0f, 0.0f, 1.0f));
    indigoButton_->SetPressedColor(Color(0.0f, 0.0f, 1.0f));
    indigoButton_->SetPosition(GetColorButtonTargetPos(5));
    indigoButton_->SetVar("Color", 5);
    SavePositionToVar(indigoButton_);
    SubscribeToEvent(indigoButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Фиолетовая кнопка.
    violetButton_ = UI_ROOT->CreateChild<MyButton>();
    violetButton_->SetStyle("ColorButton");
    violetButton_->SetNormalColor(Color(0.75f, 0.0f, 0.75f));
    violetButton_->SetHoverColor(Color(1.0f, 0.0f, 1.0f));
    violetButton_->SetPressedColor(Color(1.0f, 0.0f, 1.0f));
    violetButton_->SetPosition(GetColorButtonTargetPos(6));
    violetButton_->SetVar("Color", 6);
    SavePositionToVar(violetButton_);
    SubscribeToEvent(violetButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleColorButtonClick));

    // Дискета, информирующая о сохранении.
    floppyImage_ = UI_ROOT->CreateChild<BorderImage>();
    floppyImage_->SetStyle("FloppyImage");
    // Изначально дискета не видна.
    floppyImage_->SetColor(Color(1.0f, 1.0f, 1.0f, 0.0f));

    // Остаток ходов.
    turnsText_ = UI_ROOT->CreateChild<Text>();
    turnsText_->SetStyle("TurnsText");

    // Кнопка для уменьшения число ходов.
    decreaseTurnsButton_ = UI_ROOT->CreateChild<MyButton>();
    decreaseTurnsButton_->SetStyle("PrevButton");
    decreaseTurnsButton_->SetPosition(5, 100);
    SubscribeToEvent(decreaseTurnsButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleDecreaseTurnsButtonClick));

    // Кнопка для увеличения число ходов.
    increaseTurnsButton_ = UI_ROOT->CreateChild<MyButton>();
    increaseTurnsButton_->SetStyle("NextButton");
    increaseTurnsButton_->SetPosition(80, 100);
    SubscribeToEvent(increaseTurnsButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleIncreaseTurnsButtonClick));

    // Кнопка для перехода на предыдущий уровень.
    prevButton_ = UI_ROOT->CreateChild<MyButton>();
    prevButton_->SetStyle("PrevButton");
    prevButton_->SetAlignment(HA_CENTER, VA_CENTER);
    prevButton_->SetPosition(-410, 310);
    SubscribeToEvent(prevButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandlePrevButtonClick));

    // Кнопка для перезапуска уровня.
    replayButton_ = UI_ROOT->CreateChild<MyButton>();
    replayButton_->SetStyle("ReplayButton");
    replayButton_->SetAlignment(HA_CENTER, VA_CENTER);
    replayButton_->SetPosition(-330, 310);
    SubscribeToEvent(replayButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleReplayButtonClick));

    // Кнопка для перехода на следующий уровень.
    nextButton_ = UI_ROOT->CreateChild<MyButton>();
    nextButton_->SetStyle("NextButton");
    nextButton_->SetAlignment(HA_CENTER, VA_CENTER);
    nextButton_->SetPosition(NEXT_BUTTON_NORMAL_POS);
    SavePositionToVar(nextButton_);
    SubscribeToEvent(nextButton_, E_PRESSED, URHO3D_HANDLER(UIManager, HandleNextButtonClick));

    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(UIManager, HandlePostUpdate));
}

void UIManager::HandleNextButtonClick(StringHash eventType, VariantMap& eventData)
{
    // На последнем уровне кнопка перехода следующий уровень невидима, и игрок не может
    // по ней кликнуть. Однако во время загрузки сцены есть небольшой фриз. Если игрок
    // быстро щелкает по кнопке, он может нажать по ней два раза за одну итерацию игрового
    // цикла. Если это произойдет на предпоследнем уровне, то он выйдет за пределы списка.
    // Поэтому необходима данная проверка.
    if (!IsAvailableNextLevel())
        return;

    GLOBAL->neededLevelIndex_ = GLOBAL->currentLevelIndex_ + 1;

    if (GLOBAL->gameState_ == GS_WIN)
        GLOBAL->neededGameState_ = GS_PLAY;

    PlayClick();
}

void UIManager::HandlePrevButtonClick(StringHash eventType, VariantMap& eventData)
{
    if (!IsAvailablePrevLevel())
        return;

    GLOBAL->neededLevelIndex_ = GLOBAL->currentLevelIndex_ - 1;

    if (GLOBAL->gameState_ == GS_WIN)
        GLOBAL->neededGameState_ = GS_PLAY;

    PlayClick();
}

void UIManager::HandleReplayButtonClick(StringHash eventType, VariantMap& eventData)
{
    // Сцена будет загружена заново.
    GLOBAL->currentLevelIndex_ = -1;

    if (GLOBAL->gameState_ == GS_WIN)
        GLOBAL->neededGameState_ = GS_PLAY;

    PlayClick();
}

void UIManager::HandleIncreaseTurnsButtonClick(StringHash eventType, VariantMap& eventData)
{
    CONTAINER_LOGIC->turnsRemain_++;

    if (CONTAINER_LOGIC->turnsRemain_ < 1)
        CONTAINER_LOGIC->turnsRemain_ = 1;

    PlayClick();
}

void UIManager::HandleDecreaseTurnsButtonClick(StringHash eventType, VariantMap& eventData)
{
    CONTAINER_LOGIC->turnsRemain_--;

    if (CONTAINER_LOGIC->turnsRemain_ < 1)
        CONTAINER_LOGIC->turnsRemain_ = 1;

    PlayClick();
}

IntVector2 UIManager::GetColorButtonTargetPos(int color)
{
    int x = -20;

    // Кнопка с выбранным цветом смещена влево.
    if (color == selectedColor_)
        x = -40;

    // Всего 7 цветов. Кнопка с третьим цветом (нумерация с нуля) посередине (y == 0).
    // 64 - размер кнопки, 20 - промежуток между кнопками.
    int y = (color - 3) * (64 + 20);

    return IntVector2(x, y);
}

void UIManager::SavePositionToVar(UIElement* element)
{
    IntVector2 intPos = element->GetPosition();
    Vector2 floatPos((float)intPos.x_, (float)intPos.y_);
    element->SetVar("FloatPos", floatPos);
}

void UIManager::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[PostUpdate::P_TIMESTEP].GetFloat();

    UpdateElementsVisibility();

    UpdateColorButtonPos(redButton_, timeStep);
    UpdateColorButtonPos(orangeButton_, timeStep);
    UpdateColorButtonPos(yellowButton_, timeStep);
    UpdateColorButtonPos(greenButton_, timeStep);
    UpdateColorButtonPos(blueButton_, timeStep);
    UpdateColorButtonPos(indigoButton_, timeStep);
    UpdateColorButtonPos(violetButton_, timeStep);
    
    UpdateNextLevelButtonPos(timeStep);

    // Если дискета видима, то плавно уменьшаем ее видимость.
    float alpha = floppyImage_->GetColor(C_TOPLEFT).a_;
    if (alpha > 0.0f)
    {
        alpha = ToTarget(alpha, 0.0f, 1.0f, timeStep);
        floppyImage_->SetColor(Color(1.0f, 1.0f, 1.0f, alpha));
    }

    turnsText_->SetText(String(CONTAINER_LOGIC->turnsRemain_));

    if (INPUT->GetKeyPress(KEY_F2))
        DEBUG_HUD->ToggleAll();
}

void UIManager::UpdateNextLevelButtonPos(float timeStep)
{
    IntVector2 targetIntPos = NEXT_BUTTON_NORMAL_POS;

    // Если игрок победил, то кнопка перехода на следующий уровень должна
    // находиться в центре экрана. При прохождении уровня данная кнопка
    // невидима только в одном случае: игрок прошел игру, и уровней больше нет.
    if (GLOBAL->gameState_ == GS_WIN && IsAvailableNextLevel())
        targetIntPos = IntVector2(0, 0);

    Vector2 currentFloatPos = nextButton_->GetVar("FloatPos").GetVector2();
    Vector2 targetFloatPos((float)targetIntPos.x_, (float)targetIntPos.y_);
    Vector2 newFloatPos = ToTarget(currentFloatPos, targetFloatPos, 1000.0f, timeStep);
    nextButton_->SetVar("FloatPos", newFloatPos);
    nextButton_->SetPosition((int)newFloatPos.x_, (int)newFloatPos.y_);
}

bool UIManager::IsAvailableNextLevel()
{
    int effectiveLevelIndex = GLOBAL->currentLevelIndex_;

    // GLOBAL->currentLevelIndex_ == -1 только в случае перезапуска текущего уровня.
    // При этом реальный номер уровня сохранен в GLOBAL->neededLevelIndex_.
    if (effectiveLevelIndex == -1)
        effectiveLevelIndex = GLOBAL->neededLevelIndex_;

    // Больше уровней нет.
    if (effectiveLevelIndex + 1 >= CONFIG->GetNumLevels())
        return false;

    // В режиме редактора можно свободно переходить между уровнями.
    if (GLOBAL->gameState_ == GS_EDITOR)
        return true;

    // Текущий уровень еще не пройден.
    if (CONFIG->numCompletedLevels_ <= effectiveLevelIndex)
        return false;

    return true;
}

bool UIManager::IsAvailablePrevLevel()
{
    int effectiveLevelIndex = GLOBAL->currentLevelIndex_;

    if (effectiveLevelIndex == -1)
        effectiveLevelIndex = GLOBAL->neededLevelIndex_;

    if (effectiveLevelIndex == 0)
        return false;

    return true;
}

void UIManager::UpdateColorButtonPos(MyButton* button, float timeStep)
{
    int color = button->GetVar("Color").GetInt();
    IntVector2 targetIntPos = GetColorButtonTargetPos(color);

    // Кнопка уже в нужном месте.
    if (button->GetPosition() == targetIntPos)
        return;

    Vector2 currentFloatPos = button->GetVar("FloatPos").GetVector2();
    Vector2 targetFloatPos((float)targetIntPos.x_, (float)targetIntPos.y_);
    Vector2 newFloatPos = ToTarget(currentFloatPos, targetFloatPos, 200.0f, timeStep);
    button->SetVar("FloatPos", newFloatPos);
    button->SetPosition((int)newFloatPos.x_, (int)newFloatPos.y_);
}

void UIManager::HandleColorButtonClick(StringHash eventType, VariantMap& eventData)
{
    // Нельзя изменить цвет, если в данный момент производится заливка.
    if (CONTAINER_LOGIC->FillingIsDoing())
    {
        PlayFail();
        return;
    }

    MyButton* button = static_cast<MyButton*>(eventData["Element"].GetPtr());
    selectedColor_ = button->GetVar("Color").GetInt();

    PlayClick();
}

void UIManager::UpdateElementsVisibility()
{
    GameState gameState = GLOBAL->gameState_;

    if (gameState == GS_GAME_OVER)
    {
        redButton_->SetVisible(false);
        orangeButton_->SetVisible(false);
        yellowButton_->SetVisible(false);
        greenButton_->SetVisible(false);
        blueButton_->SetVisible(false);
        indigoButton_->SetVisible(false);
        violetButton_->SetVisible(false);
    }
    else
    {
        redButton_->SetVisible(true);
        orangeButton_->SetVisible(true);
        yellowButton_->SetVisible(true);
        greenButton_->SetVisible(true);
        blueButton_->SetVisible(true);
        indigoButton_->SetVisible(true);
        violetButton_->SetVisible(true);
    }

    if (gameState == GS_PLAY || gameState == GS_EDITOR)
        turnsText_->SetVisible(true);
    else
        turnsText_->SetVisible(false);

    if (gameState == GS_EDITOR && CONTAINER_LOGIC->turnsRemain_ > 1)
        decreaseTurnsButton_->SetVisible(true);
    else
        decreaseTurnsButton_->SetVisible(false);

    if (gameState == GS_EDITOR)
        increaseTurnsButton_->SetVisible(true);
    else
        increaseTurnsButton_->SetVisible(false);

    if (IsAvailablePrevLevel() && gameState != GS_GAME_OVER)
        prevButton_->SetVisible(true);
    else
        prevButton_->SetVisible(false);

    if (gameState != GS_GAME_OVER)
        replayButton_->SetVisible(true);
    else
        replayButton_->SetVisible(false);

    if (IsAvailableNextLevel() && gameState != GS_GAME_OVER)
        nextButton_->SetVisible(true);
    else
        nextButton_->SetVisible(false);
}

UIElement* UIManager::GetHoveredElement()
{
    return UI->GetElementAt(INPUT->GetMousePosition());
}

void UIManager::PlayClick()
{
    GLOBAL->PlaySound("Click", "Sounds/Click", 3);
}

void UIManager::PlayFail()
{
    GLOBAL->PlaySound("Sounds/Fail.wav");
}
