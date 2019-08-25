#include <math.h>
#include <windows.h>
#include "Ellipse.h"

DRAWING_DATA gDrawData; // global data
void drawCurve();

void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
  Ellipse(gDrawData.hdcMem,x-2,y-2,x+2,y+2);
}

void setDrawMode(MODE mode, HWND hwnd)
{
  gDrawData.drawMode = mode;
}

void createMemoryBitmap(HDC hdc)
{
  gDrawData.hdcMem = CreateCompatibleDC(hdc);
  gDrawData.hbmp = CreateCompatibleBitmap(hdc, 
    gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

  gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
  gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);

  createMemoryBitmap(hdc);
  setDrawMode(DRAW_MODE, hwnd);
}

void cleanup()
{
  DeleteDC(gDrawData.hdcMem);
}

void reset(HWND hwnd)
{
  gDrawData.centre.x = gDrawData.centre.y = 0;

  gDrawData.drawMode = DRAW_MODE;

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);

  reDraw(hwnd);
}

void plot_sympoint(int ex, int ey, COLORREF clr)
{
  int cx = gDrawData.centre.x;
  int cy = gDrawData.centre.y;

  SetPixel(gDrawData.hdcMem, ex+cx,cy-ey, clr);
  //SetPixel(gDrawData.hdcMem, -ex+cx,cy-ey, clr);
  SetPixel(gDrawData.hdcMem, -ex+cx,cy+ey, clr);
  //SetPixel(gDrawData.hdcMem, ex+cx,cy+ey, clr);
}

void addPoint(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_MODE:
      /* the coordinates of the centre is stored 
         and the curve is drawn */
      SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
      drawPoint(x,y);
      gDrawData.centre.x = x;
      gDrawData.centre.y = y;
      drawCurve();
      setDrawMode(DRAWN_MODE, hwnd);
      reDraw(hwnd);
      break;
    case DRAWN_MODE:
      MessageBox(hwnd,
       "Curve already drawn, now you can clear the area", 
          "Warning",MB_OK);
      break;
    default:
      break;
  }
}

void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx, 
    gDrawData.maxBoundary.cy, gDrawData.hdcMem, 
    0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  addPoint(hwnd,x,y);
}

void processCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      setDrawMode(DRAW_MODE, hwnd);
      break;
    case ID_EXIT:
        response=MessageBox(hwnd,
          "Quit the program?", "EXIT", 
          MB_YESNO);
        if(response==IDYES) 
            PostQuitMessage(0);
        break;
    default:
      break;
  }
}

void drawCurve()
{
  int x=0,y=0;
  double d=5.0/12.0;
  plot_sympoint(x,y, RGB(0,0,0));
  while(x<2)
  {
    /* in region 1*/
    if(d<0)
    {
      /*choose NE*/
      d=d+((float)(5-3*x*x-9*x)/12.0);
      x=x+1;
      y=y+1;
    }
    else                           
    {
      /*choose E*/
      d=d+((float)(-7-3*x*x-9*x)/12.0);
      x=x+1;
    }
    plot_sympoint(x,y, RGB(0,0,0));
  }
  d=(y+1)-((x+0.5)*(x+0.5)*(x+0.5)/12);
  while(x<=10)
  {
    /*in region 2*/
    if(d<0)
    {
      /*choose N*/
      d=d+1.0;
      y=y+1;
    }
    else
    {
      /*choose NE*/
      d=d+((float)(35-12*x*x-24*x)/48.0);
      y=y+1;
      x=x+1;
    }
    plot_sympoint(x,y, RGB(0,0,0));
  }
}

LRESULT CALLBACK WindowF(HWND hwnd,UINT message,WPARAM wParam,
                         LPARAM lParam) 
{
  HDC hdc;
  PAINTSTRUCT ps;
  int x,y;

  switch(message)
  {
    case WM_CREATE:
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      ReleaseDC(hwnd, hdc);
      break;
    
    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;

    case WM_LBUTTONDOWN:
      x = LOWORD(lParam);
      y = HIWORD(lParam);
      processLeftButtonDown(hwnd, x,y);
      break;

    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      drawImage(hdc);
      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      cleanup();
      PostQuitMessage(0);
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}
