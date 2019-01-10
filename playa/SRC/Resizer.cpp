/**************************************************************************************
 *                                                                                    *
 * This application contains code from OpenDivX and is released as a "Larger Work"    *
 * under that license. Consistant with that license, this application is released     *
 * under the GNU General Public License.                                              *
 *                                                                                    *
 * The OpenDivX license can be found at: http://www.projectmayo.com/opendivx/docs.php *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html                      *
 *                                                                                    *
 * Copyright (c) 2001 - Project Mayo                                                  *
 *                                                                                    *
 * Authors: Damien Chavarria <adrc at projectmayo.com>                                *
 *                                                                                    *
 **************************************************************************************/

#include "Resizer.h"

/*
 * The class used to draw
 * the resize frame...
 *
 */

aspect_ratio_factors_t aspectRatios[] = {

	{1,  1},
	{1,  1},
	{4,  3},
	{16, 9},
	{1,  1},
};


Resizer::Resizer() {

	this->lastPt.x = -1;
	this->lastPt.y = -1;
}

Resizer::~Resizer() {

}

void Resizer::Start(POINT *pt, DWORD originalWidth, DWORD originalHeight) {

	this->lastPt.x = -1;
	this->lastPt.y = -1;

	this->originalWidth  = originalWidth;
	this->originalHeight = originalHeight;
}

void Resizer::DrawFree(HWND hwnd, POINT *pt1) {

	POINT p, pt;
	RECT  rect;
	HPEN  pen, oldPen;
	HDC   dc;
	int   oldMode;

	pt.x = pt1->x;
	pt.y = pt1->y;

	GetClientRect(hwnd, &rect);
	
	p.x = rect.left;
	p.y = rect.top;

	ClientToScreen(hwnd, &p);
	dc = GetDC(NULL);

	pen    = CreatePen(PS_SOLID, 2, 0);
	oldPen = (HPEN) SelectObject(dc, pen);

	oldMode = SetROP2(dc, R2_NOT);

	/*
	 * Draw back if needed
	 */

	if(this->lastPt.x != -1 && this->lastPt.y != -1) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, this->lastPt.x, p.y + 2);
		LineTo(dc, this->lastPt.x, this->lastPt.y);
		LineTo(dc, p.x + 2, this->lastPt.y);
		LineTo(dc, p.x + 2, p.y + 2);
	}

	if(pt1->x > 32768 || pt1->y > 32768) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
	
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_HEIGHT);
		LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_HEIGHT);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_HEIGHT;

		goto end;
	}

	ClientToScreen(hwnd, &pt);

	/*
	 * Now check where we are
	 */


	if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH && (pt.y - p.y) >= DEFAULT_SKIN_HEIGHT) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, pt.x + 5, p.y + 2);
		LineTo(dc, pt.x + 5, pt.y + 5);
		LineTo(dc, p.x + 2,  pt.y + 5);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = pt.x + 5;
		this->lastPt.y = pt.y + 5;
	}
	else {

		if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH) {

			MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
			LineTo(dc, pt.x + 5, p.y + 2);
			LineTo(dc, pt.x + 5, p.y + DEFAULT_SKIN_HEIGHT);
			LineTo(dc, p.x + 2,  p.y + DEFAULT_SKIN_HEIGHT);
			LineTo(dc, p.x + 2, p.y + 2);
	
			this->lastPt.x = pt.x + 5;
			this->lastPt.y = p.y + DEFAULT_SKIN_HEIGHT;

		}
		else {

			if((pt.y - p.y) >= DEFAULT_SKIN_HEIGHT) {
			
				MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
				LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
				LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, pt.y + 5);
				LineTo(dc, p.x + 2, pt.y + 5);
				LineTo(dc, p.x + 2, p.y + 2);
	
				this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
				this->lastPt.y = pt.y + 5;

			}
			else {

				MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
				LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
				LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_HEIGHT);
				LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_HEIGHT);
				LineTo(dc, p.x + 2, p.y + 2);
	
				this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
				this->lastPt.y = p.y + DEFAULT_SKIN_HEIGHT;
			}
		}
	}

end:

	/*
	 * And puts everything back
	 */

	SetROP2(dc, oldMode);

	SelectObject(dc, oldPen);
			
	DeleteObject(pen);
	ReleaseDC(NULL, dc);
}

