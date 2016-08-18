#include "ContainerLogic.h"
#include "Urho3DAliases.h"
#include "UIManager.h"

// Расстояние, на котором молекулы перестают взаимодействовать.
// Чем больше это расстояние, тем активнее одноцветные молекулы собираются в круглые
// лужи, так как каждая молекула подвергаются влиянию бОльшего числа молекул.
// Однако, если переборщить, то одноцветные молекулы будут притягиваться с другого края ёмкости.
// (На самом деле не притягиваться, а выдавливаться молекулами другого цвета, то есть молекулы
// движутся из-за разницы давлений со стороны окружающих молекул).
// Поэтому выбираем расстояние, при котором 3 одноцветных молекулы, выстроенные в линию и
// слегка вдавленные друг в друга, стянутся в треугольник (то есть две крайние молекулы
// ряда взаимодействуют).
static const float INTERACTION_RANGE = MOLECULE_RADIUS * 4.0f;
// Задержка перед изменением цвета очередных пяти молекул при заливке.
static const float FILLING_DELAY = 0.02f;

// Имена файлов материалов молекул.
const char* colorFiles[] = {
    "Materials/Red.xml",
    "Materials/Orange.xml",
    "Materials/Yellow.xml",
    "Materials/Green.xml",
    "Materials/Blue.xml",
    "Materials/Indigo.xml",
    "Materials/Violet.xml"
};

ContainerLogic::ContainerLogic(Context* context) :
    Component(context)
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ContainerLogic, HandleUpdate));
}

void ContainerLogic::RegisterObject(Context* context)
{
    context->RegisterFactory<ContainerLogic>();

    // Остаток ходов сохраняется в файл сцены.
    URHO3D_ATTRIBUTE("Turns", int, turnsRemain_, DEFAULT_TURNS_REMAIN, AM_FILE);
}

void ContainerLogic::SetMoleculeColor(Node* molecule, int color)
{
    // Номер цвета сохраняется в переменную ноды.
    molecule->SetVar("Color", color);

    // Меняем материал модели.
    StaticModel* staticModel = molecule->GetComponent<StaticModel>();
    staticModel->SetMaterial(GET_MATERIAL(colorFiles[color]));
}

void ContainerLogic::UpdateFilling(float timeStep)
{
    if (fillingDelay_ > 0.0f)
        fillingDelay_ -= timeStep;

    if (fillingDelay_ <= 0.0f)
    {
        // Меняем цвет пяти молекул за раз.
        for (int i = 0; i < 5 && filledMolecules_.Size() > 0; i++)
        {
            // Забираем первую молекулу из списка.
            Node* molecule = filledMolecules_.Front();
            filledMolecules_.Erase(0, 1);

            // Меняем ее цвет.
            SetMoleculeColor(molecule, UI_MANAGER->selectedColor_);
        }

        // Снова устанавливаем задержку.
        fillingDelay_ = FILLING_DELAY;
    }
}

void ContainerLogic::Fill(Node* startMolecule)
{
    // Цвет стартовой молекулы.
    int startColor = GetMoleculeColor(startMolecule);

    // Стартовая молекула уже нужного цвета.
    if (startColor == UI_MANAGER->selectedColor_)
        return;

    GLOBAL->PlaySound("Fill", "Sounds/Fill", 3);

    turnsRemain_--;

    auto molecules = node_->GetChildren();

    // Очищаем все молекулы от служебного тега.
    for (Node* molecule : molecules)
        molecule->RemoveTag("Checked");

    // Список молекул, для которых нужно проверить их соседей (ищутся соседи
    // с таким же цветом). Но сами эти молекулы уже проверены.
    PODVector<Node*> needChekNeighbors;

    // Стартовая молекула уже проверена.
    startMolecule->AddTag("Checked");
    // Но нужно проверить ее соседей.
    needChekNeighbors.Push(startMolecule);
    // А также ее цвет будет изменен.
    filledMolecules_.Push(startMolecule);

    // Выполняем, пока в списке есть молекулы.
    while (needChekNeighbors.Size() > 0)
    {
        // Забираем первую молекулу из списка. Порядок важен, чтобы
        // заливка при анимации распространялась от стартовой молекулы.
        Node* molecule = needChekNeighbors.Front();
        needChekNeighbors.Erase(0, 1);

        // Находим всех соседей рассматриваемой моекулы.
        for (Node* anotherMolecule : molecules)
        {
            // Уже проверенные молекулы пропускаем.
            if (anotherMolecule->HasTag("Checked"))
                continue;

            // Если дистанция между молекулами слишком большая, то они не соседние.
            float distance = (molecule->GetPosition() - anotherMolecule->GetPosition()).Length();
            if (distance > MOLECULE_RADIUS * 2.2f)
                continue;

            // Если у соседа цвет стартовой молекулы, то заливка распространяется дальше.
            // Меняем цвет молекулы и помещаем ее в список для проверки ее соседей.
            if (GetMoleculeColor(anotherMolecule) == startColor)
            {
                anotherMolecule->AddTag("Checked");
                needChekNeighbors.Push(anotherMolecule);
                filledMolecules_.Push(anotherMolecule);
            }
        }
    }

    // Подготавливаемся к вызову UpdateFilling(). Первые 5 молекул
    // изменят цвет без задержки.
    fillingDelay_ = 0.0f;
}

bool ContainerLogic::IsSingleColor()
{
    auto molecules = node_->GetChildren();

    // Ёмкость пуста.
    if (molecules.Size() == 0)
        return true;

    // Цвет первой молекулы.
    int firstColor = GetMoleculeColor(molecules[0]);

    for (int i = 1; i < (int)molecules.Size(); i++)
    {
        // Найдена молекула с другим цветом.
        if (GetMoleculeColor(molecules[i]) != firstColor)
            return false;
    }

    return true;
}

