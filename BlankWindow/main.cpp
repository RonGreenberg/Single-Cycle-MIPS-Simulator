/*
Beginning DirectX 11 Game Programming
By Allen Sherrod and Wendy Jones
*/


#include <Windows.h>
#include <Gdiplus.h>
#include <stdio.h>
#include "udp_listen.h"

#define SCALE 2

HBRUSH hOrange = CreateSolidBrush(RGB(255,180,0));
HBRUSH hRed = CreateSolidBrush(RGB(255,0,0));
HBRUSH hYellow = CreateSolidBrush(RGB(255,255,0));
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
int scale = SCALE;


int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow )
{
  UNREFERENCED_PARAMETER( prevInstance );
  UNREFERENCED_PARAMETER( cmdLine );

  WNDCLASSEX wndClass = { 0 };
  wndClass.cbSize = sizeof( WNDCLASSEX ) ;
  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = WndProc;
  wndClass.hInstance = hInstance;
  wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
  wndClass.hbrBackground =(HBRUSH)GetStockObject(BLACK_BRUSH);
  //wndClass.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
  wndClass.lpszMenuName = NULL;
  wndClass.lpszClassName = L"VirtualScreenClass";

  if( !RegisterClassEx( &wndClass ) )
    return -1;

  RECT rc = { 0, 0, 511, 511 };
  AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

  HWND hwnd = CreateWindowA( "VirtualScreenClass", "UDP Virtual Screen",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
    rc.bottom - rc.top, NULL, NULL, hInstance, NULL );

  if( !hwnd ) {
    return -1;
  }

  UDP_init(); // initializing udp listener (server)
  ShowWindow( hwnd, cmdShow ); // showing window


  // Demo Initialize
  MSG msg = { 0 };
  RECT my_rect;
  HDC hDC = GetDC(hwnd);
  unsigned char *buf = NULL;
  int res;

  while( msg.message != WM_QUIT ) {
    if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    } else {
       res = UDP_get_msg_non_blocking(&buf); // checking if a UDP message was received
       if (res == 0) { // UDP message received
         HBRUSH color = CreateSolidBrush(RGB((unsigned int)buf[0], (unsigned int)buf[1], (unsigned int)buf[2]));
         SetRect(&my_rect, (unsigned int)buf[3] * scale, (unsigned int)buf[4]* scale, (unsigned int)buf[5]* scale, (unsigned int)buf[6]* scale);
         FillRect(hDC, &my_rect, color);
         DeleteObject(color);
       }
    }
  }

  return static_cast<int>( msg.wParam );
}


LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  PAINTSTRUCT paintStruct;
  HDC hDC;

  switch( message ) {
  case WM_PAINT:
    hDC = BeginPaint( hwnd, &paintStruct );
    EndPaint( hwnd, &paintStruct );
    break;

  case WM_DESTROY:
    PostQuitMessage( 0 );
    break;

  default:
    return DefWindowProc( hwnd, message, wParam, lParam );
  }

  return 0;
}
