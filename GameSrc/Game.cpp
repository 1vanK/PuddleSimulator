#include "Global.h"
#include "ContainerLogic.h"
#include "UIManager.h"
#include "MyButton.h"
#include "Urho3DAliases.h"
#include "Utils.h"
#include "Config.h"

// Имена файлов материалов молекул определены в ContainerLogic.cpp.
extern const char* colorFiles[];
// Радиус сосуда при создании нового уровня.
#define DEFAULT_CONTAINER_RADIUS 5.0f
// Обычный цвет фона.
static const Color FOG_COLOR(0.4f, 0.5f, 0.8f);
// Цвет фона в режиме редактора.
static const Color FOG_COLOR_EDITOR(0.8f, 0.5f, 0.4f);
// Задержка в секундах перед растартом уровня при проигрыше.
#define GAME_OVER_DELAY 1.0f

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    Game(Context* context) : Application(context)
    {
        // Самый первый обработчик для самого первого события. Используется для смены состояния
        // игры в начале итерации игрового цикла перед возникновением других событий.
        SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(Game, ApplyGameState));

        // Регистрируем собственные компоненты.
        ContainerLogic::RegisterObject(context);
        MyButton::RegisterObject(context_);
    }

    void Setup()
    {
        engineParameters_["WindowTitle"] = "Puddle Simulator";
        engineParameters_["LogName"] = FILE_SYSTEM->GetAppPreferencesDir("1vanK", "PuddleSimulator") + "Log.log";
        engineParameters_["FullScreen"] = false;
        engineParameters_["WindowWidth"] = 920;
        engineParameters_["WindowHeight"] = 720;
        // Собственная папка с ресурсами указывается перед стандартными.
        // Таким образом можно подсунуть движку свои ресурсы вместо стандартных.
        engineParameters_["ResourcePaths"] = "GameData;Data;CoreData";
    }

    // Полный путь к файлу сцены.
    String GetFullLevelPath(int index)
    {
        return FILE_SYSTEM->GetProgramDir() + "GameData/Scenes/" + CONFIG->GetLevelFileName(index);
    }

    // Меняет цвет фона и показывает/прячет дно ёмкости в зависимости
    // от состояния игры.
    void UpdateFogColorAndContainerBottomVisible()
    {
        // Дно ёмкости видно только в режиме редактирования.
        Node* bottomNode = GLOBAL->scene_->GetChild("ContainerBottom");
        bottomNode->SetEnabled(GLOBAL->gameState_ == GS_EDITOR);

        Node* zoneNode = GLOBAL->scene_->GetChild("Zone");
        Zone* zone = zoneNode->GetComponent<Zone>();

        // В режиме редактирования туман красного цвета.
        if (GLOBAL->gameState_ == GS_EDITOR)
            zone->SetFogColor(FOG_COLOR_EDITOR);
        else
            zone->SetFogColor(FOG_COLOR);
    }

    // Показывает сцену на экране.
    void SetupViewport()
    {
        Node* cameraNode = GLOBAL->scene_->GetChild("Camera");
        Camera* camera = cameraNode->GetComponent<Camera>();
        SharedPtr<Viewport> viewport(new Viewport(context_, GLOBAL->scene_, camera));
        RENDERER->SetViewport(0, viewport);
    }

    // Загружает и запускает уровень.
    void StartLevel(int levelIndex)
    {
        levelIndex = Clamp(levelIndex, 0, CONFIG->GetNumLevels() - 1);
        GLOBAL->currentLevelIndex_ = GLOBAL->neededLevelIndex_ = levelIndex;
        SharedPtr<File> file = CACHE->GetFile(GetFullLevelPath(levelIndex));
        
        // Если не удалось загрузить сцену, то создаем пустой уровень и переходим
        // в режим редактирования.
        if (!file || !GLOBAL->scene_->LoadXML(*file))
        {
            CreateScene();
            GLOBAL->gameState_ = GLOBAL->neededGameState_ = GS_EDITOR;
        }

        UpdateFogColorAndContainerBottomVisible();
        SetupViewport();
    }

    void Start()
    {
        // Каждая игра будет уникальной.
        SetRandomSeed(Time::GetSystemTime());
        // Блокируем Alt+Enter.
        INPUT->SetToggleFullscreen(false);
        // Ограничиваем ФПС, чтобы снизить нагрузку на систему.
        ENGINE->SetMaxFps(60);

        // DefaultRenderPath используется при создании вьюпортов.
        // Изначально DefaultRenderPath соответствует Forward.xml.
        RenderPath* defaultRenderPath = RENDERER->GetDefaultRenderPath();
        // Добавляем виньетку.
        defaultRenderPath->Append(GET_XML_FILE("PostProcess/Vignette.xml"));
        // Добавляем затенение. Изначально сила затенения равна 0.
        defaultRenderPath->Append(GET_XML_FILE("PostProcess/Fade.xml"));

        // Создаем собственные подсистемы после инициализации встроенных,
        // так как они могут обращаться к встроенным в своих конструкторах.
        context_->RegisterSubsystem(new Config(context_));
        context_->RegisterSubsystem(new Global(context_));
        context_->RegisterSubsystem(new UIManager(context_));

        // Используется одна сцена на всю игру, но она может полностью очищаться
        // и загружаться из файла.
        GLOBAL->scene_ = new Scene(context_);

        // Загружаем последний непройденный уровень. Список уровней не может быть пустым.
        StartLevel(CONFIG->numCompletedLevels_);

        // Запускаем зацикленное проигрывание фоновой музыки.
        GLOBAL->PlayMusic("Music/Music.ogg");
        // Фоновая музыка тише звуков.
        AUDIO->SetMasterGain(SOUND_MUSIC, 0.5f);

        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
    }

    // Управляет силой затенения в зависимости от игрового состояния.
    void UpdateFade(float timeStep)
    {
        float targetFade = 0.0f;

        if (GLOBAL->gameState_ == GS_GAME_OVER)
            targetFade = 1.0f;

        RenderPath* renderPath = RENDERER->GetViewport(0)->GetRenderPath();
        float currentFade = renderPath->GetShaderParameter("FadeValue").GetFloat();

        if (currentFade != targetFade)
        {
            float newFade = ToTarget(currentFade, targetFade, 1.0f / GAME_OVER_DELAY, timeStep);
            renderPath->SetShaderParameter("FadeValue", newFade);
        }
    }

    // Меняет текущее игровое состояние на требуемое. Также производит смену уровня.
    void ApplyGameState(StringHash eventType, VariantMap& eventData)
    {
        if (GLOBAL->gameState_ != GLOBAL->neededGameState_)
        {
            GLOBAL->gameState_ = GLOBAL->neededGameState_;
            UpdateFogColorAndContainerBottomVisible();

            if (GLOBAL->gameState_ == GS_WIN)
            {
                // Звук победы.
                GLOBAL->PlaySound("Sounds/Win.wav");

                // Обновляем число завершенных уровней.
                if (CONFIG->numCompletedLevels_ < GLOBAL->currentLevelIndex_ + 1)
                    CONFIG->numCompletedLevels_ = GLOBAL->currentLevelIndex_ + 1;
            }
            else if (GLOBAL->gameState_ == GS_GAME_OVER)
            {
                // Звук поражения.
                GLOBAL->PlaySound("Sounds/GameOver.wav");
            }
        }

        if (GLOBAL->currentLevelIndex_ != GLOBAL->neededLevelIndex_)
        {
            GLOBAL->currentLevelIndex_ = GLOBAL->neededLevelIndex_;
            StartLevel(GLOBAL->currentLevelIndex_);
        }
    }

    // Создает пустой уровень. Используется, если не удалось загрузить уровень из файла.
    void CreateScene()
    {
        Scene* scene = GLOBAL->scene_;
        scene->Clear();
        scene->CreateComponent<Octree>();

        Node* lightNode = scene->CreateChild("DirectionalLight");
        lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
        Light* light = lightNode->CreateComponent<Light>();
        light->SetColor(Color(0.6f, 0.6f, 0.4f));
        light->SetLightType(LIGHT_DIRECTIONAL);

        Node* zoneNode = scene->CreateChild("Zone");
        Zone* zone = zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
        zone->SetAmbientColor(Color(0.4f, 0.4f, 0.6f));
        zone->SetFogColor(FOG_COLOR);

        // Ёмкость для молекул.
        Node* containerNode = scene->CreateChild("Container");
        containerNode->CreateComponent<ContainerLogic>();
        
        // Дно ёмкости.
        Node* containerBottomNode = scene->CreateChild("ContainerBottom");
        containerBottomNode->SetPosition(Vector3(0.0f, 0.0f, 10.0f));
        containerBottomNode->SetScale(DEFAULT_CONTAINER_RADIUS);
        StaticModel* bottomObject = containerBottomNode->CreateComponent<StaticModel>();
        bottomObject->SetModel(GET_MODEL("Models/ContainerBottom.mdl"));
        bottomObject->ApplyMaterialList();

        Node* cameraNode = scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(0.0f, 0.0f, -20.0f));
        Camera* camera = cameraNode->CreateComponent<Camera>();
        camera->SetOrthographic(true);
    }

    // Создает молекулу.
    void CreateMolecule(const Vector3& pos, int color)
    {
        Node* node = GLOBAL->scene_->GetChild("Container")->CreateChild();
        node->SetPosition(pos);
        // Индекс цвета хранится в переменной ноды.
        node->SetVar("Color", color);

        StaticModel* object = node->CreateComponent<StaticModel>();
        object->SetModel(GET_MODEL("Models/Molecule.mdl"));
        object->SetMaterial(GET_MATERIAL(colorFiles[color]));
    }

    // Создает молекулу в случайном месте сосуда.
    void CreateMolecule(int color)
    {
        // Случайное расстояние от центра.
        float dist = Random(0.1f, DEFAULT_CONTAINER_RADIUS - MOLECULE_RADIUS);
        // Случайный угол.
        float angle = Random(0.0f, 360.0f);
        // Итоговая координата.
        Vector3 pos = Vector3(dist * sin(angle), dist * cos(angle), 0.0f);

        CreateMolecule(pos, color);
    }

    // Проецирует курсор на плоскость XOY.
    Vector3 CursorToPlaneXOY() const
    {
        float mouseX = (float)INPUT->GetMousePosition().x_ / GRAPHICS->GetWidth();
        float mouseY = (float)INPUT->GetMousePosition().y_ / GRAPHICS->GetHeight();
        Node* cameraNode = GLOBAL->scene_->GetChild("Camera");
        Camera* camera = cameraNode->GetComponent<Camera>();
        
        // Так как используется ортогональная камера и ее направление перпендикулярно
        // плоскости Z=0, то проецирование очень простое.
        float distance = -cameraNode->GetPosition().z_;
        Vector3 result = camera->ScreenToWorldPoint(Vector3(mouseX, mouseY, distance));
        
        // Проверяем значение Z.
        //if (result.z_ != 0.0f)
        //    DEBUG_HUD->SetAppStats("Z не равно 0, а равно", result.z_);
        
        // На всякий случай страхуемся от случая, когда Z будет отличаться от нуля
        // из-за недостаточной точности вычислений (хотя в тестах такого не было ни разу).
        result.z_ = 0.0f;
        return result;
    }

    // Молекула под курсором мыши.
    Node* RaycastToMolecule() const
    {
        float mouseX = (float)INPUT->GetMousePosition().x_ / GRAPHICS->GetWidth();
        float mouseY = (float)INPUT->GetMousePosition().y_ / GRAPHICS->GetHeight();
        Camera* camera = GLOBAL->scene_->GetChild("Camera")->GetComponent<Camera>();
        Ray cameraRay = camera->GetScreenRay(mouseX, mouseY);
        PODVector<RayQueryResult> results;
        RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
        GLOBAL->scene_->GetComponent<Octree>()->RaycastSingle(query);
        
        if (results.Size())
            return results[0].node_;
        
        return nullptr;
    }

    // Край контейнера притягивается к курсору мыши.
    void ResizeContainer()
    {
        IntVector2 mousePos = INPUT->GetMousePosition();
        float mouseX = (float)mousePos.x_ / GRAPHICS->GetWidth();
        float mouseY = (float)mousePos.y_ / GRAPHICS->GetHeight();
        Node* cameraNode = GLOBAL->scene_->GetChild("Camera");
        float depth = -cameraNode->GetPosition().z_;
        Camera* camera = cameraNode->GetComponent<Camera>();
        Vector3 worldPos = camera->ScreenToWorldPoint(Vector3(mouseX, mouseY, depth));
        float newRadius = worldPos.Length();
        // Модель дна контейнера (белый круг) имеет радиус 1. То есть новый
        // радиус соответствует масштабу.
        GLOBAL->scene_->GetChild("ContainerBottom")->SetScale(newRadius);
    }

    // Апдейт в состоянии GS_GAME_OVER.
    void UpdateGameOver(float timeStep)
    {
        // Есть только один экземпляр класса, поэтому смело используем
        // статическую переменную.
        static float gameOverTimer_ = 0.0f;

        gameOverTimer_ += timeStep;

        if (gameOverTimer_ > GAME_OVER_DELAY)
        {
            GLOBAL->neededGameState_ = GS_PLAY;
            GLOBAL->currentLevelIndex_ = -1;
            gameOverTimer_ = 0.0f;
        }
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

        UpdateFade(timeStep);

        if (GLOBAL->gameState_ == GS_GAME_OVER)
        {
            UpdateGameOver(timeStep);
            return;
        }

        // В режиме редактора меняем размер ёмкости при зажатом шифте.
        if (INPUT->GetKeyDown(KEY_SHIFT) && GLOBAL->gameState_ == GS_EDITOR)
            ResizeContainer();

        // В режиме редактора создаем молекулы при нажатии ЛКМ.
        if (INPUT->GetMouseButtonPress(MOUSEB_LEFT) && !UI_MANAGER->GetHoveredElement()
            && GLOBAL->gameState_ == GS_EDITOR)
        {
            Vector3 pos = CursorToPlaneXOY();
            CreateMolecule(pos, UI_MANAGER->selectedColor_);
        }

        // В режиме редактора создаем 5 молекул при нажатии пробела.
        if (INPUT->GetKeyPress(KEY_SPACE) && GLOBAL->gameState_ == GS_EDITOR)
        {
            for (int i = 0; i < 5; i++)
                CreateMolecule(UI_MANAGER->selectedColor_);
        }

        // В режиме редактора меняем цвет молекул правой кнопкой мыши.
        if (INPUT->GetMouseButtonDown(MOUSEB_RIGHT) && GLOBAL->gameState_ == GS_EDITOR)
        {
            Node* molecule = RaycastToMolecule();

            if (molecule)
                CONTAINER_LOGIC->SetMoleculeColor(molecule, UI_MANAGER->selectedColor_);
        }

        // В режиме редактора сохраняем сцену при нажатии S.
        if (INPUT->GetKeyPress(KEY_S) && GLOBAL->gameState_ == GS_EDITOR)
        {
            File file(context_, GetFullLevelPath(GLOBAL->currentLevelIndex_), FILE_WRITE);
            GLOBAL->scene_->SaveXML(file);
            // Делаем дискету видимой.
            UI_MANAGER->floppyImage_->SetColor(Color::WHITE);
        }

        // По нажатию клавиши E игра переходит в режим редактора и обратно.
        // В процессе заливки режим менять нельзя.
        if (INPUT->GetKeyPress(KEY_E) && !CONTAINER_LOGIC->FillingIsDoing())
        {
            if (GLOBAL->gameState_ == GS_EDITOR)
                GLOBAL->neededGameState_ = GS_PLAY;
            else
                GLOBAL->neededGameState_ = GS_EDITOR;
        }

        // В игровом режиме при нажатии ЛКМ происходит заливка.
        if (INPUT->GetMouseButtonPress(MOUSEB_LEFT) && !CONTAINER_LOGIC->FillingIsDoing()
            && GLOBAL->gameState_ == GS_PLAY)
        {
            Node* molecule = RaycastToMolecule();
            
            if (molecule)
                CONTAINER_LOGIC->Fill(molecule);
        }
    }

    void Stop()
    {
        // Сохраняем конфиг при выходе из игры.
        CONFIG->Save();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
