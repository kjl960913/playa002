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


#include "Skin.h"

/*
 * Class Functions
 *
 */

/*
 * Constructor : Loads the default skin
 *
 */


Skin::Skin(HINSTANCE hInstance, HWND hwnd)
{

	this->LoadDefault(hInstance, hwnd);
	
	if(this->skin == NULL ||this->controls == NULL) {
		MessageBox(NULL, "Cannot load default skin!", "Error", MB_OK);
	}

	/*
 	 * Default values for the rects
	 */

	this->menuButton.left   = 2;
	this->menuButton.right  = 14;
	this->menuButton.top    = 3;
	this->menuButton.bottom = 14;

	this->minimizeButton.left   = 266;
	this->minimizeButton.right  = 278;
	this->minimizeButton.top    = 3;
	this->minimizeButton.bottom = 14;

	this->closeButton.left   = 283;
	this->closeButton.right  = 295;
	this->closeButton.top    = 3;
	this->closeButton.bottom = 14;

	this->rewindButton.left   = 15;
	this->rewindButton.right  = 15+29;
	this->rewindButton.top    = 343;
	this->rewindButton.bottom = 360;

	this->stopButton.left   = 52;
	this->stopButton.right  = 52+29;
	this->stopButton.top    = 343;
	this->stopButton.bottom = 360;

	this->playButton.left   = 89;
	this->playButton.right  = 89+29;
	this->playButton.top    = 343;
	this->playButton.bottom = 360;

	this->pauseButton.left   = 126;
	this->pauseButton.right  = 126+29;
	this->pauseButton.top    = 343;
	this->pauseButton.bottom = 360;

	this->forwardButton.left   = 263;
	this->forwardButton.right  = 300;
	this->forwardButton.top    = 343;
	this->forwardButton.bottom = 360;

	this->loadButton.left   = 236;
	this->loadButton.right  = 263+44;
	this->loadButton.top    = 343;
	this->loadButton.bottom = 360;

	this->progressBar.left   = 12;
	this->progressBar.right  = 281;
	this->progressBar.top    = 325;
	this->progressBar.bottom = 332;

	this->volumeBar.left   = 212;
	this->volumeBar.right  = 273;
	this->volumeBar.top    = 296;
	this->volumeBar.bottom = 304;

	this->progressCursor.left   = 15;
	this->progressCursor.right  = 43;
	this->progressCursor.top    = 321;
	this->progressCursor.bottom = 336;

	this->volumeCursor.left   = 250;
	this->volumeCursor.right  = 270;
	this->volumeCursor.top    = 293;
	this->volumeCursor.bottom = 307;
	
	this->volume   = 100;
	this->progress = 0.0;

	this->compactMode = 0;
}

/*
 * Destructor
 *
 */

Skin::~Skin()
{
	DeleteObject((HGDIOBJ) this->skin); 

	DeleteObject((HGDIOBJ) this->controls); 
	DeleteObject((HGDIOBJ) this->bottom); 
	DeleteObject((HGDIOBJ) this->logo); 
}

/*
 * Returns TRUE if the point is
 * inside the given rect...
 *
 */

BOOL Skin::isInsideRect(int x, int y, RECT rect)
{
	return (x > rect.left && x < rect.right && y > rect.top && y < rect.bottom);
}

/*
 * loads the Default Skin
 *
 */

int Skin::LoadDefault(HINSTANCE hInstance, HWND hwnd) {

	HDC   dc, sdc, ddc;
	DWORD i;

	this->skin     = LoadBitmap(hInstance, MAKEINTRESOURCE(ID_SKIN)); 
	this->controls = LoadBitmap(hInstance, MAKEINTRESOURCE(ID_CONTROLS));
	this->logo     = LoadBitmap(hInstance, MAKEINTRESOURCE(ID_LOGO));

	dc = GetDC(hwnd);

	this->bottom = CreateCompatibleBitmap(dc, 256, 115);

	sdc = CreateCompatibleDC(dc);
	SelectObject(sdc, this->skin);

	ddc = CreateCompatibleDC(dc);
	SelectObject(ddc, this->bottom);


	for(i=0; i < 256; i++) {

		BitBlt(ddc, i, 0, 1, 115, sdc, 200, 260, SRCCOPY);
	}

	ReleaseDC(hwnd, dc);
	DeleteDC(sdc);
	DeleteDC(ddc);

	this->skinColor = 0;

	return 1;
}


/*
 * Loads a new skin
 *
 */

