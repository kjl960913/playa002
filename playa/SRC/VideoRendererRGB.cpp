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
 * Authors: Damien Chavarria                                                          *
 *          DivX Advanced Research Center <darc at projectmayo.com>                   *
 *                                                                                    *
 **************************************************************************************/
#include <ddraw.h>
#include "VideoRendererRGB.h"

/*
 * DirectDraw includes
 *
 */



/*
 * RGB VideoRenderer Class
 */


MediaVideoRendererRGB::MediaVideoRendererRGB()
{
	this->subtitler    = NULL;

	this->lpdd         = NULL;
	this->lpddClipper  = NULL;
	this->lpddsPrimary = NULL;
	this->lpddsBack    = NULL;

	this->bpp          = 0;
	this->videoMode    = VIDEO_MODE_NONE;
}

MediaVideoRendererRGB::~MediaVideoRendererRGB()
{

}

/*
 * Media Item functions
 */

media_type_t  MediaVideoRendererRGB::GetType()
{
	return MEDIA_TYPE_VIDEO_RENDERER;
}

char         *MediaVideoRendererRGB::GetName()
{
	return "Standard RGB Video Renderer";
}

MP_RESULT MediaVideoRendererRGB::Connect(MediaItem *item)
{
	/*
	 * We accept only Subtitlers
	 *
	 */

	if(item && item->GetType() == MEDIA_TYPE_SUBTITLER) {

		/*
		 * Accpets the subtitle source
		 */

		this->subtitler = (MediaItemSubtitler *) item;

		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererRGB::ReleaseConnections()
{
	if(this->subtitler)
		this->subtitler = NULL;

	return MP_RESULT_OK;
}


DWORD         MediaVideoRendererRGB::GetCaps()
{
	return 0;
}

MP_RESULT     MediaVideoRendererRGB::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Video Renderer Functions
 */

MP_RESULT MediaVideoRendererRGB::Init(HWND hwnd, unsigned int width, unsigned int height)
{
	HRESULT        ddrval;
	DDSURFACEDESC2 ddsd;

	if(hwnd && width > 0 && height > 0) {

		/*
		 * Create the main DirectDraw Object
		 */

		ddrval = DirectDrawCreateEx(NULL, (void **) &this->lpdd, IID_IDirectDraw7, NULL);

		if(FAILED(ddrval)) {
	
			return MP_RESULT_ERROR;
		}

		ddrval = lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);

		if(FAILED(ddrval)) {

			this->lpdd->Release();
			this->lpdd = NULL;
			
			return MP_RESULT_ERROR;
		}
	
		/*
		 * Create the primary surface
		 */

		memset( &ddsd, 0, sizeof(ddsd) );
	    ddsd.dwSize     = sizeof( ddsd );

	    ddsd.dwFlags           = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

	    ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL);

		if(FAILED(ddrval)) {

			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
		}

		/*
		 * Now set Clipping
		 */

		ddrval = this->lpdd->CreateClipper(0, &this->lpddClipper, NULL);

		if(FAILED(ddrval)) {

			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			
			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
		}

	    ddrval = this->lpddClipper->SetHWnd(0, hwnd);

	    if(FAILED(ddrval)) {

			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			
			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
		}

		ddrval = this->lpddsPrimary->SetClipper(this->lpddClipper);

		if(ddrval != DD_OK) {

			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			
			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
		}
	
		/*
		 * Finally Create Back Surface
		 */
	
		ZeroMemory(&ddsd, sizeof(ddsd));
    	ddsd.dwSize     = sizeof(ddsd);
			
		ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		
		ddsd.dwWidth  = width;
		ddsd.dwHeight = height;

		if(this->lpdd->CreateSurface(&ddsd, &this->lpddsBack, NULL) != DD_OK) {
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			
			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
			
		}

		this->width  = width;
		this->height = height;

		this->hwndPlayback = hwnd;

		/*
		 * Now Get the video mode
		 */

		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddrval = this->lpddsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		this->lpddsBack->Unlock(NULL);

		switch(ddsd.ddpfPixelFormat.dwRGBBitCount) {

		case 8:
			return MP_RESULT_ERROR;
			break;

		case 16:
			this->bpp       = 2;
			this->videoMode = VIDEO_MODE_RGB16;
			break;

		case 24:
			this->bpp       = 3;
			this->videoMode = VIDEO_MODE_RGB24;
			break;

		case 32:
			this->bpp       = 4;
			this->videoMode = VIDEO_MODE_RGB32;
			break;
		}

		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

/*
 * Init the renderer for 
 * fullscreen drawing
 *
 */

MP_RESULT MediaVideoRendererRGB::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height)
{
	if(hwnd && width > 0 && height > 0) {
	
		HRESULT         ddrval;
	    DDSURFACEDESC2  ddsd;
    
	    ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
		
		if( FAILED(ddrval))
			return MP_RESULT_ERROR;
    
	    /*
	     * Set Exclusive Cooperative Mode
	     */

		ddrval = this->lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL); //DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		
		if( FAILED(ddrval)) {

	        this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
		}

		/*
		 * Go to fullscreen
		 *
		 */

		DDSURFACEDESC2 ddDesc;

		memset(&ddDesc, 0, sizeof(DDSURFACEDESC2));
		ddDesc.dwSize    = sizeof(DDSURFACEDESC2);

		/*
		 * We need to know witch resolution
		 * we are in...
		 */

		ddrval = this->lpdd->GetDisplayMode(&ddDesc);

		if(FAILED(ddrval)) {

	        this->lpdd->Release();
			this->lpdd=NULL;
			return MP_RESULT_ERROR;;
		}

		/*
		 * Store the fullscreen mode
		 */

		this->fullscreenWidth  = ddDesc.dwWidth;
		this->fullscreenHeight = ddDesc.dwHeight;

		this->fullscreenVideoHeight = height * this->fullscreenWidth / width;

		/*
		 * And create the primary surface
		 */
   
	    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		
		ddsd.dwSize  = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;// | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY; /*| DDSCAPS_FLIP | DDSCAPS_COMPLEX*/;

		//ddsd.dwBackBufferCount = 1;
    
		ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );

		if(FAILED(ddrval)) {

	        this->lpdd->Release();
			this->lpdd=NULL;
			return MP_RESULT_ERROR;;
		}

		/*
		 * Get the back buffer
		 *
		 */

		ZeroMemory(&ddsd, sizeof(ddsd));
    	ddsd.dwSize     = sizeof(ddsd);
			
		ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		
		ddsd.dwWidth  = width;
		ddsd.dwHeight = height;

		if(this->lpdd->CreateSurface(&ddsd, &this->lpddsBack, NULL) != DD_OK) {
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			
			this->lpdd->Release();
			this->lpdd = NULL;

			return MP_RESULT_ERROR;
			
		}

		/*
		 * Create a Clipper to avoid 
		 * the overlay staying on top
		 */

	    ddrval = this->lpdd->CreateClipper(0, &this->lpddClipper, NULL);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}
	
	    ddrval = this->lpddClipper->SetHWnd(0, hwnd);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}
	
	    ddrval = this->lpddsPrimary->SetClipper(this->lpddClipper);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}
		/*
		 * Black out the primary
		 */

		this->width  = width;
		this->height = height;

		this->hwndPlayback = hwnd;

		return MP_RESULT_OK;
	}
    
	return MP_RESULT_ERROR;
}

