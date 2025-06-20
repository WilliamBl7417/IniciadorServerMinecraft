#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <fstream>
#include <string>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

const int ID_BTN_SERVER = 1;
const int ID_BTN_PLAYIT = 2;
const int ID_BTN_LAUNCH = 3;

wchar_t rutaServidor[MAX_PATH] = L"";
wchar_t rutaPlayit[MAX_PATH] = L"";
wchar_t rutaConfig[MAX_PATH] = L"";

bool ArchivoExiste(const wchar_t* ruta) {
    DWORD attrs = GetFileAttributesW(ruta);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void DeterminarRutaConfig() {
    GetModuleFileNameW(NULL, rutaConfig, MAX_PATH);
    PathRemoveFileSpecW(rutaConfig);
    wcscat_s(rutaConfig, L"\\config.txt");
}

void GuardarConfiguracion() {
    std::wofstream file(rutaConfig);
    if (file) {
        file << rutaServidor << std::endl;
        file << rutaPlayit << std::endl;
    }
}

bool CargarConfiguracion() {
    std::wifstream file(rutaConfig);
    if (file) {
        std::wstring linea1, linea2;
        std::getline(file, linea1);
        std::getline(file, linea2);
        linea1.erase(linea1.find_last_not_of(L"\r\n") + 1);
        linea2.erase(linea2.find_last_not_of(L"\r\n") + 1);
        wcscpy_s(rutaServidor, linea1.c_str());
        wcscpy_s(rutaPlayit, linea2.c_str());
        return true;
    }
    return false;
}

void LanzarProgramas() {
    Sleep(3000); // Espera silenciosa de 3 segundos
    ShellExecuteW(NULL, L"open", rutaServidor, NULL, NULL, SW_SHOWNORMAL);
    Sleep(5000); // Espera antes de iniciar el segundo
    ShellExecuteW(NULL, L"open", rutaPlayit, NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (wParam == ID_BTN_SERVER || wParam == ID_BTN_PLAYIT) {
            OPENFILENAMEW ofn = { 0 };
            wchar_t archivo[MAX_PATH] = L"";
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = archivo;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Executable Files\0*.exe\0";
            ofn.Flags = OFN_FILEMUSTEXIST;
            ofn.hwndOwner = hwnd;

            if (GetOpenFileNameW(&ofn)) {
                if (wParam == ID_BTN_SERVER)
                    wcscpy_s(rutaServidor, archivo);
                else
                    wcscpy_s(rutaPlayit, archivo);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        else if (wParam == ID_BTN_LAUNCH) {
            if (ArchivoExiste(rutaServidor) && ArchivoExiste(rutaPlayit)) {
                GuardarConfiguracion();
                DestroyWindow(hwnd);
                LanzarProgramas();
            }
            else {
                MessageBoxW(hwnd, L"Asegúrate de que ambas rutas sean válidas.", L"Error", MB_ICONERROR);
            }
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        TextOutW(hdc, 20, 20, L"Ruta Servidor:", 14);
        TextOutW(hdc, 150, 20, rutaServidor, wcslen(rutaServidor));
        TextOutW(hdc, 20, 60, L"Ruta Playit:", 13);
        TextOutW(hdc, 150, 60, rutaPlayit, wcslen(rutaPlayit));
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CrearVentana(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t* clase = L"VentanaLauncher";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = clase;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, clase, L"Seleccionar ejecutables",
        WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 200,
        NULL, NULL, hInstance, NULL
    );

    CreateWindow(L"BUTTON", L"Elegir Servidor", WS_VISIBLE | WS_CHILD,
        450, 15, 120, 25, hwnd, (HMENU)ID_BTN_SERVER, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Elegir Playit", WS_VISIBLE | WS_CHILD,
        450, 55, 120, 25, hwnd, (HMENU)ID_BTN_PLAYIT, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Iniciar", WS_VISIBLE | WS_CHILD,
        250, 110, 100, 35, hwnd, (HMENU)ID_BTN_LAUNCH, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    DeterminarRutaConfig();
    if (CargarConfiguracion()) {
        bool servidorOK = ArchivoExiste(rutaServidor);
        bool playitOK = ArchivoExiste(rutaPlayit);

        if (servidorOK && playitOK) {
            LanzarProgramas();
        }
        else {
            std::wstring mensaje = L"Error: ";
            if (!servidorOK) mensaje += L"No se encontró el archivo del servidor.\n";
            if (!playitOK)   mensaje += L"No se encontró el archivo de Playit.\n";
            mensaje += L"Selecciona nuevamente las rutas.";
            MessageBoxW(NULL, mensaje.c_str(), L"Rutas inválidas", MB_ICONWARNING);
            CrearVentana(hInstance, nCmdShow);
        }
    }
    else {
        CrearVentana(hInstance, nCmdShow);
    }

    return 0;
}
