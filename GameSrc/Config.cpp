#include "Config.h"
#include "Urho3DAliases.h"

Config::Config(Context* context) : Object(context)
{
    Load();
}

String Config::GetConfigFileName()
{
    return FILE_SYSTEM->GetAppPreferencesDir("1vanK", "PuddleSimulator") + "Config.xml";
}

void Config::Load()
{
    // Загружаем список уровней.
    SharedPtr<File> file = CACHE->GetFile("Levels.txt");
    while (!file->IsEof())
        levelList_.Push(file->ReadLine());

    // Загружаем конфиг.
    String fileName = GetConfigFileName();

    if (FILE_SYSTEM->FileExists(fileName))
    {
        File file(context_, fileName, FILE_READ);
        XMLFile xmlFile(context_);
        xmlFile.Load(file);
        XMLElement root = xmlFile.GetRoot();
        numCompletedLevels_ = root.GetInt("NumCompletedLevels");
        numCompletedLevels_ = Clamp(numCompletedLevels_, 0, GetNumLevels());
    }
}

void Config::Save()
{
    XMLFile xmlFile(context_);
    XMLElement root = xmlFile.CreateRoot("Config");
    root.SetInt("NumCompletedLevels", numCompletedLevels_);
    File file(context_, GetConfigFileName(), FILE_WRITE);
    xmlFile.Save(file);
}