void Resizer::DrawOriginal(HWND hwnd, POINT *pt1)
{
	POINT p, pt;
	RECT  rect;
	HPEN  pen, oldPen;
	HDC   dc;
	int   oldMode;

	pt.x = pt1->x;
	pt.y = pt1->y;

	GetClientRect(hwnd, &rect);
	
	p.x = rect.left;
	p.y = rect.top;

	ClientToScreen(hwnd, &p);
	dc = GetDC(NULL);

	pen    = CreatePen(PS_SOLID, 2, 0);
	oldPen = (HPEN) SelectObject(dc, pen);

	oldMode = SetROP2(dc, R2_NOT);

	/*
	 * Draw back if needed
	 */

	if(this->lastPt.x != -1 && this->lastPt.y != -1) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, this->lastPt.x, p.y + 2);
		LineTo(dc, this->lastPt.x, this->lastPt.y);
		LineTo(dc, p.x + 2, this->lastPt.y);
		LineTo(dc, p.x + 2, p.y + 2);
	}

	if(pt1->x > 32768 || pt1->y > 32768) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
	
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22;

		goto endTV;
	}

	ClientToScreen(hwnd, &pt);

	/*
	 * Now check where we are
	 */


	if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, pt.x + 5, p.y + 2);
		LineTo(dc, pt.x + 5, p.y + (pt.x - p.x - 10)*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + (pt.x - p.x - 10)*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = pt.x + 5;
		this->lastPt.y = p.y + (pt.x - p.x - 10)*this->originalHeight/this->originalWidth + 115 + 22;
	}
	else {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*this->originalHeight/this->originalWidth + 115 + 22;
	}

endTV:

	/*
	 * And puts everything back
	 */

	SetROP2(dc, oldMode);

	SelectObject(dc, oldPen);
			
	DeleteObject(pen);
	ReleaseDC(NULL, dc);
}

void Resizer::DrawTV(HWND hwnd, POINT *pt1)
{
	POINT p, pt;
	RECT  rect;
	HPEN  pen, oldPen;
	HDC   dc;
	int   oldMode;

	pt.x = pt1->x;
	pt.y = pt1->y;

	GetClientRect(hwnd, &rect);
	
	p.x = rect.left;
	p.y = rect.top;

	ClientToScreen(hwnd, &p);
	dc = GetDC(NULL);

	pen    = CreatePen(PS_SOLID, 2, 0);
	oldPen = (HPEN) SelectObject(dc, pen);

	oldMode = SetROP2(dc, R2_NOT);

	/*
	 * Draw back if needed
	 */

	if(this->lastPt.x != -1 && this->lastPt.y != -1) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, this->lastPt.x, p.y + 2);
		LineTo(dc, this->lastPt.x, this->lastPt.y);
		LineTo(dc, p.x + 2, this->lastPt.y);
		LineTo(dc, p.x + 2, p.y + 2);
	}

	if(pt1->x > 32768 || pt1->y > 32768) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
	
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22;

		goto endTV;
	}

	ClientToScreen(hwnd, &pt);

	/*
	 * Now check where we are
	 */


	if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, pt.x + 5, p.y + 2);
		LineTo(dc, pt.x + 5, p.y + (pt.x - p.x - 10)*3/4 + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + (pt.x - p.x - 10)*3/4 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = pt.x + 5;
		this->lastPt.y = p.y + (pt.x - p.x - 10)*3/4 + 115 + 22;
	}
	else {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*3/4 + 115 + 22;
	}

endTV:

	/*
	 * And puts everything back
	 */

	SetROP2(dc, oldMode);

	SelectObject(dc, oldPen);
			
	DeleteObject(pen);
	ReleaseDC(NULL, dc);
}

