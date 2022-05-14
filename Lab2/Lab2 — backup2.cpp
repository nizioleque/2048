// Lab2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Lab2.h"
#include <string.h>
#include <string>
#include <time.h>
#include <fstream>
//#include "functions.cpp"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#define MAX_LOADSTRING 100

#define AREA_SIZE 60
#define AREA_MARGIN 10
#define AREA_COUNT 4
const int borderRadius = AREA_SIZE / 6;
const int textHeight = AREA_SIZE / 2;
const int marginTop = AREA_SIZE + AREA_MARGIN;
const COLORREF textColor = RGB(0, 0, 0);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// my global variables
WCHAR szWindowClassChild[MAX_LOADSTRING]; 
RECT globalRC;
HWND parentHWND;
HWND mirrorHWND;
bool pauseMove;
// home
HWND hWndChild[AREA_COUNT][AREA_COUNT];
int AreaValues[] = { 0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };
//std::string AreaNames[] = { "", "1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048" 
COLORREF AreaColors[] = {
    RGB(204, 192, 174), //0
    RGB(238, 228, 198), //2
    RGB(239, 225, 218), //4
    RGB(243, 179, 124), //8
    RGB(246, 153, 100), //16
    RGB(246, 125, 98), //32
    RGB(247, 93, 60), //64
    RGB(237, 206, 116), //128
    RGB(239, 204, 98), //256
    RGB(243, 201, 85), //512
    RGB(238, 200, 72), //1024
    RGB(239, 192, 47) //2048
};
//int AreaData[AREA_COUNT][AREA_COUNT] = { {0,0,1,0},{2,0,2,0},{0,2,1,0},{2,0,2,0} };
int AreaData[AREA_COUNT][AREA_COUNT] = { 0 };

int filledAreas = 7;
enum winValues {_8 = 3, _16 = 4, _64 = 6, _2048 = 11};
enum gameStates {INGAME, WIN, LOSE};

int winValue = _2048;
int gameState = INGAME;
int score = 0;

// my helper functions

