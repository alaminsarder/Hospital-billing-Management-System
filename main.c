#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

#define ID_CALCULATE 1
#define ID_CLEAR     2

HWND hName, hAge, hDays, hRoom, hDoctor, hLab, hMedicine, hOutput;
HFONT hFont, hTitleFont;
sqlite3 *db;

// ডাটাবেসে ডেটা সেভ করার ফাংশন
void save_to_db(const char* name, int age, int days, float total) {
    char *err_msg = 0;
    char sql[512];
    sprintf(sql, "INSERT INTO patients (name, age, days, total_bill) VALUES('%s', %d, %d, %.2f);", name, age, days, total);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        MessageBox(NULL, "Database Error!", "Error", MB_ICONERROR);
        sqlite3_free(err_msg);
    }
}

float getFloat(HWND hEdit) {
    char buffer[100];
    GetWindowText(hEdit, buffer, 100);
    return atof(buffer);
}

// UI ডিজাইন সুন্দর করার জন্য লজিক
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            // ফন্ট তৈরি
            hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // টাইটেল
            HWND hTitle = CreateWindow("static", "Hospital Billing System", WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 10, 380, 30, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            // Labels and Inputs (সাজানো লেআউট)
            int labelX = 30, editX = 160, yStart = 60, yGap = 35, width = 180;

            char *labels[] = {"Patient Name:", "Age:", "Days Admitted:", "Room Cost/Day:", "Doctor Fee:", "Lab/Medicine Fee:"};
            HWND *handles[] = {&hName, &hAge, &hDays, &hRoom, &hDoctor, &hMedicine};

            for(int i = 0; i < 6; i++) {
                HWND hLabl = CreateWindow("static", labels[i], WS_VISIBLE | WS_CHILD, labelX, yStart + (i * yGap), 120, 20, hwnd, NULL, NULL, NULL);
                SendMessage(hLabl, WM_SETFONT, (WPARAM)hFont, TRUE);
                
                *handles[i] = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, editX, yStart + (i * yGap), width, 25, hwnd, NULL, NULL, NULL);
                SendMessage(*handles[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            // Buttons
            HWND btnCalc = CreateWindow("button", "Calculate & Save", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 30, 280, 150, 40, hwnd, (HMENU)ID_CALCULATE, NULL, NULL);
            HWND btnClear = CreateWindow("button", "Clear All", WS_VISIBLE | WS_CHILD, 200, 280, 140, 40, hwnd, (HMENU)ID_CLEAR, NULL, NULL);
            
            SendMessage(btnCalc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(btnClear, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Output Display
            hOutput = CreateWindow("static", "Total Bill: $0.00", WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER, 30, 340, 310, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 50, 100)); // নীলচে টেক্সট কালার
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_CALCULATE) {
                char name[100];
                GetWindowText(hName, name, 100);
                
                if(strlen(name) == 0) {
                    MessageBox(hwnd, "Please enter patient name!", "Warning", MB_ICONWARNING);
                    break;
                }

                int age = (int)getFloat(hAge);
                int days = (int)getFloat(hDays);
                float room = getFloat(hRoom) * days;
                float total = room + getFloat(hDoctor) + getFloat(hMedicine);

                char result[100];
                sprintf(result, "Total Bill: %.2f", total);
                SetWindowText(hOutput, result);

                save_to_db(name, age, days, total);
                MessageBox(hwnd, "Bill Calculated and Saved to Database!", "Success", MB_ICONINFORMATION);
            }

            if (LOWORD(wParam) == ID_CLEAR) {
                SetWindowText(hName, ""); SetWindowText(hAge, "");
                SetWindowText(hDays, ""); SetWindowText(hRoom, "");
                SetWindowText(hDoctor, ""); SetWindowText(hMedicine, "");
                SetWindowText(hOutput, "Total Bill: $0.00");
            }
            break;

        case WM_DESTROY:
            DeleteObject(hFont);
            DeleteObject(hTitleFont);
            sqlite3_close(db);
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR args, int nCmdShow) {
    sqlite3_open("hospital_billing.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS patients (id INTEGER PRIMARY KEY, name TEXT, age INTEGER, days INTEGER, total_bill REAL);", 0, 0, 0);

    WNDCLASS wc = {0};
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // সাদা ব্যাকগ্রাউন্ড
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = "HospitalBilling";
    wc.lpfnWndProc = WindowProc;

    RegisterClass(&wc);
    HWND hwnd = CreateWindow("HospitalBilling", "Hospital Management System v1.0", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             450, 150, 400, 450, NULL, NULL, hInst, NULL);

    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}