void Resizer::DrawWide(HWND hwnd, POINT *pt1)
{
	POINT p, pt;
	RECT  rect;
	HPEN  pen, oldPen;
	HDC   dc;
	int   oldMode;

	pt.x = pt1->x;
	pt.y = pt1->y;

	GetClientRect(hwnd, &rect);
	
	p.x = rect.left;
	p.y = rect.top;

	ClientToScreen(hwnd, &p);
	dc = GetDC(NULL);

	pen    = CreatePen(PS_SOLID, 2, 0);
	oldPen = (HPEN) SelectObject(dc, pen);

	oldMode = SetROP2(dc, R2_NOT);

	/*
	 * Draw back if needed
	 */

	if(this->lastPt.x != -1 && this->lastPt.y != -1) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, this->lastPt.x, p.y + 2);
		LineTo(dc, this->lastPt.x, this->lastPt.y);
		LineTo(dc, p.x + 2, this->lastPt.y);
		LineTo(dc, p.x + 2, p.y + 2);
	}

	if(pt1->x > 32768 || pt1->y > 32768) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
	
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22;

		goto endWide;
	}

	ClientToScreen(hwnd, &pt);

	/*
	 * Now check where we are
	 */


	if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, pt.x + 5, p.y + 2);
		LineTo(dc, pt.x + 5, p.y + (pt.x - p.x - 10)*9/16 + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + (pt.x - p.x - 10)*9/16 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = pt.x + 5;
		this->lastPt.y = p.y + (pt.x - p.x - 10)*9/16 + 115 + 22;
	}
	else {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*9/16 + 115 + 22;
	}

endWide:

	/*
	 * And puts everything back
	 */

	SetROP2(dc, oldMode);

	SelectObject(dc, oldPen);
			
	DeleteObject(pen);
	ReleaseDC(NULL, dc);
}

void Resizer::DrawCustom(HWND hwnd, POINT *pt1)
{
	POINT p, pt;
	RECT  rect;
	HPEN  pen, oldPen;
	HDC   dc;
	int   oldMode;

	pt.x = pt1->x;
	pt.y = pt1->y;

	GetClientRect(hwnd, &rect);
	
	p.x = rect.left;
	p.y = rect.top;

	ClientToScreen(hwnd, &p);
	dc = GetDC(NULL);

	pen    = CreatePen(PS_SOLID, 2, 0);
	oldPen = (HPEN) SelectObject(dc, pen);

	oldMode = SetROP2(dc, R2_NOT);

	/*
	 * Draw back if needed
	 */

	if(this->lastPt.x != -1 && this->lastPt.y != -1) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, this->lastPt.x, p.y + 2);
		LineTo(dc, this->lastPt.x, this->lastPt.y);
		LineTo(dc, p.x + 2, this->lastPt.y);
		LineTo(dc, p.x + 2, p.y + 2);
	}

	if(pt1->x > 32768 || pt1->y > 32768) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
	
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2, p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22;

		goto endCustom;
	}

	ClientToScreen(hwnd, &pt);

	/*
	 * Now check where we are
	 */


	if((pt.x - p.x) >= DEFAULT_SKIN_WIDTH) {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, pt.x + 5, p.y + 2);
		LineTo(dc, pt.x + 5, p.y + (pt.x - p.x - 10)*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + (pt.x - p.x - 10)*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = pt.x + 5;
		this->lastPt.y = p.y + (pt.x - p.x - 10)*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22;
	}
	else {

		MoveToEx(dc, p.x + 2, p.y + 2, NULL);
		
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + 2);
		LineTo(dc, p.x + DEFAULT_SKIN_WIDTH, p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2,  p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22);
		LineTo(dc, p.x + 2, p.y + 2);
	
		this->lastPt.x = p.x + DEFAULT_SKIN_WIDTH;
		this->lastPt.y = p.y + DEFAULT_SKIN_WIDTH*aspectRatios[ASPECT_RATIO_CUSTOM].yFactor/aspectRatios[ASPECT_RATIO_CUSTOM].xFactor + 115 + 22;
	}

endCustom:

	/*
	 * And puts everything back
	 */

	SetROP2(dc, oldMode);

	SelectObject(dc, oldPen);
			
	DeleteObject(pen);
	ReleaseDC(NULL, dc);
}

void Resizer::Draw(HWND hwnd, POINT *pt1, aspect_ratio_t aspectRatio) {

	switch(aspectRatio) {

	case ASPECT_RATIO_FREE:
		this->DrawFree(hwnd, pt1);
		break;

	case ASPECT_RATIO_ORIGINAL:
		this->DrawOriginal(hwnd, pt1);
		break;

	case ASPECT_RATIO_TV:
		this->DrawTV(hwnd, pt1);
		break;

	case ASPECT_RATIO_WIDE:
		this->DrawWide(hwnd, pt1);
		break;

	case ASPECT_RATIO_CUSTOM:
		this->DrawCustom(hwnd, pt1);
		break;
	}
}

POINT *Resizer::GetLastPoint()
{
	return &this->lastPt;
}


void Resizer::Stop() {

}
