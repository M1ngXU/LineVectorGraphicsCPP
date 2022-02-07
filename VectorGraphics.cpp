#include <iostream>
#include <Windows.h>
#include <windowsx.h>
#include "Vector3.cpp"
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

std::vector<Vector3> Vertices;
std::vector<std::pair<size_t, size_t>> Lines;

const UINT_PTR UPDATE_TIMER = 123456;

double Distance = 1;
double RotationX = M_PI_4;
double RotationY = -M_PI_4;
double tick = 0;
Vector3 CameraForward = Vector3(0, 0, 0);
Vector3 CameraRight = Vector3(0, 0, 0);
Vector3 CameraUp = Vector3(0, 0, 0);
Vector3 CameraPosition = Vector3(-5, 5, -5);
int LastX = 0;
int LastY = 0;
HDC hdcMem = nullptr;
HBITMAP hbmMem = nullptr;

Vector3 TransformPoint(Vector3 Direction, Vector3 From) {
    Vector3 ScreenRelativePosition = From - CameraForward * Distance - CameraPosition;
    double t = (Distance * (CameraForward * CameraForward)) / (CameraForward * Direction);

    double const_factor = (Vector3() - Direction) * CameraForward;
    return Vector3(
        CameraUp.Cross(Vector3() - Direction) * ScreenRelativePosition / const_factor,
        -Direction.Cross(CameraRight) * ScreenRelativePosition / const_factor,
        t
    );
}

LONG GetWidth(HWND hWnd) {
    RECT r;
    GetClientRect(hWnd, &r);
    return r.bottom;
}

LONG GetHeight(HWND hWnd) {
    RECT r;
    GetClientRect(hWnd, &r);
    return r.right;
}

void InitiateRedraw(HWND hWnd) {
    RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
}

void Update(HWND hWnd) {
    Vertices[0].x = sin(tick);
    Vertices[0].z = cos(tick);
    tick += M_PI / 40;
}