media_video_mode_t MediaVideoRendererRGB::GetVideoMode()
{
	return this->videoMode;
}

MP_RESULT MediaVideoRendererRGB::Stop()
{
	return MP_RESULT_OK;
}

RECT *MediaVideoRendererRGB::GetFullscreenRects()
{
	return NULL;
}

MP_RESULT MediaVideoRendererRGB::Draw(MediaBuffer *buffer, RECT *rect, int frameNumber, int invertFlag)
{
	HRESULT        ddrval;
	DDSURFACEDESC2 desc;
	DWORD          i;
	subtitles_t   *sub;

	if(this->lpdd && this->lpddsBack && this->lpddsPrimary && buffer) {

		/*
		 * Let's Go!
		 */

		ZeroMemory(&desc, sizeof(DDSURFACEDESC2));
		desc.dwSize     = sizeof(DDSURFACEDESC2);

		ddrval = this->lpddsBack->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		/*
		 * Copy the pixels
		 */
	
		for(i=0; i < desc.dwHeight; i++) {

			memcpy((char *) desc.lpSurface + i*desc.lPitch, (char *) buffer->GetData() + (desc.dwHeight - i - 1)*this->bpp*this->width, this->width*this->bpp);
		}

		this->lpddsBack->Unlock(NULL);

		/*
		 * Apply the subtitles if any
		 */

		if(this->subtitler) {

			sub = this->subtitler->GetSubtitles(frameNumber);

			if(sub != NULL) {

				/*
				 * Draw them
				 */

				HDC dc;

				ddrval = this->lpddsBack->GetDC(&dc);

				if(!FAILED(ddrval)) {

					DWORD length, length2, length3;
					SetBkMode(dc, TRANSPARENT);

					switch(sub->nbSubtitles) {

					case 1:
						
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 40 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 40, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						break;

					case 2:
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						length2 = GetTabbedTextExtent(dc, sub->subtitlesText[1], 
													 strlen(sub->subtitlesText[1]),
													 0, NULL);

						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 60 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2 + 1, this->height - 40 + 1, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 60, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2, this->height - 40, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						break;

					case 3:
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						length2 = GetTabbedTextExtent(dc, sub->subtitlesText[1], 
													 strlen(sub->subtitlesText[1]),
													 0, NULL);
						length3 = GetTabbedTextExtent(dc, sub->subtitlesText[2], 
													 strlen(sub->subtitlesText[2]),
													 0, NULL);

						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 80 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2 + 1, this->height - 60 + 1, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						TextOut(dc, (this->width - length3)/2 + 1, this->height - 40 + 1, sub->subtitlesText[2], strlen(sub->subtitlesText[2]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 80, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2, this->height - 60, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						TextOut(dc, (this->width - length3)/2, this->height - 40, sub->subtitlesText[2], strlen(sub->subtitlesText[2]));
						break;
					}

					this->lpddsBack->ReleaseDC(dc);
				}
			}
		}

		/*
		 * Now Blit to the window
		 */

		if(this->hwndPlayback) {

			RECT                rcRect;
			RECT                destRect;
			POINT               pt;

	        rcRect.left   = 0;
		    rcRect.top    = 0;
			rcRect.right  = this->width;
			rcRect.bottom = this->height;

			GetClientRect( this->hwndPlayback, &destRect );

			/*
			 * squeeze rect so that
			 * we have space for controls
			 */


			destRect.left   += rect->left;
			destRect.right  =  rect->left + rect->right;

			destRect.top    += rect->top;
			destRect.bottom  = rect->bottom;

			pt.x = pt.y = 0;
				
			ClientToScreen( this->hwndPlayback, &pt );
			OffsetRect(&destRect, pt.x, pt.y);

			while( 1 )
			{
				ddrval = this->lpddsPrimary->Blt( &destRect, this->lpddsBack, &rcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);

				if( ddrval == DD_OK )
				{
					break;
				}

				if( ddrval == DDERR_SURFACELOST )
				{
					if(!this->lpdd->RestoreAllSurfaces())
					{
						return MP_RESULT_ERROR;
					}	
				}	

				if( ddrval != DDERR_WASSTILLDRAWING )
				{
					return MP_RESULT_ERROR;
				}
			}

			if(ddrval != DD_OK)
			{
				return MP_RESULT_ERROR;
			}

			return MP_RESULT_OK;
		}
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererRGB::DrawFullscreen(MediaBuffer *buffer, int frameNumber, int invertFlag, int desktop)
{
	DDSURFACEDESC2 desc;
	DWORD          i;
	HRESULT        ddrval;
	subtitles_t   *sub;

	if(buffer && this->lpdd && this->lpddsPrimary) {

		ZeroMemory(&desc, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);

		this->lpddsBack->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

		for(i=0; i < desc.dwHeight; i++) {

			memcpy((char *) desc.lpSurface + i*desc.lPitch, (char *) buffer->GetData() + (desc.dwHeight - i - 1)*this->bpp*this->width, this->width*this->bpp);
		}

		this->lpddsBack->Unlock(NULL);

		/*
		 * Subtitles
		 */

		if(this->subtitler) {

			sub = this->subtitler->GetSubtitles(frameNumber);

			if(sub != NULL) {

				/*
				 * Draw them
				 */

				HDC dc;

				ddrval = this->lpddsBack->GetDC(&dc);

				if(!FAILED(ddrval)) {

					DWORD length, length2, length3;
					SetBkMode(dc, TRANSPARENT);

					switch(sub->nbSubtitles) {

					case 1:
						
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 40 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 40, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						break;

					case 2:
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						length2 = GetTabbedTextExtent(dc, sub->subtitlesText[1], 
													 strlen(sub->subtitlesText[1]),
													 0, NULL);

						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 60 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2 + 1, this->height - 40 + 1, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 60, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2, this->height - 40, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						break;

					case 3:
						length = GetTabbedTextExtent(dc, sub->subtitlesText[0], 
													 strlen(sub->subtitlesText[0]),
													 0, NULL);
						length2 = GetTabbedTextExtent(dc, sub->subtitlesText[1], 
													 strlen(sub->subtitlesText[1]),
													 0, NULL);
						length3 = GetTabbedTextExtent(dc, sub->subtitlesText[2], 
													 strlen(sub->subtitlesText[2]),
													 0, NULL);

						SetTextColor(dc, 0);
						TextOut(dc, (this->width - length)/2 + 1, this->height - 80 + 1, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2 + 1, this->height - 60 + 1, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						TextOut(dc, (this->width - length3)/2 + 1, this->height - 40 + 1, sub->subtitlesText[2], strlen(sub->subtitlesText[2]));
						SetTextColor(dc, 0xFFFFFF);
						TextOut(dc, (this->width - length)/2, this->height - 80, sub->subtitlesText[0], strlen(sub->subtitlesText[0]));
						TextOut(dc, (this->width - length2)/2, this->height - 60, sub->subtitlesText[1], strlen(sub->subtitlesText[1]));
						TextOut(dc, (this->width - length3)/2, this->height - 40, sub->subtitlesText[2], strlen(sub->subtitlesText[2]));
						break;
					}

					this->lpddsBack->ReleaseDC(dc);
				}
			}
		}

		/*
		 * Clear the back buffer
		 *
		 */
		/*
		 * Now Blit to the Back Buffer
		 */

		RECT rcRect, dst;

		rcRect.left   = 0;
		rcRect.top    = 0;
		rcRect.right  = this->width;
		rcRect.bottom = this->height;

		DWORD height;
		DWORD width;

		switch(options.aspect_ratio) {

		case ASPECT_RATIO_FREE:
		case ASPECT_RATIO_ORIGINAL:

			width  = this->width;
			height = this->height;
			break;

		case ASPECT_RATIO_TV:
		case ASPECT_RATIO_WIDE:
		case ASPECT_RATIO_CUSTOM:
			
			if(aspectRatios[options.aspect_ratio].yFactor < aspectRatios[options.aspect_ratio].xFactor) {

				width  = this->width;
				height = this->width*aspectRatios[options.aspect_ratio].yFactor/aspectRatios[options.aspect_ratio].xFactor;
			}
			else {
			
				height = this->height;
				width  = this->height*aspectRatios[options.aspect_ratio].xFactor/aspectRatios[options.aspect_ratio].yFactor;
			}
			break;
		}

		if(this->fullscreenWidth * height / width > this->fullscreenHeight) {

		    dst.left   = (this->fullscreenWidth - (this->fullscreenHeight * width / height)) / 2; 
			dst.top    = 0; 
			dst.right  = dst.left + (this->fullscreenHeight * width / height);
			dst.bottom = this->fullscreenHeight; 
		}
		else {

			dst.left   = 0;
			dst.top    = (this->fullscreenHeight - (this->fullscreenWidth * height / width))/2;
			dst.right  = this->fullscreenWidth;
			dst.bottom = (this->fullscreenHeight - (this->fullscreenWidth * height / width))/2 + (this->fullscreenWidth * height / width);
		}

		ddrval = this->lpddsPrimary->Blt( &dst, this->lpddsBack, &rcRect, DDBLT_ASYNC, NULL);
	
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererRGB::Close()
{
	if(this->lpdd) {
		
		this->lpdd->RestoreDisplayMode();
		this->lpdd->SetCooperativeLevel(this->hwndPlayback, DDSCL_NORMAL);
	}

	if(this->lpddsBack) {
	
		this->lpddsBack->Release();
		this->lpddsBack = NULL;
	}

	if(this->lpddsPrimary) {
	
		this->lpddsPrimary->Release();
		this->lpddsPrimary = NULL;
	}

	if(this->lpdd) {
	
		this->lpdd->Release();
		this->lpdd = NULL;
	}

	return MP_RESULT_OK;
}

