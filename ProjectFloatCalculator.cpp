// ProjectFloatCalculator.cpp : Defines the entry point for the application.
//

#include "ProjectFloatCalculator.h"
#include "TaskMan.h"
#include "FileDlgMgr.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
FileDlgMgr fileDlgMgr;
TaskMan tasks;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MsgNewTask(HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_PROJECTFLOATCALCULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECTFLOATCALCULATOR));

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
    WNDCLASSEXW wcex {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECTFLOATCALCULATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PROJECTFLOATCALCULATOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
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
    static POINT click = {0, 0};
    static bool dragging = false;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_FILE_NEW:
                switch (MessageBox(hWnd,
                    TEXT("If you haven't saved your progress, you will lose your work.\n\nDo you wish to continue?"),
                    TEXT("New Project"),
                    MB_ICONQUESTION | MB_YESNO))
                {
                case IDYES:
                    tasks.clearMap();
                    break;

                default: break;
                }
                break;

            case IDM_FILE_OPEN:
                switch (MessageBox(hWnd,
                    TEXT("If you haven't saved your progress, you will lose your work.\n\nDo you wish to continue?"),
                    TEXT("New Project"),
                    MB_ICONQUESTION | MB_YESNO))
                {
                case IDYES:
                    {
                        std::wstring filename;

                        fileDlgMgr.init(hWnd);
                        filename = fileDlgMgr.Open();

                        if (!filename.empty())
                            tasks.loadTasks(filename);
                    }
                    break;

                default: break;
                }
                break;

            case IDM_FILE_SAVE:
                {
                    std::wstring filename;

                    fileDlgMgr.init(hWnd);
                    filename = fileDlgMgr.Save();

                    if (!filename.empty())
                        tasks.saveTasks(filename);
                }
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            case ID_ADD_TASK:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_NEWTASKDLG), hWnd, MsgNewTask);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            SetBkMode(ps.hdc, TRANSPARENT);
            
            tasks.draw(ps);

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_LBUTTONDOWN:
        /* Obtain mouse position for drag detection */
        SetCapture(hWnd);
        GetCursorPos(&click);
        break;

    case WM_MOUSEMOVE:
        if (DragDetect(hWnd, click))
        {
            POINT np;

            GetCursorPos(&np);
            np.x -= click.x;
            np.y -= click.y;

            if (!dragging)
            {
                dragging = true;
            }
            else
            {
                tasks.mouseDrag(hWnd, np.x, np.y);
            }
        }
        else
        {
            if (dragging)
            {
                dragging = false;

                tasks.commitDrag(hWnd);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
        }
        break;

    case WM_LBUTTONUP:
        ReleaseCapture();
        if (wParam & MK_SHIFT)
        {
            tasks.mouseSelect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        else
        {
            tasks.mouseHighlight(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        break;

    case WM_RBUTTONUP:
        tasks.mouseGraph(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        break;

    case WM_KEYUP:
        switch (wParam)
        {
        case VK_DELETE:
            tasks.deleteSelected();
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            break;

        case VK_SPACE:
            tasks.calculate();
            break;

        default: return DefWindowProc(hWnd, message, wParam, lParam);
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



#if __cplusplus >= 201103L
std::wstring getEditBoxText(HWND parent, INT_PTR id) noexcept
{
    std::wstring r;
    HWND dlg;
    int sz;

    // Get handle from ID and check
    dlg = GetDlgItem(parent, id);
    if (!dlg) return r;

    // Allocate space and copy in
    r.resize(sz = GetWindowTextLength(dlg));
    GetWindowText(dlg, &r[0], sz+1); // Only valid since C++11

    return r;
}
#else
std::wstring getEditBoxText(HWND parent, int id) noexcept
{
    wchar_t *pbuf;
    wchar_t buf[256] {};
    std::wstring r;
    HWND dlg;
    int sz;

    // Get handle from ID and check
    dlg = GetDlgItem(parent, id);
    if (!dlg) return r;

    // Allocate space and copy in
    sz = GetWindowTextLength(dlg);
    if (sz > 255)
    {
        pbuf = new(std::nothrow) wchar_t[sz+1];
        if (!pbuf) return r;
    }
    else
    {
        pbuf = buf;
    }
    GetWindowText(dlg, pbuf, sz+1);
    try {
        r.assign(pbuf, sz);
    } catch (...) {
        r.clear();
    }

    // Free memory and exit
    if (sz > 255) delete[] pbuf;
    return r;
}
#endif



unsigned int getEditBoxNum(HWND parent, int id) noexcept
{
    wchar_t text[32] {};
    wchar_t *end = text+32;
    HWND dlg;
    int r;

    // Get handle from ID and check
    dlg = GetDlgItem(parent, id);
    if (!dlg) return 0;

    // Length check
    if (GetWindowTextLength(dlg) > 31)
    {
        MessageBox(parent, TEXT("Edit field number is too long."), TEXT("Error"), MB_ICONHAND);
        return 0;
    }
    GetWindowText(dlg, text, 32);

    // Conversion
    r = wcstoul(text, &end, 10);

    return r;
}



// Message handler for about box.
INT_PTR CALLBACK MsgNewTask(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_CLOSE:
        EndDialog(hDlg, IDC_BTN_CANCELADDTASK);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_BTN_ADDTASK:
                tasks.addTask(getEditBoxText(hDlg, IDC_TASKNAME),
                              getEditBoxNum (hDlg, IDC_UNITTIME));

                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;

            case IDC_BTN_CANCELADDTASK:
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            return DefWindowProc(hDlg, message, wParam, lParam);

        default: return DefWindowProc(hDlg, message, wParam, lParam);
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}
