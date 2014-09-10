

#include <math.h>
#include <conio.h>
#include <windows.h>
#include <Wincon.h>


int main(){
	HANDLE outcon = GetStdHandle(STD_OUTPUT_HANDLE);//you don't have to call this function every time

	CONSOLE_FONT_INFOEX font;//CONSOLE_FONT_INFOEX is defined in some windows header
	GetCurrentConsoleFontEx(outcon, false, &font);//PCONSOLE_FONT_INFOEX is the same as CONSOLE_FONT_INFOEX*
	font.dwFontSize.X = 7;
	font.dwFontSize.Y = 12;
	SetCurrentConsoleFontEx(outcon, false, &font);
	SetConsoleTextAttribute(outcon, 0x0C);
	printf("HELLO");
	return 0;
}