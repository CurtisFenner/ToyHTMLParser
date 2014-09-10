#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdio.h>


//gcc wixel.c -mwindows

const char g_szClassName[] = "myWindowClass";

void Line(HDC hdc,int x,int y,int a,int b) {
	MoveToEx(hdc,x,y,NULL);
	LineTo(hdc,a,b);
}

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			// Start up
			break;
		case WM_PAINT: {
				BITMAP bm;
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				// use BitBlt? and SRCCOPY
				HPEN pen = CreatePen(PS_SOLID | PS_COSMETIC,1,0x00ffaa88);
				SelectObject(hdc, pen);
				MoveToEx(hdc,20,40,NULL);
				LineTo(hdc,21,40);
				RECT where;
				where.left = 20;
				where.top = 20;
				where.right = 0;
				where.bottom = 0;
				DrawText(hdc,"Hello there person!",-1,&where,DT_LEFT | DT_NOCLIP);
				EndPaint(hwnd, &ps);
				break;
			}
		case WM_LBUTTONDOWN:
			//Left mouse. also has R and M mouse
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	if (nCmdShow == 1) {
		HWND hWnd = GetConsoleWindow();
		ShowWindow( hWnd, SW_MINIMIZE );  //won't hide the window without SW_MINIMIZE
		ShowWindow( hWnd, SW_HIDE );
	}

	WNDCLASSEX wc; // A class (type) of Window to make new ones
	HWND hwnd;
	MSG Msg;

	//Step 1: Registering the Window Class
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0; //The style of the structure (doesn't mean much)
	wc.lpfnWndProc   = WndProc; //Pointer to window procedure
	wc.cbClsExtra    = 0; //amount of extra data allocated
	wc.cbWndExtra    = 0; //amount extra PER WINDOW
	wc.hInstance     = hInstance; //Handle to application
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION); // alt tab icon
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW); //cursor when hoving on window
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // background brush to set color
	wc.lpszMenuName  = NULL; //name of a menu resource to use for the windows in this class
	wc.lpszClassName = g_szClassName; //name to identify with
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION); //top left window icon

	if (!RegisterClassEx(&wc)) {
		//MessageBox(NULL, "Window Registration Failed!", "Error!",MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"Wixel",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL) {
		return 0;
	}
	ShowWindow(hwnd, 10);
	//ShowWindow(hwnd,SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	// Step 3: The Message Loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg); // makes it nicer to play with, apparently
		DispatchMessage(&Msg); // sends it to the appropriate Window procedure
	}
	return Msg.wParam;
}