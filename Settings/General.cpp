#include "General.h"

#include <shellapi.h>

#include "../3RVX/Logger.h"
#include "../3RVX/Settings.h"
#include "../3RVX/SkinInfo.h"

#include "UIContext.h"
#include "resource.h"

#define KEY_NAME L"3RVX"
#define STARTUP_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

void General::Command(unsigned short nCode, unsigned short ctrlId) {
    switch (nCode) {
    case BN_CLICKED:
        if (ctrlId == BTN_WEBSITE && _url != L"") {
            ShellExecute(NULL, L"open", _url.c_str(),
                NULL, NULL, SW_SHOWNORMAL);
        }
        break;

    case CBN_SELCHANGE:
        switch (ctrlId) {
        case CMB_SKIN:
            LoadSkinInfo(_ctxt->GetComboSelection(CMB_SKIN));
            break;

        case CMB_LANG:
            // Language selection
            break;
        }
    }
}

void General::LoadSettings() {
    Settings *settings = Settings::Instance();
    _ctxt->SetCheck(CHK_STARTUP, RunOnStartup());
    _ctxt->SetCheck(CHK_NOTIFY, settings->NotifyIconEnabled());
    _ctxt->SetCheck(CHK_SOUNDS, settings->SoundEffectsEnabled());

    /* Determine which skins are available */
    std::list<std::wstring> skins = FindSkins(Settings::SkinDir().c_str());
    for (std::wstring skin : skins) {
        _ctxt->AddComboItem(CMB_SKIN, skin);
    }

    /* Update the combo box with the current skin */
    std::wstring current = settings->CurrentSkin();
    int idx = _ctxt->SelectComboItem(CMB_SKIN, current);
    if (idx == CB_ERR) {
        _ctxt->SelectComboItem(CMB_SKIN, DEFAULT_SKIN);
    }
    LoadSkinInfo(current);

    /* Populate the language box */
    std::list<std::wstring> languages = FindLanguages(
        settings->LanguagesDir().c_str());
    for (std::wstring language : languages) {
        _ctxt->AddComboItem(CMB_LANG, language);
    }
    std::wstring currentLang = settings->LanguageName();
    _ctxt->SelectComboItem(CMB_LANG, currentLang);
}

void General::SaveSettings() {
    RunOnStartup(true);
}

bool General::RunOnStartup() {
    long res;
    HKEY key;
    bool run = false;

    res = RegOpenKeyEx(HKEY_CURRENT_USER, STARTUP_KEY, NULL, KEY_READ, &key);
    if (res == ERROR_SUCCESS) {
        res = RegQueryValueEx(key, KEY_NAME, NULL, NULL, NULL, NULL);
        run = (res == ERROR_SUCCESS);
    }

    RegCloseKey(key);
    return run;
}

bool General::RunOnStartup(bool enable) {
    long res;
    HKEY key;
    bool ok = false;

    std::wstring path = Settings::AppDir() + L"\\3RVX.exe";

    res = RegOpenKeyEx(HKEY_CURRENT_USER, STARTUP_KEY, NULL, KEY_ALL_ACCESS, &key);
    if (res == ERROR_SUCCESS) {
        if (enable) {
            res = RegSetValueEx(key, KEY_NAME, NULL, REG_SZ,
                (LPBYTE) path.c_str(), path.size() + 1);
            ok = (res == ERROR_SUCCESS);
        } else {
            res = RegDeleteValue(key, KEY_NAME);
            ok = (res == ERROR_SUCCESS);
        }
    }
    RegCloseKey(key);
    return ok;
}



std::list<std::wstring> General::FindSkins(std::wstring dir) {
    std::list<std::wstring> skins;
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    CLOG(L"Finding skins in: %s", dir.c_str());
    dir += L"\\*";
    hFind = FindFirstFile(dir.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return skins;
    }

    do {
        std::wstring fName(ffd.cFileName);
        if (fName.at(0) == L'.') {
            continue;
        }
        QCLOG(L"%s", fName.c_str());
        skins.push_back(fName);
    } while (FindNextFile(hFind, &ffd));
    FindClose(hFind);

    return skins;
}

void General::LoadSkinInfo(std::wstring skinName) {
    std::wstring skinXML = Settings::Instance()->SkinXML(skinName);
    SkinInfo s(skinXML);

    std::wstring authorText(L"Author: ");
    authorText.append(s.Author());
    _ctxt->SetText(LBL_AUTHOR, authorText);

    std::wstring url = s.URL();
    if (url == L"") {
        _ctxt->Disable(BTN_WEBSITE);
    } else {
        _url = s.URL();
        _ctxt->Enable(BTN_WEBSITE);
    }
}

std::list<std::wstring> General::FindLanguages(std::wstring dir) {
    std::list<std::wstring> languages;
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    CLOG(L"Finding language translations in: %s", dir.c_str());
    dir += L"\\*.xml";
    hFind = FindFirstFile(dir.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        CLOG(L"FindFirstFile() failed");
        return languages;
    }

    do {
        std::wstring fName(ffd.cFileName);

        if (fName.at(0) == L'.') {
            continue;
        }

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        QCLOG(L"%s", fName.c_str());
        languages.push_back(fName);
    } while (FindNextFile(hFind, &ffd));
    FindClose(hFind);

    return languages;
}

