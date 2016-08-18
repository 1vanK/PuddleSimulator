:: Указываем путь к git.exe.
set "PATH=c:\Program Files (x86)\Git\bin\"
:: Скачиваем репозиторий.
git clone https://github.com/Urho3D/Urho3D.git
:: Переходим в папку со скачанными исходниками.
cd Urho3D
:: Возвращаем состояние репозитория к определённой версии (17 августа 2016).
git reset --hard 98a3c8d49aa8a6eb1404a3aec480b1517c913c27
:: Ждём нажатия ENTER для закрытия консоли.
pause
