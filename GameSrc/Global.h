#pragma once
#include <Urho3D/Urho3DAll.h>

#define GLOBAL GetSubsystem<Global>()

enum GameState
{
    // В процессе игры.
    GS_PLAY,
    // Игра выиграна.
    GS_WIN,
    // Игра проиграна.
    GS_GAME_OVER,
    // Редактирование уровня.
    GS_EDITOR
};

class Global : public Object
{
    URHO3D_OBJECT(Global, Object);

public:
    // Игра использует только одну сцену.
    SharedPtr<Scene> scene_;
    // Текущее игровое состояние.
    GameState gameState_ = GS_PLAY;
    // Игровое состояние, которое будет установлено в начале следующей итерации игрового цикла.
    GameState neededGameState_ = GS_PLAY;
    // Номер текущего уровня.
    int currentLevelIndex_ = 0;
    // Номер уровня, который будет загружен в начале следующей итерации игрового цикла.
    int neededLevelIndex_ = 0;

    Global(Context* context);

    // Одинаковые звуки не будут воспроизводиться одновременно.
    void PlaySound(const String& fileName);
    // Однотипные звуки не будут воспроизводиться одновременно.
    // Будут проигрываться файлы с именами fileNameBegin + Random(num_variations) + ".wav".
    // Один и тот же файл не будет проигрываться два раза подряд.
    void PlaySound(const String& type, const String& fileNameBegin, int num_variations);
    // Запускает зацикленное воспроизведение фоновой музыки.
    void PlayMusic(const String& fileName);

private:
    // Корневая нода для всех источников звука. Не принадлежит ни одной сцене.
    SharedPtr<Node> soundRoot_;
    // Нода для музыкального проигрывателя. Не принадлежит ни одной сцене.
    SharedPtr<Node> musicNode_;
};