void RedrawScreen(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT r;
    GetClientRect(hWnd, &r);

    HGDIOBJ hOld = SelectObject(hdcMem, hbmMem);
    FillRect(hdcMem, &r, CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF)));

    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));

    for (std::pair<size_t, size_t> l : Lines) {
        Vector3 FromWorld = Vertices[std::get<0>(l)];
        Vector3 ToWorld = Vertices[std::get<1>(l)];

        Vector3 From = TransformPoint(FromWorld - CameraPosition, CameraPosition);
        Vector3 To = TransformPoint(ToWorld - CameraPosition, CameraPosition);

        bool ValidFrom = From.z < 1 && From.z > 0;
        bool ValidTo = To.z < 1 && To.z > 0;

        if (ValidFrom && !ValidTo) {
            To = TransformPoint(FromWorld - ToWorld, FromWorld);
            ValidTo = true;
        }
        if (ValidTo && !ValidFrom) {
            From = TransformPoint(ToWorld - FromWorld, ToWorld);
            ValidFrom = true;
        }
        if (ValidFrom && ValidTo) {
            MoveToEx(hdcMem, (int)((0.5 + From.x) * r.right), (int)((0.5 - From.y) * r.bottom), nullptr);
            LineTo(hdcMem, (int)((0.5 + To.x) * r.right), (int)((0.5 - To.y) * r.bottom));
        }
    }

    BitBlt(hdc, 0, 0, r.right, r.bottom, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdc, hOld);

    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_MOUSEWHEEL:
        Distance *= pow(1.1, GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
        return 0;
    case WM_CREATE:
    case WM_SETFOCUS:
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(hWnd, &p);
        LastX = p.x;
        LastY = p.y;

        wParam = 0;
        lParam = MAKELPARAM(LastX, LastY);
    case WM_MOUSEMOVE: {
        if ((wParam & MK_SHIFT) != MK_SHIFT) {
            RotationX += (double)(GET_X_LPARAM(lParam) - LastX) / (double)GetWidth(hWnd) * M_PI;
            RotationY -= (double)(GET_Y_LPARAM(lParam) - LastY) / (double)GetHeight(hWnd) * M_PI_2;

            CameraUp.x = -sin(RotationX) * sin(RotationY);
            CameraUp.y = cos(RotationY);
            CameraUp.z = -cos(RotationX) * sin(RotationY);

            CameraRight.x = cos(RotationX);
            CameraRight.y = 0;
            CameraRight.z = -sin(RotationX);

            CameraForward = CameraRight.Cross(CameraUp);
            InitiateRedraw(hWnd);
        }
        if ((wParam & MK_CONTROL) == MK_CONTROL) {
            POINT p = {};
            p.x = LastX;
            p.y = LastY;
            ClientToScreen(hWnd, &p);
            SetCursorPos(p.x, p.y);
            SetCursor(nullptr);
        } else {
            LastX = GET_X_LPARAM(lParam);
            LastY = GET_Y_LPARAM(lParam);
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        return 0;
    } case WM_DESTROY:
        ReleaseDC(hWnd, hdcMem);
        DeleteObject(hbmMem);
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        ReleaseDC(hWnd, hdcMem);
        DeleteObject(hbmMem);
        HDC dc = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(dc);
        hbmMem = CreateCompatibleBitmap(dc, LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    case WM_KEYDOWN:
        if ((HIWORD(lParam) & KF_REPEAT) == KF_REPEAT) return 0;
        switch (wParam) {
        case 0x53:
            CameraPosition -= CameraForward;
            break;
        case 0x57:
            CameraPosition += CameraForward;
            break;
        case 0x41:
            CameraPosition -= CameraRight;
            break;
        case 0x44:
            CameraPosition += CameraRight;
            break;
        case 0x51:
            CameraPosition -= CameraUp;
            break;
        case 0x45:
            CameraPosition += CameraUp;
            break;
        default:
            return 0;
        }
        InitiateRedraw(hWnd);
        return 0;
    case WM_TIMER:
        if (wParam == UPDATE_TIMER) {
            Update(hWnd);
            InitiateRedraw(hWnd);
        }
        return 0;
    case WM_PAINT:
        RedrawScreen(hWnd);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int main()
{
    Vertices.push_back(Vector3(0, 2, 0)); //0

    Vertices.push_back(Vector3(-1, -1, -1)); //1
    Vertices.push_back(Vector3(1, -1, -1)); //2
    Vertices.push_back(Vector3(1, -1, 1)); //3
    Vertices.push_back(Vector3(-1, -1, 1)); //4

    Vertices.push_back(Vector3(-1, 1, -1)); //5
    Vertices.push_back(Vector3(1, 1, -1)); //6
    Vertices.push_back(Vector3(1, 1, 1)); //7
    Vertices.push_back(Vector3(-1, 1, 1)); //8

    Lines.push_back(std::make_pair(0, 5));
    Lines.push_back(std::make_pair(0, 6));
    Lines.push_back(std::make_pair(0, 7));
    Lines.push_back(std::make_pair(0, 8));

    Lines.push_back(std::make_pair(1, 5));
    Lines.push_back(std::make_pair(2, 6));
    Lines.push_back(std::make_pair(3, 7));
    Lines.push_back(std::make_pair(4, 8));

    Lines.push_back(std::make_pair(1, 2));
    Lines.push_back(std::make_pair(2, 3));
    Lines.push_back(std::make_pair(3, 4));
    Lines.push_back(std::make_pair(4, 1));

    Lines.push_back(std::make_pair(5, 6));
    Lines.push_back(std::make_pair(6, 7));
    Lines.push_back(std::make_pair(7, 8));
    Lines.push_back(std::make_pair(8, 5));



    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = TEXT("Main Window");
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (RegisterClass(&wc) == FALSE) std::cout << GetLastError();

    HWND hWnd = CreateWindowEx(
        0, wc.lpszClassName,
        TEXT("Vector Graphics"),
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        200, 200, 500, 500,
        nullptr, nullptr, wc.hInstance, nullptr
    );
    if (hWnd == nullptr) std::cout << GetLastError();

    SetTimer(hWnd, UPDATE_TIMER, 10, nullptr);


    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        DispatchMessage(&msg);
    }
}