int Skin::Load(char *directory, HWND hwnd) {

	HDC   dc, sdc, ddc;
	DWORD i;

	if(directory != NULL) {

		char *mainc, *logoc, *controlsc;

		mainc     = (char *) new char[strlen(directory) + 10];
		strcpy(mainc, directory);

		logoc     = (char *) new char[strlen(directory) + 10];
		strcpy(logoc, directory);
		
		controlsc = (char *) new char[strlen(directory) + 20];
		strcpy(controlsc, directory);

		if(this->skin)
			DeleteObject((HGDIOBJ) this->skin);
		
		if(this->controls)
			DeleteObject((HGDIOBJ) this->controls);

		if(this->logo)
			DeleteObject((HGDIOBJ) this->logo);

		this->skin =     (HBITMAP) LoadImage(NULL, strcat(mainc, "\\main.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		this->logo =     (HBITMAP) LoadImage(NULL, strcat(logoc, "\\logo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		this->controls = (HBITMAP) LoadImage(NULL, strcat(controlsc, "\\controls.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		free(mainc);
		free(logoc);
		free(controlsc);

		if(this->skin == 0 || this->logo == 0 || this->controls == 0) {

			this->LoadDefault(GetModuleHandle(NULL), hwnd);
		}
		else {

			/*
			 * Do some cache
			 *
			 */

			dc = GetDC(hwnd);

			sdc = CreateCompatibleDC(dc);
			SelectObject(sdc, this->skin);

			ddc = CreateCompatibleDC(dc);
			SelectObject(ddc, this->bottom);


			for(i=0; i < 256; i++) {

				BitBlt(ddc, i, 0, 1, 115, sdc, 200, 260, SRCCOPY);
			}

			ReleaseDC(hwnd, dc);
			DeleteDC(sdc);
			DeleteDC(ddc);

			/*
			 * Now load the background color from the config.txt file
			 */

			SkinFile *skinFile = new SkinFile(directory);

			this->skinColor = skinFile->getColor("background");

			skinFile->Close();	
		}
	}

	return 1;
}

/*
 * Return Default Width
 *
 */

int Skin::GetDefaultWidth()
{
	return DEFAULT_SKIN_WIDTH;	
}

/*
 * Return Default Width
 *
 */


int Skin::GetDefaultHeight()
{
	return DEFAULT_SKIN_HEIGHT;	
}

/*
 * Set volume and Progress
 * (for display only!)
 *
 */

int Skin::SetVolume(HWND hwnd, int new_volume)
{
	int          sizeX, sizeY;
	RECT         clientRect;

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;

	this->volume = new_volume;

	this->volumeCursor.left   = 215 + (sizeX - DEFAULT_SKIN_WIDTH) + 35*this->volume/100;
	this->volumeCursor.right  = 235 + (sizeX - DEFAULT_SKIN_WIDTH) + 35*this->volume/100;
	this->volumeCursor.top    = 293 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->volumeCursor.bottom = 307 + (sizeY - DEFAULT_SKIN_HEIGHT);

	return 1;
}

int Skin::SetProgress(HWND hwnd, double new_progress)
{
	int          sizeX, sizeY;
	RECT         clientRect;

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;

	this->progress = new_progress;

	this->progressCursor.left   =  15 + (int)((double)(sizeX - DEFAULT_SKIN_WIDTH + 263 - 28)*this->progress)/100;
	this->progressCursor.right  =  43 + (int)((double)(sizeX - DEFAULT_SKIN_WIDTH + 263 - 28)*this->progress)/100;
	this->progressCursor.top    =  321 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->progressCursor.bottom =  336 + (sizeY - DEFAULT_SKIN_HEIGHT);

	return 1;
}

int Skin::SetVolumeCursorX(HWND hwnd, int x)
{
	int  the_volume;
	int  sizeX, sizeY;
	RECT clientRect;
	HDC          hdc, bdc;
	
	hdc = GetDC(hwnd);
	bdc = CreateCompatibleDC(hdc);
 
	if(hdc == NULL || bdc == NULL) {

		return 0;
	}

	SelectObject(bdc, this->skin);

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;
	
	if(x < volumeBar.left)
		the_volume = 0;
	else {
		if(x > volumeBar.right)
			the_volume = 100;
		else {

			the_volume = (x - volumeBar.left)*100/(volumeBar.right - volumeBar.left);
		}
	}

	SetVolume(hwnd, the_volume);

	BitBlt(hdc, volumeBar.left, volumeBar.top - 5, (volumeBar.right - volumeBar.left), (volumeBar.bottom - volumeBar.top) + 10, 
		   bdc, 212, 291, SRCCOPY);

	SelectObject(bdc, this->controls);

	BitBlt(hdc, volumeCursor.left, volumeCursor.top, (volumeCursor.right - volumeCursor.left), (volumeCursor.bottom - volumeCursor.top), 
		   bdc, 32, 45, SRCCOPY);

	DeleteDC(bdc);
	ReleaseDC(hwnd, hdc);

	return 1;
}

int Skin::SetProgressCursorX(HWND hwnd, int x)
{
	int  the_progress;
	int  sizeX, sizeY, i;
	RECT clientRect;
	HDC  hdc, bdc;
	
	hdc = GetDC(hwnd);
	bdc = CreateCompatibleDC(hdc);
 
	if(hdc == NULL || bdc == NULL) {

		return 0;
	}

	SelectObject(bdc, this->skin);

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;
	
	if(x < progressBar.left)
		the_progress = 0;
	else {
		if(x > progressBar.right)
			the_progress = 100;
		else {

			the_progress = (x - progressBar.left)*100/(progressBar.right - progressBar.left);
		}
	}

	SetProgress(hwnd, the_progress);

	BitBlt(hdc, 0, 320 + (sizeY - DEFAULT_SKIN_HEIGHT), 200, 20, bdc, 0, 320, SRCCOPY);

	for(i=0; i < (sizeX - DEFAULT_SKIN_WIDTH); i++) {

		BitBlt(hdc, 200 + i, 320 + (sizeY - DEFAULT_SKIN_HEIGHT), 1, 20, bdc, 200, 320, SRCCOPY);
	}

	BitBlt(hdc, 200 + (sizeX - DEFAULT_SKIN_WIDTH), 320 + (sizeY - DEFAULT_SKIN_HEIGHT), 100, 20, bdc, 200, 320, SRCCOPY);


	SelectObject(bdc, this->controls);

	BitBlt(hdc, progressCursor.left, progressCursor.top, (progressCursor.right - progressCursor.left), (progressCursor.bottom - progressCursor.top), 
		   bdc, 2, 45, SRCCOPY);

	DeleteDC(bdc);
	ReleaseDC(hwnd, hdc);

	return 1;
}

int Skin::SetProgressValue(HWND hwnd, double progress)
{
	int  sizeX, sizeY;
	RECT clientRect;
	HDC  hdc, bdc;
	
	if(progress != this->progress) {

		hdc = GetDC(this->hwnd);
		bdc = CreateCompatibleDC(hdc);
 
		if(hdc == NULL) {

			return 0;
		}

		SelectObject(bdc, this->skin);

		/*
 		* Get some info about the
		* window size and all.
		*/

		GetClientRect(hwnd, &clientRect);

		sizeX = clientRect.right - clientRect.left;
		sizeY = clientRect.bottom - clientRect.top;
	
		BitBlt(hdc, progressCursor.left, progressCursor.top, 
			  (progressCursor.right - progressCursor.left), 
			  (progressCursor.bottom - progressCursor.top), 
			  bdc, 200, 321, SRCCOPY);

		SetProgress(hwnd, progress);


		/*
		 * The cursor
		 */

		SelectObject(bdc, this->controls);

		BitBlt(hdc, progressCursor.left, progressCursor.top, 
			  (progressCursor.right - progressCursor.left), 
			  (progressCursor.bottom - progressCursor.top), 
			  bdc, 2, 45, SRCCOPY);

		DeleteDC(bdc);
		ReleaseDC(hwnd, hdc);
	}

	return 1;
}

/*
 * Calculates the video size
 * and position on the hwnd
 */

RECT *Skin::GetVideoRect()
{
	if(this->hwnd) {

		GetClientRect(this->hwnd, &this->videoRect);

		if(this->compactMode) {

			return &this->videoRect;
		}
		else {

			this->videoRect.left    = 7;
			this->videoRect.right  -= 15;
			this->videoRect.top     = 22;
			this->videoRect.bottom -= 115;

			return &this->videoRect;
		}
	}

	return &this->videoRect;
}

/*
 * Set/Unset the compact mode
 */

int Skin::SetCompact(int compact)
{
	this->compactMode = compact;

	return 0;
}

/*
 * Gives the Progress and Volume
 *
 */

int Skin::GetProgress()
{
	return (int) this->progress;
}

int Skin::GetVolume()
{
	return this->volume;
}

/*
 * Display : display the skin 
 *           into a window
 *
 */

Skin::Display(HWND hwnd, MediaPlayback *playback)
{
	HDC          hdc, bdc;
	PAINTSTRUCT  ps;
	int          sizeX, sizeY, i;
	RECT         clientRect;
	HBRUSH       brush, oldBrush;

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;

	this->hwnd = hwnd;

	if(!playback->IsInFullscreen() || playback->desktopMode) {

		hdc  = BeginPaint(hwnd, &ps);

		if(this->compactMode) {

			/*
			 * Simply Draw the logo
			 */
		
			bdc	 = CreateCompatibleDC(hdc);
			SelectObject(bdc, this->logo);

			if(playback->IsOverlay()) {

				brush = CreateSolidBrush(DD_OVERLAY_COLORREF); 
	
				oldBrush = (HBRUSH) SelectObject(hdc, brush);
				Rectangle(hdc, 0, 0, sizeX, sizeY); 
		
				SelectObject(hdc, oldBrush);
				DeleteObject((HGDIOBJ) brush);
			}
			else {
				if(!playback->IsPlaying()) {

					brush = CreateSolidBrush(this->skinColor); 

					oldBrush = (HBRUSH) SelectObject(hdc, brush);
					Rectangle(hdc, 0, 0, DEFAULT_SKIN_WIDTH + (sizeX - DEFAULT_SKIN_WIDTH), 
						  DEFAULT_SKIN_HEIGHT + (sizeY - DEFAULT_SKIN_HEIGHT)); 
	
					SelectObject(bdc, this->logo);

					if(sizeY > LOGO_HEIGHT)
						BitBlt(hdc, (sizeX - LOGO_WIDTH)/2, (sizeY - LOGO_HEIGHT)/2, LOGO_WIDTH, LOGO_HEIGHT, bdc, 0, 0, SRCCOPY);

					SelectObject(hdc, oldBrush);
					DeleteObject((HGDIOBJ) brush);
				}
			}

			DeleteDC(bdc);
		}
		else {

			/*
			 * Paint the normal skin
			 */

			bdc  = CreateCompatibleDC(hdc);
			SelectObject(bdc, this->skin);

			if(hdc == NULL || bdc == NULL) {

				return 0;
			}

			/*
			 * First blit the background
 			 */

			/*
			 * Title
			 */

			BitBlt(hdc, 0, 0, 21, 22, bdc, 0, 0, SRCCOPY);

			for(i = 0; i < (sizeX - DEFAULT_SKIN_WIDTH) / 2; i++) {	

				BitBlt(hdc, 21 + i, 0, 1, 22, bdc, 21, 0, SRCCOPY);
			}

			BitBlt(hdc, 21 + (sizeX - DEFAULT_SKIN_WIDTH) / 2, 0, 240, 22, bdc, 21, 0, SRCCOPY);

			for(i = 0; i < (sizeX - DEFAULT_SKIN_WIDTH) / 2 + ((sizeX - DEFAULT_SKIN_WIDTH) % 2); i++) {

				BitBlt(hdc, 261 + (sizeX - DEFAULT_SKIN_WIDTH) / 2 + i, 0, 1, 22, bdc, 261, 0, SRCCOPY);
			}

			BitBlt(hdc, 261 + (sizeX - DEFAULT_SKIN_WIDTH), 0, 39, 22, bdc, 261, 0, SRCCOPY);

			/*
			 * The play Area
			 *
			 */

			for(i=0; i < 238 + (sizeY - DEFAULT_SKIN_HEIGHT); i++) {

				BitBlt(hdc, 0, 22 + i, 7, 1, bdc, 0, 22, SRCCOPY);
				BitBlt(hdc, 292 + (sizeX - DEFAULT_SKIN_WIDTH), 22 + i, 8, 1, bdc, 292, 22, SRCCOPY);
			}

			/*	
			 * The bottom
			 */

			BitBlt(hdc, 0, 260 + (sizeY - DEFAULT_SKIN_HEIGHT), 200, 115, bdc, 0, 260, SRCCOPY);
			BitBlt(hdc, 200 + (sizeX - DEFAULT_SKIN_WIDTH), 260 + (sizeY - DEFAULT_SKIN_HEIGHT), 100, 115, bdc, 200, 260, SRCCOPY);

			SelectObject(bdc, this->bottom);

			if(sizeX - DEFAULT_SKIN_WIDTH > 0) {

				int pass = (sizeX - DEFAULT_SKIN_WIDTH) / 256;

				for(i=0; i < pass; i++) {

					BitBlt(hdc, 200 + i*256, 260 + (sizeY - DEFAULT_SKIN_HEIGHT), 256, 115, bdc, 0, 0, SRCCOPY);
				}
		
				BitBlt(hdc, 200 + pass*256, 260 + (sizeY - DEFAULT_SKIN_HEIGHT), (sizeX - DEFAULT_SKIN_WIDTH) - pass*256, 115, bdc, 0, 0, SRCCOPY);
			}	

			/*
			 * Some info
			 */

			if(playback->HasVideo() && !this->compactMode) {

				HFONT font, oldFont;
				DWORD time, total;
				char  buffer[24];
				DWORD h, m, s, ht, mt, st;
				char *file;

				font = CreateFont(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
				oldFont = (HFONT) SelectObject(hdc, (HGDIOBJ) font);

				SetBkMode(hdc, TRANSPARENT);
				
				if(strrchr(playback->GetFilename(), '\\') != NULL) {

					file = strrchr(playback->GetFilename(), '\\')+1;
				}
				else {

					file = playback->GetFilename();
				}
				
				TextOut(hdc, 20, 282 + (sizeY - DEFAULT_SKIN_HEIGHT), file, strlen(file));

				time = playback->GetActualTime();
				total = playback->GetTotalTime();

				h = time / 3600;
				m = (time - h*3600) / 60;
				s = (time - h*3600 - m*60);
				
				ht = total / 3600;
				mt = (total - ht*3600) / 60;
				st = (total - ht*3600 - mt*60);

				sprintf(buffer, "%.2d:%.2d:%.2d / %.2d:%.2d:%.2d", h, m, s, ht, mt, st);

				TextOut(hdc, 20, 296 + (sizeY - DEFAULT_SKIN_HEIGHT), buffer, strlen(buffer));

				SelectObject(hdc, (HGDIOBJ) oldFont);
				DeleteObject((HGDIOBJ)font);
			}

			/*
			 * Now the controls
			 *
			 */

			SelectObject(bdc, this->controls);	

			BitBlt(hdc, progressCursor.left, progressCursor.top, (progressCursor.right - progressCursor.left), (progressCursor.bottom - progressCursor.top), 
				   bdc, 2, 45, SRCCOPY);

			BitBlt(hdc, volumeCursor.left, volumeCursor.top, (volumeCursor.right - volumeCursor.left), (volumeCursor.bottom - volumeCursor.top), 
				   bdc, 32, 45, SRCCOPY);

			/*
			 * And the time info
			 */

			if(!playback->desktopMode) {

				if(!playback->IsPlaying() && !playback->IsPaused()) {	

					brush = CreateSolidBrush(this->skinColor); 
	
					oldBrush = (HBRUSH) SelectObject(hdc, brush);
					Rectangle(hdc, 7, 22, 292 + (sizeX - DEFAULT_SKIN_WIDTH), 260 + (sizeY - DEFAULT_SKIN_HEIGHT)); 
		
					SelectObject(bdc, this->logo);

					if(sizeY > LOGO_HEIGHT + 135 + 22)
						BitBlt(hdc, (sizeX - LOGO_WIDTH)/2, 50 + (sizeY - DEFAULT_SKIN_HEIGHT)/2, LOGO_WIDTH, LOGO_HEIGHT, bdc, 0, 0, SRCCOPY);

					SelectObject(hdc, oldBrush);
					DeleteObject((HGDIOBJ) brush);
				}
				else {

					if(playback->IsOverlay()) {
		
						brush = CreateSolidBrush(DD_OVERLAY_COLORREF);	
						
						oldBrush = (HBRUSH) SelectObject(hdc, brush);
						Rectangle(hdc, 7, 22, 292 + (sizeX - DEFAULT_SKIN_WIDTH), 260 + (sizeY - DEFAULT_SKIN_HEIGHT)); 
		
						SelectObject(hdc, oldBrush);
						DeleteObject((HGDIOBJ) brush);
					}
				}

				if(playback->IsPaused()) {

					if(playback->IsOverlay()) {
		
						brush = CreateSolidBrush(DD_OVERLAY_COLORREF);	
						
						oldBrush = (HBRUSH) SelectObject(hdc, brush);
						Rectangle(hdc, 7, 22, 292 + (sizeX - DEFAULT_SKIN_WIDTH), 260 + (sizeY - DEFAULT_SKIN_HEIGHT)); 
		
						SelectObject(hdc, oldBrush);
						DeleteObject((HGDIOBJ) brush);
					}
					else {
						brush = CreateSolidBrush(0); 
 			
						oldBrush = (HBRUSH) SelectObject(hdc, brush);
						Rectangle(hdc, 7, 22, 292 + (sizeX - DEFAULT_SKIN_WIDTH), 260 + (sizeY - DEFAULT_SKIN_HEIGHT)); 
	
						SelectObject(hdc, oldBrush);
						DeleteObject((HGDIOBJ) brush);
					}
				}
			}

			DeleteDC(bdc);
		}

		EndPaint(hwnd, &ps); 
	}
	else {

		if(playback->IsOverlay() && playback->HasVideo()) {
	
			RECT *fullRects;

			fullRects = playback->videoRenderer->GetFullscreenRects();

			if(fullRects != NULL) {

				GetClientRect(hwnd, &clientRect);

				hdc  = BeginPaint(hwnd, &ps);
	
				brush = CreateSolidBrush(0);	
				oldBrush = (HBRUSH) SelectObject(hdc, brush);
			
				Rectangle(hdc, fullRects[0].left, fullRects[0].top, fullRects[0].left + fullRects[0].right + 50, fullRects[0].top + fullRects[0].bottom); 
				Rectangle(hdc, fullRects[2].left, fullRects[2].top, fullRects[2].left + fullRects[2].right + 50, fullRects[2].top + fullRects[2].bottom + 50); 
			
				DeleteObject((HGDIOBJ) brush);
				
				brush = CreateSolidBrush(DD_OVERLAY_COLORREF_FULLSCREEN);	
							
				(HBRUSH) SelectObject(hdc, brush);
			
				Rectangle(hdc, fullRects[1].left, fullRects[1].top, fullRects[1].left + fullRects[1].right + 50, fullRects[1].top + fullRects[1].bottom); 
			
				SelectObject(hdc, oldBrush);
				DeleteObject((HGDIOBJ) brush);

				EndPaint(hwnd, &ps); 
			}
		}
		else {

			if(playback->HasVideo()) {

				hdc  = BeginPaint(hwnd, &ps);
	
				brush = CreateSolidBrush(0);	
				oldBrush = (HBRUSH) SelectObject(hdc, brush);
			
				Rectangle(hdc, 0, 0, sizeX, sizeY); 
				
				SelectObject(hdc, oldBrush);
				DeleteObject((HGDIOBJ) brush);

				EndPaint(hwnd, &ps); 
			}
		}
	}

	return 1;
}


/*
 * Display the FPS
 *
 */

int Skin::DisplayFps(HWND hwnd, MediaPlayback *playback)
{
	HDC          hdc, bdc;
	int          sizeX, sizeY;
	RECT         clientRect;

	if(playback) {

		/*
	 	 * Get some info about the
		 * window size and all.
		*/

		GetClientRect(hwnd, &clientRect);






		sizeX = clientRect.right - clientRect.left;
		sizeY = clientRect.bottom - clientRect.top;

		hdc  = GetDC(hwnd);
		bdc  = CreateCompatibleDC(hdc);

		SelectObject(bdc, this->skin);

		BitBlt(hdc, 0, 260 + (sizeY - DEFAULT_SKIN_HEIGHT), 200, 40, bdc, 0, 260, SRCCOPY);

		char msg[10];
		sprintf(msg, "%3.f FPS", playback->GetCurrentFps());
		
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 15, 280 + (sizeY - DEFAULT_SKIN_HEIGHT), msg, 7);

		DeleteDC(bdc);
		ReleaseDC(hwnd, hdc);
	}
	
	return 1;
}

/*
 * Displays the buffering Bar
 *
 *
 */

int   Skin::DrawTime(HWND hwnd, MediaPlayback *playback) {

	HDC          hdc, bdc, ddc;
	int          sizeX, sizeY;
	RECT         clientRect;
	HFONT        font, oldFont;
	HBITMAP      buff;

	if(hwnd) {

		/*
	 	 * Get some info about the
		 * window size and all.
		 */

		GetClientRect(hwnd, &clientRect);

		sizeX = clientRect.right - clientRect.left;
		sizeY = clientRect.bottom - clientRect.top;

		hdc  = GetDC(hwnd);
		bdc  = CreateCompatibleDC(hdc);

		ddc  = CreateCompatibleDC(hdc);
		buff = CreateCompatibleBitmap(hdc, 106, 28);

		SelectObject(bdc, this->skin);
		SelectObject(ddc, buff);

		BitBlt(ddc, 0, 0, 106, 28, bdc, 17, 280, SRCCOPY);

		HFONT font, oldFont;
		DWORD time, total;
		char  buffer[24];
		DWORD h, m, s, ht, mt, st;
		char *file;

		if(strrchr(playback->GetFilename(), '\\') != NULL) {

			file = strrchr(playback->GetFilename(), '\\')+1;
		}
		else {

			file = playback->GetFilename();
		}


		font = CreateFont(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = (HFONT) SelectObject(ddc, (HGDIOBJ) font);

		SetBkMode(ddc, TRANSPARENT);
		TextOut(ddc, 3, 2, file, strlen(file));

		time = playback->GetActualTime();
		total = playback->GetTotalTime();

		h = time / 3600;
		m = (time - h*3600) / 60;
		s = (time - h*3600 - m*60);
			
		ht = total / 3600;
		mt = (total - ht*3600) / 60;
		st = (total - ht*3600 - mt*60);

		sprintf(buffer, "%.2d:%.2d:%.2d / %.2d:%.2d:%.2d", h, m, s, ht, mt, st);	

		TextOut(ddc, 3, 16, buffer, strlen(buffer));	

		SelectObject(ddc, (HGDIOBJ) oldFont);
		DeleteObject((HGDIOBJ)font);

		BitBlt(hdc, 17, 280 + (sizeY - DEFAULT_SKIN_HEIGHT), 106, 28, ddc, 0, 0, SRCCOPY);

		DeleteObject((HGDIOBJ) buff);

		DeleteDC(bdc);
		DeleteDC(ddc);
		ReleaseDC(hwnd, hdc);
	}

	return 0;
}


int   Skin::DrawBufferingState(HWND hwnd, DWORD state)
{
	HDC          hdc, bdc;
	int          sizeX, sizeY;
	RECT         clientRect;
	HFONT        font, oldFont;

	if(hwnd && state > 0) {

		/*
	 	 * Get some info about the
		 * window size and all.
		*/

		GetClientRect(hwnd, &clientRect);

		sizeX = clientRect.right - clientRect.left;
		sizeY = clientRect.bottom - clientRect.top;

		hdc  = GetDC(hwnd);
		bdc  = CreateCompatibleDC(hdc);

		SelectObject(bdc, this->skin);

		BitBlt(hdc, 17, 280 + (sizeY - DEFAULT_SKIN_HEIGHT), 106, 28, bdc, 17, 280, SRCCOPY);

		font = CreateFont(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = (HFONT) SelectObject(hdc, (HGDIOBJ) font);


		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 20, 282 + (sizeY - DEFAULT_SKIN_HEIGHT), "Buffering...", 12);

		SelectObject(hdc, (HGDIOBJ) oldFont);
		DeleteObject((HGDIOBJ)font);

		MoveToEx(hdc, 20, 295 + (sizeY - DEFAULT_SKIN_HEIGHT), NULL);
		LineTo(hdc, 120, 295 + (sizeY - DEFAULT_SKIN_HEIGHT));
		LineTo(hdc, 120, 305 + (sizeY - DEFAULT_SKIN_HEIGHT));
		LineTo(hdc, 20, 305 + (sizeY - DEFAULT_SKIN_HEIGHT));
		LineTo(hdc, 20, 295 + (sizeY - DEFAULT_SKIN_HEIGHT));

		Rectangle(hdc, 20, 295 + (sizeY - DEFAULT_SKIN_HEIGHT), 20 + state, 305 + (sizeY - DEFAULT_SKIN_HEIGHT));

		DeleteDC(bdc);
		ReleaseDC(hwnd, hdc);
	}

	return 1;
}


/*
 * Update the rects according
 * to the new size of the window
 *
 */

int Skin::UpdateSize(HWND hwnd)
{

	int          sizeX, sizeY;
	RECT         clientRect;

	/*
 	 * Get some info about the
	 * window size and all.
	 */

	GetClientRect(hwnd, &clientRect);

	sizeX = clientRect.right - clientRect.left;
	sizeY = clientRect.bottom - clientRect.top;

	this->menuButton.left   = 2;
	this->menuButton.right  = 14;
	this->menuButton.top    = 3;
	this->menuButton.bottom = 14;

	this->minimizeButton.left   = 266 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->minimizeButton.right  = 278 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->minimizeButton.top    = 3;
	this->minimizeButton.bottom = 14;

	this->closeButton.left   = 283 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->closeButton.right  = 295 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->closeButton.top    = 3;
	this->closeButton.bottom = 14;

	this->rewindButton.left   = 15;
	this->rewindButton.right  = 15+29;
	this->rewindButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->rewindButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->stopButton.left   = 52;
	this->stopButton.right  = 52+29;
	this->stopButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->stopButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->playButton.left   = 89;
	this->playButton.right  = 89+29;
	this->playButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->playButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->pauseButton.left   = 126;
	this->pauseButton.right  = 126+29;
	this->pauseButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->pauseButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->forwardButton.left   = 163;
	this->forwardButton.right  = 192;
	this->forwardButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->forwardButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->loadButton.left   = 236 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->loadButton.right  = 236 + 44 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->loadButton.top    = 343 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->loadButton.bottom = 360 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->resizeButton.left   = 282 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->resizeButton.right  = 282 + 15 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->resizeButton.top    = 357 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->resizeButton.bottom = 357 + 15 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->progressBar.left   = 12;
	this->progressBar.right  = 281 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->progressBar.top    = 325 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->progressBar.bottom = 332 + (sizeY - DEFAULT_SKIN_HEIGHT);

	this->volumeBar.left   = 212 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->volumeBar.right  = 273 + (sizeX - DEFAULT_SKIN_WIDTH);
	this->volumeBar.top    = 296 + (sizeY - DEFAULT_SKIN_HEIGHT);
	this->volumeBar.bottom = 304 + (sizeY - DEFAULT_SKIN_HEIGHT);

	SetVolume(hwnd, this->volume);
	SetProgress(hwnd, this->progress);

	return 1;
}

/*
 * Updates special zones upon action
 *
 */

int Skin::SetActionStart(HWND hwnd, int action)
{
	HDC          hdc, bdc;
	
	hdc = GetDC(hwnd);
	bdc = CreateCompatibleDC(hdc);
 
	if(hdc == NULL || bdc == NULL) {

		return 0;
	}

	SelectObject(bdc, this->controls);

	switch(action) {

	case ACTION_REWIND:

		BitBlt(hdc, rewindButton.left, rewindButton.top, (rewindButton.right - rewindButton.left), (rewindButton.bottom - rewindButton.top),
			   bdc, 2, 26, SRCCOPY);
		break;

	case ACTION_STOP:

		BitBlt(hdc, stopButton.left, stopButton.top, (stopButton.right - stopButton.left), (stopButton.bottom - stopButton.top),
			   bdc, 33, 26, SRCCOPY);
		break;
	
	case ACTION_PLAY:
		
		BitBlt(hdc, playButton.left, playButton.top, (playButton.right - playButton.left), (playButton.bottom - playButton.top),
			   bdc, 64, 26, SRCCOPY);
		break;
	
	case ACTION_PAUSE:
		
		BitBlt(hdc, pauseButton.left, pauseButton.top, (pauseButton.right - pauseButton.left), (pauseButton.bottom - pauseButton.top),
			   bdc, 95, 26, SRCCOPY);
		break;
	
	case ACTION_FORWARD:
		
		BitBlt(hdc, forwardButton.left, forwardButton.top, (forwardButton.right - forwardButton.left), (forwardButton.bottom - forwardButton.top),
			   bdc, 126, 26, SRCCOPY);
		break;

	case ACTION_LOAD:
		
		BitBlt(hdc, loadButton.left, loadButton.top, (loadButton.right - loadButton.left), (loadButton.bottom - loadButton.top),
			   bdc, 157, 26, SRCCOPY);
		break;

	default:
		break;
	}

	DeleteDC(bdc);
	ReleaseDC(hwnd, hdc);

	return 1;
}

int Skin::SetActionEnd(HWND hwnd, int action)
{
	HDC          hdc, bdc;
	
	hdc = GetDC(hwnd);
	bdc = CreateCompatibleDC(hdc);
 
	if(hdc == NULL || bdc == NULL) {

		return 0;
	}

	SelectObject(bdc, this->skin);

	switch(action) {

	case ACTION_REWIND:

		BitBlt(hdc, rewindButton.left, rewindButton.top, (rewindButton.right - rewindButton.left), (rewindButton.bottom - rewindButton.top),
			   bdc, 15, 343, SRCCOPY);
		break;

	case ACTION_STOP:

		BitBlt(hdc, stopButton.left, stopButton.top, (stopButton.right - stopButton.left), (stopButton.bottom - stopButton.top),
			   bdc, 52, 343, SRCCOPY);
		break;
	
	case ACTION_PLAY:
		
		BitBlt(hdc, playButton.left, playButton.top, (playButton.right - playButton.left), (playButton.bottom - playButton.top),
			   bdc, 89, 343, SRCCOPY);
		break;
	
	case ACTION_PAUSE:
		
		BitBlt(hdc, pauseButton.left, pauseButton.top, (pauseButton.right - pauseButton.left), (pauseButton.bottom - pauseButton.top),
			   bdc, 126, 343, SRCCOPY);
		break;
	
	case ACTION_FORWARD:
		
		BitBlt(hdc, forwardButton.left, forwardButton.top, (forwardButton.right - forwardButton.left), (forwardButton.bottom - forwardButton.top),
			   bdc, 163, 343, SRCCOPY);
		break;

	case ACTION_LOAD:
		
		BitBlt(hdc, loadButton.left, loadButton.top, (loadButton.right - loadButton.left), (loadButton.bottom - loadButton.top),
			   bdc, 236, 343, SRCCOPY);
		break;

	default:
		break;
	}

	DeleteDC(bdc);
	ReleaseDC(hwnd, hdc);
	
	return 1;
}

/*
 * Returns the action corresponding
 * to the given mouse click inside 
 * the skin
 *
 */

int Skin::GetAction(int x, int y)
{
	if(isInsideRect(x, y, this->menuButton))
		return ACTION_MENU;

	if(isInsideRect(x, y, this->minimizeButton))
		return ACTION_MINIMIZE;

	if(isInsideRect(x, y, this->closeButton))
		return ACTION_CLOSE;

	if(isInsideRect(x, y, this->rewindButton))
		return ACTION_REWIND;

	if(isInsideRect(x, y, this->stopButton))
		return ACTION_STOP;

	if(isInsideRect(x, y, this->playButton))
		return ACTION_PLAY;

	if(isInsideRect(x, y, this->pauseButton))
		return ACTION_PAUSE;

	if(isInsideRect(x, y, this->forwardButton))
		return ACTION_FORWARD;

	if(isInsideRect(x, y, this->loadButton))
		return ACTION_LOAD;

	if(isInsideRect(x, y, this->resizeButton))
		return ACTION_RESIZE;

	if(isInsideRect(x, y, this->volumeCursor))
		return ACTION_VOLUME_CURSOR;
	
	if(isInsideRect(x, y, this->progressCursor))
		return ACTION_PROGRESS_CURSOR;

	if(isInsideRect(x, y, this->volumeBar))
		return ACTION_VOLUME_BAR;

	if(isInsideRect(x, y, this->progressBar))
		return ACTION_PROGRESS_BAR;
	
	return ACTION_NONE;
}
