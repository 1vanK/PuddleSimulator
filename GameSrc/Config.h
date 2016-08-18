/*
Подсистема для загрузки и сохранения настроек.
*/

#pragma once
#include <Urho3D/Urho3DAll.h>

#define CONFIG GetSubsystem<Config>()

class Config : public Object
{
    URHO3D_OBJECT(Config, Object);

public:
    // Количество пройденных уровней.
    int numCompletedLevels_ = 0;

    Config(Context* context);
    
    // Сохранение конфига в файл.
    void Save();
    // Общее число уровней в файле GameData/Levels.txt.
    int GetNumLevels() const { return levelList_.Size(); }
    // Возвращает нужную строку из файла GameData/Levels.txt.
    String GetLevelFileName(int index) const { return levelList_[index]; }

private:
    // Список строк из GameData/Levels.txt.
    Vector<String> levelList_;

    // Путь для сохранения конфига.
    String GetConfigFileName();
    // Загружает конфиг и список строк из GameData/Levels.txt.
    void Load();
};
