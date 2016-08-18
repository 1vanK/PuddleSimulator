/*
Ёмкость для молекул.
*/

#pragma once
#include "Global.h"

// Число ходов при создании нового уровня.
#define DEFAULT_TURNS_REMAIN 5
// Радиус молекулы.
#define MOLECULE_RADIUS 0.5f
// Быстрый доступ к ёмкости. Гарантируется, что ёмкость всегда доступна после инициализации игры.
#define CONTAINER_LOGIC GLOBAL->scene_->GetChild("Container")->GetComponent<ContainerLogic>()

class ContainerLogic : public Component
{
    URHO3D_OBJECT(ContainerLogic, Component);

public:
    // Осталось ходов.
    int turnsRemain_ = DEFAULT_TURNS_REMAIN;

    ContainerLogic(Context* context);
    static void RegisterObject(Context* context);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    // Меняет цвет определенной молекулы. Цвет молекулы задается цифрами от 0 до 6.
    void SetMoleculeColor(Node* molecule, int color);
    // Заливка текущим цветом, начиная с определенной молекулы. Заливка на самом
    // деле происходит в UpdateFilling, а в данной функции подготавливается список
    // молекул, которые должны изменить цвет.
    void Fill(Node* startMolecule);
    // В данный момент производится заливка. Пользовательский ввод заблокирован.
    bool FillingIsDoing() const { return filledMolecules_.Size() > 0; }

private:
    // Список молекул, которые должны поменять свой цвет в процессе заливки.
    // Если этот список не пустой, значит в данный момент происходит заливка,
    // и игрок не может кликать по молекулам и менять выбранный цвет.
    PODVector<Node*> filledMolecules_;
    // Задержка перед изменением цвета следующей молекулы в процессе заливки.
    float fillingDelay_ = 0.0f;

    // Радиус ёмкости. Модель дна емкости (белый круг) имеет радиус 1.
    // Значит реальный радиус ёмкости равен масштабу дна.
    float GetRadius() const { return GLOBAL->scene_->GetChild("ContainerBottom")->GetScale().x_; }
    // Нода не должна быть равна nullptr, проверка не производится.
    int GetMoleculeColor(Node* molecule) const { return molecule->GetVar("Color").GetInt(); }
    // Анимация заливки.
    void UpdateFilling(float timeStep);
    // Обновление позиций молекул (весь расчет физики тут).
    void UpdateMolecules(float timeStep);
    // Проверяет, что все молекулы в ёмкости одинакового цвета (то есть уровень пройден).
    bool IsSingleColor();
};