void ContainerLogic::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    UpdateMolecules(timeStep);

    if (GLOBAL->gameState_ != GS_PLAY)
        return;

    // Если в данный момент производится заливка, то анимируем ее.
    if (FillingIsDoing())
    {
        UpdateFilling(timeStep);
        return;
    }

    // Игрок выигрывает, если все молекулы одинакового цвета.
    if (IsSingleColor())
        GLOBAL->neededGameState_ = GS_WIN;
    // Игрок проигрывает, если есть молекулы разного цвета и больше нет ходов.
    else if (turnsRemain_ == 0)
        GLOBAL->neededGameState_ = GS_GAME_OVER;
}

void ContainerLogic::UpdateMolecules(float timeStep)
{
    auto molecules = node_->GetChildren();

    // Обнуляем силы, действующие на молекулы.
    for (unsigned i = 0; i < molecules.Size(); i++)
        molecules[i]->SetVar("Force", Vector3::ZERO);

    // Вычисляем воздействия молекул друг на друга.
    for (Node* molecule : molecules)
    {
        for (Node* anotherMolecule : molecules)
        {
            // Сама с собой молекула не взаимодействует.
            if (molecule == anotherMolecule)
                continue;

            Vector3 moleculePos = molecule->GetPosition();
            Vector3 anotherMoleculePos = anotherMolecule->GetPosition();

            // Расстояние между молекулами.
            float distance = (moleculePos - anotherMoleculePos).Length();

            if (Equals(distance, 0.0f))
            {
                // Случилось страшное - молекулы находятся в одной точке.
                // Задаем им противоположные скорости.
                // Вероятность, что скорости по обеим осям будут нулевыми, крайне мала.
                // В любом случае, в следующий раз снова будет произведена попытка
                // разлепить молекулы.
                Vector3 speed = Vector3(Random(-2.0f, 2.0f), Random(-2.0f, 2.0f), 0.0f);
                molecule->SetVar("Speed", speed);
                anotherMolecule->SetVar("Speed", -speed);
                continue;
            }

            // Молекулы слишком далеко и не взаимодействуют.
            if (distance >= INTERACTION_RANGE)
                continue;

            // Направление от текущей молекулы до другой.
            Vector3 direction = (anotherMoleculePos - moleculePos).Normalized();

            // Сила отталкивая возрастает при уменьшении дистанции.
            // Результат операции в диапазоне (0, 1).
            float forceModulus = 1.0f - distance / INTERACTION_RANGE;

            // Результат следующей операции также в диапазоне (0, 1), но график "прилипает" к нулю.
            // То есть на больших расстояниях отталкивание мало, но при сближении
            // молекул отталкивание резко (нелинейно) возрастает.
            // Вдавленные друг в друга молекулы будут отталкиваться очень сильно.
            forceModulus = forceModulus * forceModulus * forceModulus;

            // Закон отталкивания мы получили, теперь усиливаем его.
            forceModulus = forceModulus * 25.0f;
            
            // Если молекулы разного типа, то отталкивание сильнее, они хотят держаться друг от друга дальше.
            if (GetMoleculeColor(molecule) != GetMoleculeColor(anotherMolecule))
                forceModulus *= 2.0f;

            Vector3 moleculeForce = molecule->GetVar("Force").GetVector3();
            Vector3 newMoleculeForce = moleculeForce - forceModulus * direction;
            molecule->SetVar("Force", newMoleculeForce);

            Vector3 anotherMoleculeForce = anotherMolecule->GetVar("Force").GetVector3();
            Vector3 newAnotherMoleculeForce = anotherMoleculeForce + forceModulus * direction;
            anotherMolecule->SetVar("Force", newAnotherMoleculeForce);
        }
    }

    float containerRadius = GetRadius();

    // Если молекулы вылетают за пределы сосуда, то сильно толкаем их назад.
    for (Node* molecule : molecules)
    {
        Vector3 pos = molecule->GetPosition();

        if (pos.Length() > containerRadius - MOLECULE_RADIUS)
        {
            // Направление к центру ёмкости.
            Vector3 backDirection = -pos.Normalized();

            float backModulus = (pos.Length() - containerRadius + MOLECULE_RADIUS) * 200.0f;
            Vector3 force = molecule->GetVar("Force").GetVector3();
            Vector3 newForce = force + backDirection * backModulus;
            molecule->SetVar("Force", newForce);
        }
    }

    // Модифицируем скорости всех молекул, с учетом действующих на них сил.
    for (Node* molecule : molecules)
    {
        Vector3 force = molecule->GetVar("Force").GetVector3();
        Vector3 speed = molecule->GetVar("Speed").GetVector3();
        Vector3 newSpeed = speed + force * timeStep;
        molecule->SetVar("Speed", newSpeed);
    }

    // Вязкость (внутреннее трение жидкости). Замедляем молекулы со временем.
    for (Node* molecule : molecules)
    {
        Vector3 speed = molecule->GetVar("Speed").GetVector3();

        // Чем больше значение скорости, тем быстрее она уменьшается.
        // Однако молекулы долго продолжают двигаться с маленькой скоростью.
        Vector3 newSpeed = speed.Lerp(Vector3::ZERO, timeStep * 0.5f);

        molecule->SetVar("Speed", newSpeed);
    }

    // Наконец, применяем расчитанные скорости.
    for (Node* molecule : molecules)
    {
        Vector3 speed = molecule->GetVar("Speed").GetVector3();
        molecule->Translate(speed * timeStep);
    }
}
