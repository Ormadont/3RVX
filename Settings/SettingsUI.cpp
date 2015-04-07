#include "SettingsUI.h"
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

/* DLGTEMPLATEEX Structure */
#include <pshpack1.h>
typedef struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>

#include "../3RVX/Logger.h"
#include "../3RVX/Settings.h"

/* Tabs*/
#include "General.h"

HINSTANCE hInst;
HWND mainWnd;

int APIENTRY wWinMain(
        _In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPTSTR    lpCmdLine,
        _In_ int       nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    Logger::Start();
    CLOG(L"Starting SettingsUI...");
    Settings *settings = Settings::Instance();
    settings->Load();

    hInst = hInstance;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = NULL;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = NULL;
    wcex.cbWndExtra = NULL;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"3RVX SettingsUI";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        CLOG(L"Could not register class");
        return EXIT_FAILURE;
    }

    HWND mainWnd = CreateWindowEx(
        NULL, L"3RVX SettingsUI", L"3RVX SettingsUI",
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, hInst, NULL);
    PostMessage(mainWnd, WM_INITDIALOG, NULL, NULL);

    PROPSHEETPAGE psp[4];

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = NULL;
    psp[0].hInstance = hInst;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = (DLGPROC) General::GeneralTabProc;
    psp[0].pszTitle = NULL;
    psp[0].lParam = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = NULL;
    psp[1].hInstance = hInst;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_DISPLAY);
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = (DLGPROC) ComboDlgProc;
    psp[1].pszTitle = NULL;
    psp[1].lParam = 0;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = NULL;
    psp[2].hInstance = hInst;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_HOTKEYS);
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = (DLGPROC) ComboDlgProc;
    psp[2].pszTitle = NULL;
    psp[2].lParam = 0;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = NULL;
    psp[3].hInstance = hInst;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_ABOUT);
    psp[3].pszIcon = NULL;
    psp[3].pfnDlgProc = (DLGPROC) ComboDlgProc;
    psp[3].pszTitle = NULL;
    psp[3].lParam = 0;

    PROPSHEETHEADER psh;
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK;
    psh.hwndParent = mainWnd;
    psh.hInstance = hInst;
    psh.pszIcon = MAKEINTRESOURCE(COLOR_WINDOW);
    psh.pszCaption = L"3RVX Settings";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = (PFNPROPSHEETCALLBACK) PropSheetProc;

    CLOG(L"Launching modal property sheet.");
    return PropertySheet(&psh);
}

LRESULT CALLBACK WndProc(
        HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

DLGPROC ComboDlgProc(HWND hdlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam) {

    LPNMHDR     lpnmhdr;

    switch (uMessage) {
    case WM_INITDIALOG: {
        break;
    }
        // on any command notification, tell the property sheet to enable the Apply button
    case WM_COMMAND:
        //PropSheet_Changed(GetParent(hdlg), hdlg);
        break;

    case WM_NOTIFY:
        lpnmhdr = (NMHDR FAR *)lParam;

        switch (lpnmhdr->code) {
        case PSN_APPLY:   //sent when OK or Apply button pressed
            break;

        case PSN_RESET:   //sent when Cancel button pressed
            break;

        case PSN_SETACTIVE:
            //this will be ignored if the property sheet is not a wizard
            //PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_BACK | PSWIZB_FINISH);
            return FALSE;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return FALSE;
}

int CALLBACK PropSheetProc(HWND hWnd, UINT msg, LPARAM lParam) {
    switch (msg) {
    case PSCB_PRECREATE:
        /* Disable the help button: */
        if (((LPDLGTEMPLATEEX) lParam)->signature == 0xFFFF) {
            ((LPDLGTEMPLATEEX) lParam)->style &= ~DS_CONTEXTHELP;
        } else {
            ((LPDLGTEMPLATE) lParam)->style &= ~DS_CONTEXTHELP;
        }
        return TRUE;
    }

    return TRUE;
}