#include <windows.h>
#include "Fill.h"

void fill();

double roundVal(double x)
{
  int ix;
  if (x >= 0) 
    ix = (int)(x + 0.5);
  else
    ix = (int)(x - 0.5);
  return (double)(ix);
}

void swap(int &x, int &y)
{ 
  int tmp;
  tmp = x;
  x = y;
  y = tmp;
}

void setupMenus(HWND hwnd)
{
  HMENU hmenu, hsubmenu;
  hmenu=GetMenu(hwnd);
  hsubmenu=GetSubMenu(hmenu, 0);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      // enable 'Draw Polygon', disable 'Filling' menu
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAW_MODE :
    case DRAW_MODE_1 :
    case DRAW_MODE_2 :
    case FILL_MODE :
    case FILLED_MODE :
      // disable 'Draw Polygon', 'Filling' menu 
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAWN_MODE :
      // enable 'Filling' menus, disable 'Draw Polygon' menu
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_ENABLED);
      break;
  }
}

void performFilling(HWND hwnd)
{
  setDrawMode(FILL_MODE, hwnd);
  SelectObject(gDrawData.hdcMem, gDrawData.hFillPen);
  fill();
  reDraw(hwnd);
  setDrawMode(FILLED_MODE, hwnd);
}

void processCommand(int cmd, HWND hwnd)
{
  switch(cmd)
  {
    case ID_FILL:
      performFilling(hwnd);
      break;
    default:
      processCommonCommand(cmd, hwnd);
      break;
  }
}

bool adjustHorizontallyForBorderPixel(int& x, int y)
{
  if (GetPixel(gDrawData.hdcMem, x, y) != CLR_BOUNDARY)  
  {
    // if (x,y) is not R(border), R could be on either left or right
    if (GetPixel(gDrawData.hdcMem, x-1, y) == CLR_BOUNDARY)  
      x--;
    else if (GetPixel(gDrawData.hdcMem, x+1, y) == CLR_BOUNDARY)  
      x++;
    else
    {
      // neither self or left or right neighbor contains border, skip
      return false; 
    }
  }
  return true;   
}   

bool adjustForDrawingLine(int& x1, int&x2, int y)
{
  // make adjustments for special cases
  if (adjustHorizontallyForBorderPixel(x1, y) == false)
    return false;  
  // start with R...
  if (adjustHorizontallyForBorderPixel(x2, y) == false)
    return false;  
  // end with R...
  // now we have R...R    
  if (x2>x1)
  {
    // skip RRR.. to move to R...
    while ((GetPixel(gDrawData.hdcMem, x1, y)) == CLR_BOUNDARY)
    {
      x1++;
    }
    while ((GetPixel(gDrawData.hdcMem, x2, y)) == CLR_BOUNDARY)
    {
      x2--;
    }
    x2++;
    if (x2>x1)
    {
      // x1 is set at W (not R), x2 set at R
      // Windows API LineTo draws upto not including x2
      return true; 
    }
  }
  return false; // no point of drawing line
}

void drawLine(int x1, int x2, int y)
{
  // draw fill-line from (x1,y) to (x2-1,y)
  if (adjustForDrawingLine(x1, x2, y))
  {
    MoveToEx (gDrawData.hdcMem, x1, y, NULL);
    LineTo (gDrawData.hdcMem, x2, y);
  }
}

int edge_cmp(const void *lvp, const void *rvp)
{
  /* convert from void pointers to structure pointers */
  const EDGE_ENTRY *lp = (const EDGE_ENTRY *)lvp;
  const EDGE_ENTRY *rp = (const EDGE_ENTRY *)rvp;

  /* if the y minimum values are different, compare on minimum y */
  if (lp->yMin != rp->yMin)
    return lp->yMin - rp->yMin;

  /* otherwise, if the current x values are different, 
     compare on current x */
  return ((int)(roundVal(lp->x))) - ((int)(roundVal(rp->x)));
}

int doubleArea(int x1, int y1, int x2, int y2, int x3, int y3) 
{ 
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))); 
} 
  
/* A function to check whether point P(x, y) lies inside the triangle formed  
   by A(x1, y1), B(x2, y2) and C(x3, y3) */ 
bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y) 
{    
   /* Calculate area of triangle ABC */
   int A = doubleArea (x1, y1, x2, y2, x3, y3); 
  
   /* Calculate area of triangle PBC */   
   int A1 = doubleArea (x, y, x2, y2, x3, y3); 
  
   /* Calculate area of triangle PAC */   
   int A2 = doubleArea (x1, y1, x, y, x3, y3); 
  
   /* Calculate area of triangle PAB */    
   int A3 = doubleArea (x1, y1, x2, y2, x, y); 
    
   /* Check if sum of A1, A2 and A3 is same as A */ 
   return (A == A1 + A2 + A3);
}

int min(int a, int b){
	if (a<b)
		return a;
	return b;
}

int max(int a, int b){
	if (a>b)
		return a;
	return b;
}

void fill()
{
	bool insideTriangle[3];
	
	int X_MIN = gDrawData.cornerPts[0].x;
	int X_MAX = gDrawData.cornerPts[0].x;
	int Y_MIN = gDrawData.cornerPts[0].y;
	int Y_MAX = gDrawData.cornerPts[0].y;

	for (int i = 1; i < 9; i++){
		X_MIN = min(X_MIN, gDrawData.cornerPts[i].x);
		X_MAX = max(X_MAX, gDrawData.cornerPts[i].x);
		Y_MIN = min(Y_MIN, gDrawData.cornerPts[i].y);
		Y_MAX = max(Y_MAX, gDrawData.cornerPts[i].y);
	}
		
	for(int y = Y_MIN; y <= Y_MAX; y++){	
		for (int x = X_MIN; x <= X_MAX; x++){
			COLORREF color = GetPixel(gDrawData.hdcMem, x, y);
			// check if it is boundary
			// update the gDrawData
			for(int i=0;i<3;i++){
				int X1 = gDrawData.cornerPts[3*i+0].x;
				int X2 = gDrawData.cornerPts[3*i+1].x;
				int X3 = gDrawData.cornerPts[3*i+2].x;
				int Y1 = gDrawData.cornerPts[3*i+0].y;
				int Y2 = gDrawData.cornerPts[3*i+1].y;
				int Y3 = gDrawData.cornerPts[3*i+2].y;
				if (isInside(X1, Y1, X2, Y2, X3, Y3, x, y))		insideTriangle[i] = true;
				else insideTriangle[i] = false;
			}

			if(color != RGB(0,0,0)){
				if (insideTriangle[0] && insideTriangle[1] && insideTriangle[2])	SetPixel(gDrawData.hdcMem, x, y, RGB(255, 0, 0));
				else if (insideTriangle[0] && insideTriangle[1] )	SetPixel(gDrawData.hdcMem, x, y, RGB(0, 0, 255));
				else if (insideTriangle[1] && insideTriangle[2] )	SetPixel(gDrawData.hdcMem, x, y, RGB(0, 255, 0));
				else if (insideTriangle[0] && insideTriangle[2] )	SetPixel(gDrawData.hdcMem, x, y, RGB(255, 255, 0));
				else if (insideTriangle[0] || insideTriangle[1] || insideTriangle[2])	SetPixel(gDrawData.hdcMem, x, y, RGB(0, 255, 255));								
			}
		}	
	}
}