void SetArea(HDC* hdc, HWND* hWnd) {
    HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(*hdc, pen);

    // score area
    {
        COLORREF currColor = AreaColors[0];
        std::string currText = std::to_string(score);
        std::wstring ss = std::wstring(currText.begin(), currText.end());
        LPCWSTR s = ss.c_str();

        int marginY = AREA_MARGIN;
        int marginX = AREA_MARGIN;
        int width = AREA_COUNT * AREA_SIZE + (AREA_COUNT - 1) * AREA_MARGIN;
        int height = AREA_SIZE;

        HBRUSH brush = CreateSolidBrush(currColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(*hdc, brush);
        RoundRect(*hdc, marginX, marginY, marginX + width, marginY + height, borderRadius, borderRadius);

        SelectObject(*hdc, oldBrush);
        DeleteObject(brush);

        SetTextColor(*hdc, textColor);

        HFONT font = CreateFontW(
            textHeight,
            0,
            0,
            0,
            FW_BOLD,
            false,
            FALSE,
            0,
            EASTEUROPE_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            _T("Comic Sans MS")
        );
        HFONT oldFont = (HFONT)SelectObject(*hdc, font);

        RECT rc = {
            marginX, marginY, marginX + width, marginY + height
        };
        DrawText(*hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(*hdc, oldFont);
        DeleteObject(font);

    }


    // game area
    for (int i = 0; i < AREA_COUNT; i++) {
        for (int j = 0; j < AREA_COUNT; j++)
        {
            COLORREF currColor = AreaColors[AreaData[i][j]];
            std::string currText = AreaData[i][j] ? std::to_string(AreaValues[AreaData[i][j]]) : "";
            std::wstring ss = std::wstring(currText.begin(), currText.end());
            LPCWSTR s = ss.c_str();

            int marginY = i * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN + marginTop;
            int marginX = j * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN;

            HBRUSH brush = CreateSolidBrush(currColor);
            HBRUSH oldBrush = (HBRUSH)SelectObject(*hdc, brush);
            RoundRect(*hdc, marginX, marginY, marginX + AREA_SIZE, marginY + AREA_SIZE, borderRadius, borderRadius);

            SelectObject(*hdc, oldBrush);
            DeleteObject(brush);
            
            SetTextColor(*hdc, textColor);

            HFONT font = CreateFontW(
                textHeight,
                0,
                0,
                0,
                FW_BOLD,
                false,
                FALSE,
                0,
                EASTEUROPE_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                _T("Comic Sans MS")
            );
            HFONT oldFont = (HFONT)SelectObject(*hdc, font);

            RECT rc = {
                marginX, marginY, marginX + AREA_SIZE, marginY + AREA_SIZE
            };
            DrawText(*hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(*hdc, oldFont);
            DeleteObject(font);

        }
    }

    SelectObject(*hdc, oldPen);
    DeleteObject(pen);

}

void RefreshLayout() {
    InvalidateRect(parentHWND, NULL, FALSE);
    InvalidateRect(mirrorHWND, NULL, FALSE);
}

int& AccessData(int i, int j, int direction) {
    // direction:
    //  0 up
    //  1 right
    //  2 down
    //  3 left

    switch (direction) {
    case 0:
        return AreaData[i][j];
    case 1:
        return AreaData[j][AREA_COUNT - 1 - i];
    case 2:
        return AreaData[AREA_COUNT - 1 - i][j];
    case 3:
        return AreaData[j][i];
    }
}

void AddBlock() {
    bool success = false;
    do {
        int randX = rand() % AREA_COUNT;
        int randY = rand() % AREA_COUNT;
        if (AreaData[randY][randX] == 0) {
            AreaData[randY][randX] = 1;
            success = true;
        }
    } while (!success);
}

void NewGame() {
    score = 0;
    for (int i = 0; i < AREA_COUNT; i++)
    {
        for (int j = 0; j < AREA_COUNT; j++)
        {
            AreaData[i][j] = 0;
        }
    }
    filledAreas = 0;
    gameState = INGAME;
    AddBlock();
    RefreshLayout();
}

void GameOver(bool win) {
    gameState = win ? WIN : LOSE;
}

void UpdateGoal(int wmId) {
    HMENU menu = GetMenu(parentHWND);

    CheckMenuItem(menu, ID_GOAL_8, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(menu, ID_GOAL_16, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(menu, ID_GOAL_64, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(menu, ID_GOAL_2048, MF_BYCOMMAND | MF_UNCHECKED);

    switch (wmId) {
        case ID_GOAL_8:
            winValue = _8;
            CheckMenuItem(menu, ID_GOAL_8, MF_BYCOMMAND | MF_CHECKED);
            break;

        case ID_GOAL_16:
            winValue = _16;
            CheckMenuItem(menu, ID_GOAL_16, MF_BYCOMMAND | MF_CHECKED);
            break;

        case ID_GOAL_64:
            winValue = _64;
            CheckMenuItem(menu, ID_GOAL_64, MF_BYCOMMAND | MF_CHECKED);
            break;

        case ID_GOAL_2048:
            winValue = _2048;
            CheckMenuItem(menu, ID_GOAL_2048, MF_BYCOMMAND | MF_CHECKED);
            break;
    }

    for (int i = 0; i < AREA_COUNT; i++)
    {
        for (int j = 0; j < AREA_COUNT; j++)
        {
            if (AreaData[i][j] >= winValue) GameOver(true);
        }
    }
}



void KeyReact(int direction) {
    if (gameState != INGAME) return;
    bool movementMade = false;

    // move & collapse
    for (int j = 0; j < AREA_COUNT; j++) {
        // for each column ....

        int i = 0;
        int empty = 0;
        while (i < AREA_COUNT) {

            if (AccessData(i, j, direction) == 0) {
                // komórka pusta - przesuwam kolumnę w górę
                for (int k = i; k < AREA_COUNT - 1; k++) {

                    // zmieniam tylko jeśli się różnią
                    if (AccessData(k, j, direction) != AccessData(k + 1, j, direction)) {
                        AccessData(k, j, direction) = AccessData(k + 1, j, direction);
                        movementMade = true;
                    }

                }

                if (AccessData(AREA_COUNT - 1, j, direction) != 0) {
                    AccessData(AREA_COUNT - 1, j, direction) = 0;
                    movementMade = true;
                }

                empty++;
            }

            else if (i < AREA_COUNT - 1 && AccessData(i, j, direction) == AccessData(i + 1, j, direction)) {
                // dwa takie same - łączę
                AccessData(i, j, direction)++;
                movementMade = true;
                score += AreaValues[AccessData(i, j, direction)];
                if (AccessData(i, j, direction) >= winValue) {
                    GameOver(true);
                    return;
                }

                // i przesuwam od następnego
                for (int k = i + 1; k < AREA_COUNT - 1; k++) {
                    AccessData(k, j, direction) = AccessData(k + 1, j, direction);
                }
                AccessData(AREA_COUNT-1, j, direction) = 0;

                i++;
                if (filledAreas > 0) filledAreas--;
            }

            else if (i < AREA_COUNT - 1 && AccessData(i+1, j, direction) == 0) {
                // następny pusty
                // przesuwam od następnego
                for (int k = i + 1; k < AREA_COUNT - 1; k++) {

                    if (AccessData(k, j, direction) != AccessData(k + 1, j, direction)) {
                        AccessData(k, j, direction) = AccessData(k + 1, j, direction);
                        movementMade = true;
                    }
                }

                if (AccessData(AREA_COUNT - 1, j, direction) != 0) {
                    AccessData(AREA_COUNT - 1, j, direction) = 0;
                    movementMade = true;
                }

                empty++;
            }

            else {
                i++;
            }

            if (empty >= 4) break;
        }
    }

    // check if game is over
    if (filledAreas >= AREA_COUNT * AREA_COUNT) {
        GameOver(false);
        return;
    }

    // add new
    if (movementMade) {
        AddBlock();
    }

    RefreshLayout();
}

void SaveGame() {
    std::string currText = std::to_string(gameState);
    std::wstring ss = std::wstring(currText.begin(), currText.end());
    LPCWSTR s = ss.c_str();

    WritePrivateProfileStringW(
        L"GAME",
        L"STATUS",
        s,
        L".\\2048.ini"
    );

    currText = std::to_string(score);
    ss = std::wstring(currText.begin(), currText.end());
    s = ss.c_str();

    WritePrivateProfileStringW(
        L"GAME",
        L"SCORE",
        s,
        L".\\2048.ini"
    );

    currText = std::to_string(winValue);
    ss = std::wstring(currText.begin(), currText.end());
    s = ss.c_str();

    WritePrivateProfileStringW(
        L"GAME",
        L"GOAL",
        s,
        L".\\2048.ini"
    );

    // save AreaData
    currText = "";
    for (int i = 0; i < AREA_COUNT; i++) {
        for (int j = 0; j < AREA_COUNT; j++) {
            currText += std::to_string(AreaData[i][j]) + ";";
        }
    }

    ss = std::wstring(currText.begin(), currText.end());
    s = ss.c_str();

    WritePrivateProfileStringW(
        L"GAME",
        L"BOARD",
        s,
        L".\\2048.ini"
    );

}

void SaveGame2() {
    std::ofstream stream("2048.txt");

    stream << gameState << std::endl;
    stream << score << std::endl;
    stream << winValue << std::endl;

    for (int i = 0; i < AREA_COUNT; i++) {
        for (int j = 0; j < AREA_COUNT; j++) {
            stream << AreaData[i][j] << std::endl;
        }
    }

    stream.close();
}

void LoadGame2() {
    std::ifstream stream("2048.txt");

    stream >> gameState;
    stream >> score;
    stream >> winValue;

    for (int i = 0; i < AREA_COUNT; i++) {
        for (int j = 0; j < AREA_COUNT; j++) {
            stream >> AreaData[i][j];
        }
    }

    stream.close();
}

void LoadGame() {
    gameState = GetPrivateProfileInt(
        L"GAME",
        L"STATUS",
        INGAME,
        L".\\2048.ini"
    );

    score = GetPrivateProfileInt(
        L"GAME",
        L"SCORE",
        0,
        L".\\2048.ini"
    );

    winValue = GetPrivateProfileInt(
        L"GAME",
        L"STATUS",
        _2048,
        L".\\2048.ini"
    );

    //char buf[3 * AREA_COUNT * AREA_COUNT];
    int len = 4 * AREA_COUNT * AREA_COUNT;
    TCHAR buf[4 * AREA_COUNT * AREA_COUNT];
    char buf2[4 * AREA_COUNT * AREA_COUNT];

    DWORD charsRead = GetPrivateProfileStringW(
        L"GAME",
        L"BOARD",
        L"",
        buf,
        len,
        L".\\2048.ini"
    );

    OutputDebugString(buf);
    OutputDebugString(L"\n");


    //wcstombs_s(nullptr, buf2, len, buf, len-1);
    wcstombs(buf2, buf, len - 1);

    char* pch;
    pch = strtok(buf2, ";");
    int i = 0, j = 0;
    while (pch != NULL)
    {
        int val = atoi(pch);
        AreaData[i][j] = val;
        j++;
        if (j >= AREA_COUNT) {
            i++;
            j = 0;
        }

        pch = strtok(NULL, " ,.-");
    }


    //SaveGame();

}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

ATOM                MyRegisterChildClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProcChild(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_CHILD_TITLE, szWindowClassChild, MAX_LOADSTRING);

    MyRegisterClass(hInstance);
    MyRegisterChildClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);

    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(250, 247, 238)));

    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2);
    //wcex.lpszMenuName = nullptr;

    wcex.lpszClassName  = szWindowClass;
    
    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterChildClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcChild;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(204, 192, 174)));

    //wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClassChild;

    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // configuration
   srand(time(NULL));
   //SaveGame2();
   LoadGame2();

   // custom
   int windowWidth = AREA_COUNT * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN;
   int windowHeight = AREA_COUNT * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN + marginTop;

   RECT rc;
   rc.top = 0;
   rc.bottom = windowHeight;
   rc.left = 0;
   rc.right = windowWidth;
   AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, TRUE);
   globalRC = rc;

   //HWND hWnd = CreateWindowExW(WS_EX_TOPMOST, szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
   //    rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);


   //HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, rc.right-rc.left, rc.bottom-rc.top, nullptr, nullptr, hInstance, nullptr);

   HWND hWndMirror = CreateWindowW(szWindowClass, szTitle, WS_CAPTION | WS_OVERLAPPED,
       CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, hWnd, nullptr, hInstance, nullptr);

   parentHWND = hWnd;
   mirrorHWND = hWndMirror;
   pauseMove = false;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   ShowWindow(hWndMirror, nCmdShow);

   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            case ID_NEWGAME:
                NewGame();
                break;

            case ID_GOAL_8:
            case ID_GOAL_16:
            case ID_GOAL_64:
            case ID_GOAL_2048:
                UpdateGoal(wmId);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
            
            // custom
            SetArea(&hdc, &hWnd);
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CREATE:
    {

        //for (int i = 0; i < AREA_COUNT; i++) {
        //    for (int j = 0; j < AREA_COUNT; j++) {
        //        int marginY = i * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN;
        //        int marginX = j * (AREA_MARGIN + AREA_SIZE) + AREA_MARGIN;

        //        hWndChild[i][j] = CreateWindowW(szWindowClassChild, nullptr, WS_CHILD | WS_VISIBLE,
        //            marginX, marginY, AREA_SIZE, AREA_SIZE, hWnd, nullptr, hInst, nullptr);

        //    }
        //}


    }
    break;

    case WM_MOVE:
    {
        if (pauseMove) break;

        pauseMove = true;

        HWND otherWindow = hWnd == parentHWND ? mirrorHWND : parentHWND;

        RECT rc;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        // get current size of the window
        GetWindowRect(hWnd, &rc);
        // modify size of the window
        screenWidth -= rc.right;
        screenHeight -= rc.bottom;

        MoveWindow(otherWindow, screenWidth,
            screenHeight, 0, 0, TRUE);

        int midX = GetSystemMetrics(SM_CXSCREEN) / 2;
        int w = rc.right - rc.left;

        int midY = GetSystemMetrics(SM_CYSCREEN) / 2;
        int h = rc.bottom - rc.top;

        if (rc.left >= midX - w && rc.left <= midX
            && rc.top >= midY - h && rc.top <= midY) {
            SetWindowLong(mirrorHWND, GWL_EXSTYLE, GetWindowLong(mirrorHWND, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(mirrorHWND, 0, (255 * 50) / 100, LWA_ALPHA);
        }
        else {
            SetWindowLong(parentHWND, GWL_EXSTYLE, GetWindowLong(mirrorHWND, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(mirrorHWND, 0, 255, LWA_ALPHA);
        }

        pauseMove = false;
    }
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMaxSize.x = mmi->ptMaxTrackSize.x = globalRC.right - globalRC.left;
        mmi->ptMaxSize.y = mmi->ptMaxTrackSize.y = globalRC.bottom - globalRC.top;

        mmi->ptMinTrackSize.x = globalRC.right - globalRC.left;
        mmi->ptMinTrackSize.y = globalRC.bottom - globalRC.top;
    }
    break;

    case WM_KEYDOWN:
    {
        int keyCode = (int)wParam;
        switch (keyCode) {
        case 0x57:
            // W key
            KeyReact(0);

            break;
        case 0x41:
            // A key
            KeyReact(3);

            break;
        case 0x53:
            // S key
            KeyReact(2);

            break;
        case 0x44:
            // D key
            KeyReact(1);

            break;
        }
    }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        
        // custom 

        //RECT rc;
        //GetClientRect(hWnd, &rc);
        //HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
        //HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        //Rectangle(hdc, 3, 3, 10, 10);

        //SelectObject(hdc, oldBrush);
        //DeleteObject(brush);



        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
