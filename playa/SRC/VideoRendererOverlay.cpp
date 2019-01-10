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
#include "VideoRendererOverlay.h"

DDPIXELFORMAT ddpfOverlayFormats[] = 
{   
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},  // YUY2
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},  // UYVY
};

media_video_mode_t videoModes[] = 
{
	VIDEO_MODE_YUY2,
	VIDEO_MODE_UYVY,
};

/*
 * Overlay VideoRenderer Class
 */


MediaVideoRendererOverlay::MediaVideoRendererOverlay()
{
	this->subtitler    = NULL;
	
	this->lpdd         = NULL;
	this->lpddsPrimary = NULL;
	this->lpddsOverlay = NULL;

	this->videoMode    = VIDEO_MODE_NONE;
}

MediaVideoRendererOverlay::~MediaVideoRendererOverlay()
{
}

/*
 * Media Item functions
 */

media_type_t MediaVideoRendererOverlay::GetType()
{
	return MEDIA_TYPE_VIDEO_RENDERER;
}

char *MediaVideoRendererOverlay::GetName()
{
	return "Overlay Video Renderer";
}

MP_RESULT MediaVideoRendererOverlay::Connect(MediaItem *item)
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

MP_RESULT MediaVideoRendererOverlay::ReleaseConnections()
{
	/*
	 * Accepts nothing excpt sub
	 */

	this->subtitler   = NULL;

	return MP_RESULT_OK;
}


DWORD         MediaVideoRendererOverlay::GetCaps()
{
	return 0;
}

MP_RESULT     MediaVideoRendererOverlay::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Check for Overlay Support
 *
 */

BOOL MediaVideoRendererOverlay::AreOverlaysSupported()
{

    DDCAPS  capsDrv;
    HRESULT ddrval;
	
    /* Get driver capabilities to 
	 * determine Overlay support.
	 */

    ZeroMemory(&capsDrv, sizeof(capsDrv));
    capsDrv.dwSize = sizeof(capsDrv);
 
    ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
    
	if (FAILED(ddrval))
        return FALSE;
 
    /* Does the driver support overlays in the current mode? 
     * Overlay related APIs will fail without hardware support.
	 */

    if (!(capsDrv.dwCaps & DDCAPS_OVERLAY))
        return FALSE;
	
    return TRUE;
}

/*
 * Video Renderer Functions
 */

MP_RESULT MediaVideoRendererOverlay::Init(HWND hwnd, unsigned int width, unsigned int height)
{
	if(hwnd && width > 0 && height > 0) {
	
		HRESULT         ddrval;
	    DDSURFACEDESC2  ddsd;
	    DDSURFACEDESC2  ddsdOverlay;
		DWORD           i;
    
		this->invertFlag  = FALSE;

		ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
		
		if( FAILED(ddrval))
			return MP_RESULT_ERROR;
    
		/*
		 * Check for Overlay Support
		 */

		if(!this->AreOverlaysSupported()) {

	        this->lpdd->Release();
			this->lpdd=NULL;
			return MP_RESULT_ERROR;
		}
	    /*
	     * Set Normal Cooperative Mode
	     */

		ddrval = this->lpdd->SetCooperativeLevel(NULL, DDSCL_NORMAL);
		
		if( FAILED(ddrval)) {

	        this->lpdd->Release();
			this->lpdd=NULL;
			return MP_RESULT_ERROR;
		}

		/*
		 * And create the primary surface
		 */
   
	    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		
		ddsd.dwSize  = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    
		ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );

		if(FAILED(ddrval)) {

			this->lpdd->Release();
		    this->lpdd=NULL;
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

		/*
		 * Now create the Overlay
		 */

		/*
		 * First try the best 
		 * configuration
		 */
    
	    ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));

		ddsdOverlay.dwSize  = sizeof(DDSURFACEDESC2);

	    ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
		ddsdOverlay.dwFlags= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		
		ddsdOverlay.dwWidth           = width;
		ddsdOverlay.dwHeight          = height;
    
	    i = 0;
    
		do {
	
			ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
			this->videoMode = videoModes[i];

  			ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		
		} while( FAILED(ddrval) && (++i < 2) );

		/*
		 * If it failed try a 
		 * more simple overlay
		 */

		if(FAILED(ddrval)) {

		    ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
			ddsdOverlay.dwSize     = sizeof(DDSURFACEDESC2);
			
			ddsdOverlay.dwWidth           = width;
			ddsdOverlay.dwHeight          = height;
	        ddsdOverlay.dwBackBufferCount = 0;
			
			ddsdOverlay.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
			ddsdOverlay.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        
			// Try to create the overlay surface

		    i = 0;
    
			do {
	
				ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
				this->videoMode = videoModes[i];

	 			ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		
			} while( FAILED(ddrval) && (++i < 2) );
        
			if (FAILED(ddrval)) {

				this->lpddsPrimary->Release();
				this->lpddsPrimary = NULL;

		        this->lpdd->Release();
				this->lpdd=NULL;

				this->videoMode = VIDEO_MODE_NONE;

	            return MP_RESULT_ERROR;
			}
		}

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		this->width  = width;
		this->height = height;

		this->bpp    = 16;

		/*
		 * Now get the Physical Depth
		 */

		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddrval = this->lpddsPrimary->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		this->lpddsPrimary->Unlock(NULL);

		switch(ddsd.ddpfPixelFormat.dwRGBBitCount) {

		case 8:
			return MP_RESULT_ERROR;
			break;

		case 16:
			this->physicalDepth = 16;
			break;

		case 24:
			this->physicalDepth = 24;
			break;

		case 32:
			this->physicalDepth = 32;
			break;
		}
		
		/*
		 * Get the screen size
		 *
		 */

		ddrval = this->lpdd->GetDisplayMode(&ddsd);

		/*
		 * Store the fullscreen mode
		 */

		this->fullscreenWidth  = ddsd.dwWidth;
		this->fullscreenHeight = ddsd.dwHeight;

		/*
		 * And keep the window pointer
		 */

		this->hwndPlayback = hwnd;

		return MP_RESULT_OK;
	}
    
	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererOverlay::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height)
{
	if(hwnd && width > 0 && height > 0) {
	
		HRESULT         ddrval;
	    DDSURFACEDESC2  ddsd;
	    DDSURFACEDESC2  ddsdOverlay;
		DWORD           i;

		this->invertFlag  = FALSE;
    
	    ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
		
		if( FAILED(ddrval))
			return MP_RESULT_ERROR;
    
		/*
		 * Check for Overlay Support
		 */

		if(!this->AreOverlaysSupported()) {

	        this->lpdd->Release();
			this->lpdd=NULL;
			return MP_RESULT_ERROR;
		}
	    /*
	     * Set Normal Cooperative Mode
	     */

		ddrval = this->lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
		
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

		/*
		 * And create the primary surface
		 */
   
	    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		
		ddsd.dwSize  = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    
		ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );

		if(FAILED(ddrval)) {

			this->lpdd->Release();
		    this->lpdd=NULL;
			return MP_RESULT_ERROR;
		}

		/*
		 * Now create the Overlay
		 */

		/*
		 * First try the best 
		 * configuration
		 */
    
	    ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));

		ddsdOverlay.dwSize  = sizeof(DDSURFACEDESC2);

	    ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
		ddsdOverlay.dwFlags= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		
		ddsdOverlay.dwWidth           = width;
		ddsdOverlay.dwHeight          = height;
    
	    i = 0;
    
		do {
	
			ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
			this->videoMode = videoModes[i];

  			ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		
		} while( FAILED(ddrval) && (++i < 2) );

		/*
		 * If it failed try a 
		 * more simple overlay
		 */

		if(FAILED(ddrval)) {

		    ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
			ddsdOverlay.dwSize     = sizeof(DDSURFACEDESC2);
			
			ddsdOverlay.dwWidth           = width;
			ddsdOverlay.dwHeight          = height;
	        ddsdOverlay.dwBackBufferCount = 0;
			
			ddsdOverlay.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
			ddsdOverlay.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        
			// Try to create the overlay surface

		    i = 0;
    
			do {
	
				ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
				this->videoMode = videoModes[i];

	 			ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		
			} while( FAILED(ddrval) && (++i < 2) );
        
			if (FAILED(ddrval)) {

				this->lpddsPrimary->Release();
				this->lpddsPrimary = NULL;

		        this->lpdd->Release();
				this->lpdd=NULL;

				this->videoMode = VIDEO_MODE_NONE;

	            return MP_RESULT_ERROR;
			}
		}

		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		this->width  = width;
		this->height = height;

		this->bpp    = 16;

		this->hwndPlayback = hwnd;

		return MP_RESULT_OK;
	}
    
	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererOverlay::Stop()
{
	if(this->lpdd && this->lpddsOverlay) {
	
		this->lpddsOverlay->UpdateOverlay(NULL, this->lpddsPrimary, NULL, DDOVER_HIDE, NULL);
	}

	return MP_RESULT_OK;
}

media_video_mode_t MediaVideoRendererOverlay::GetVideoMode()
{
	return this->videoMode;
}

RECT *MediaVideoRendererOverlay::GetFullscreenRects()
{
	DWORD width, height;

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

		this->fullRects[0].left   = 0;
		this->fullRects[0].right  = (this->fullscreenWidth - (width * this->fullscreenHeight / height)) / 2;
		this->fullRects[0].top    = 0;
		this->fullRects[0].bottom = this->fullscreenHeight + 10;

		this->fullRects[1].left   = (this->fullscreenWidth - (width * this->fullscreenHeight / height)) / 2;
		this->fullRects[1].right  = (width * this->fullscreenHeight / height);
		this->fullRects[1].top    = 0;
		this->fullRects[1].bottom = this->fullscreenHeight + 10;

		this->fullRects[2].left   = this->fullRects[1].left + this->fullRects[1].right;
		this->fullRects[2].right  = (this->fullscreenWidth - (width * this->fullscreenHeight / height)) / 2;
		this->fullRects[2].top    = 0;
		this->fullRects[2].bottom = this->fullscreenHeight + 10;
	}
	else {

		this->fullRects[0].left   = 0;
		this->fullRects[0].right  = this->fullscreenWidth;
		this->fullRects[0].top    = 0;
		this->fullRects[0].bottom = (this->fullscreenHeight - (this->fullscreenWidth * height / width)) / 2;

		this->fullRects[1].left   = 0;
		this->fullRects[1].right  = this->fullscreenWidth;
		this->fullRects[1].top    = (this->fullscreenHeight - (this->fullscreenWidth * height / width)) / 2;
		this->fullRects[1].bottom = this->fullscreenWidth * height / width;

		this->fullRects[2].left   = 0;
		this->fullRects[2].right  = this->fullscreenWidth;
		this->fullRects[2].top    = this->fullRects[1].top + this->fullRects[1].bottom;
		this->fullRects[2].bottom = (this->fullscreenHeight - (this->fullscreenWidth * height / width)) / 2;
	}
	
	return this->fullRects;
}

MP_RESULT MediaVideoRendererOverlay::Draw(MediaBuffer *buffer, RECT *rect, int frameNumber, int invertFlag)
{
	if(buffer && rect && this->lpdd && this->lpddsPrimary && this->lpddsOverlay) {

		POINT           pt;
	    HRESULT         ddrval;
	    RECT            rs, rd;
		DDOVERLAYFX     ovfx;
		DDSURFACEDESC2  ddsd;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000, i;
		unsigned int    uDestSizeAlign, uSrcSizeAlign;
		DWORD           dwUpdateFlags;
  	    subtitles_t    *sub;
 
		/*
		 * First copy the image into the 
		 * Overlays backbuffer...
		 */

		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddrval = this->lpddsOverlay->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
 
		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		if(invertFlag) {

			for(i = 0; i < ddsd.dwHeight; i++) {

				memcpy((char *) ddsd.lpSurface + i*ddsd.lPitch, ((char *) buffer->GetData()) + (this->height - i - 1)*ddsd.dwWidth*this->bpp/8, ddsd.dwWidth*this->bpp/8);
			}
			
		}
		else {

			if(ddsd.dwWidth == ddsd.lPitch || this->videoMode == VIDEO_MODE_YUV12) {

				memcpy(ddsd.lpSurface, buffer->GetData(), this->width*this->height*this->bpp/8);
			}
			else {
		
				for(i = 0; i < ddsd.dwHeight; i++) {

					memcpy((char *) ddsd.lpSurface + i*ddsd.lPitch, ((char *) buffer->GetData()) + i*ddsd.dwWidth*this->bpp/8, ddsd.dwWidth*this->bpp/8);
				}
			}
		}

		this->lpddsOverlay->Unlock(NULL);

		/*
	     * Get driver capabilities
	  	 */

		ZeroMemory(&capsDrv, sizeof(DDCAPS));
		capsDrv.dwSize    = sizeof(DDCAPS);

		ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
		
		if (FAILED(ddrval))
			return MP_RESULT_ERROR;
    
	    /* 
		 * We need to check the minimum stretch.  Many display adpators require that
		 * the overlay be stretched by a minimum amount.  The stretch factor will 
		 * usually vary with the display mode (including changes in refresh rate).
		 * The stretch factor is returned x1000.
         * 
		 */

		uStretchFactor1000 = capsDrv.dwMinOverlayStretch > 1000 ? capsDrv.dwMinOverlayStretch : 1000;
    
		/*
		 * Grab any alignment restrictions.  The DDCAPS struct contains a series of
		 * alignment fields that are not clearly defined. They are intended for
		 * overlay use.  It's important to observe alignment restrictions.
		 * Many adapters with overlay capabilities require the overlay image be
		 * located on 4 or even 8 byte boundaries, and have similar restrictions
		 * on the overlay width (for both source and destination areas).
		 * 
		 */

		uDestSizeAlign = capsDrv.dwAlignSizeDest;
	    uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;
    
	    dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    
		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;
    
		ZeroMemory(&ovfx, sizeof(DDOVERLAYFX));
		ovfx.dwSize     = sizeof(DDOVERLAYFX);

		switch(this->physicalDepth) {

		case 16:
		
			ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_16BPP;
			ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_16BPP;
    
			ovfx.dckDestColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_16BPP;
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_16BPP;
			break;

		case 24:
		case 32:
		default:

			ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_32BPP;
			ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_32BPP;
    
			ovfx.dckDestColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_32BPP;
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_32BPP;
			break;
		}

		/*
		 * Compute the destination
		 */

		pt.x = 0;
		pt.y = 0;

		ClientToScreen(this->hwndPlayback, &pt);

	    rd.left   = pt.x + rect->left; 
		rd.top    = pt.y + rect->top; 
		rd.right  = pt.x + rect->left + (rect->right  * uStretchFactor1000 + 999) / 1000;
		rd.bottom = pt.y + rect->bottom * uStretchFactor1000 / 1000;

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
			rd.right = (int)((rd.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;

		/*
		 * To setup properly the source rectangle  
		 * we need to know if we have to perform clipping
		 * at the edges of the screen...
		 *
		 */

		rs.left   = 0; 
		rs.top    = 0; 
		rs.right  = this->width;
		rs.bottom = this->height;

		if(rd.right < this->fullscreenWidth && rd.left > 0 && rd.top > 0 && rd.bottom < this->fullscreenHeight) {

		}
		else {

			if(rd.right > this->fullscreenWidth) {
	
				rs.right  = (this->fullscreenWidth - rd.left) * this->width / (rd.right - rd.left);
				rd.right = this->fullscreenWidth;
			}

			
			if(rd.left < 0) {
		
				rs.left = -rd.left * this->width / (rd.right - rd.left);
				rd.left = 0;
			}

			if(rd.bottom > this->fullscreenHeight) {
	
				rs.bottom  = (this->fullscreenHeight - rd.top) * this->height / (rd.bottom - rd.top);
				rd.bottom = this->fullscreenHeight;
			}

			
			if(rd.top < 0) {
		
				rs.top = -rd.top * this->height / (rd.bottom - rd.top);
				rd.top = 0;
			}
		}

   
		/*
		 * Apply any size alignment 
		 * restrictions if necessary.
		 *
		 */

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
			rs.right -= rs.right % uSrcSizeAlign;

		/*
		 * Now Update the Overlay
		 */

		ddrval = this->lpddsOverlay->UpdateOverlay(&rs, this->lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
    
		if(FAILED(ddrval)) {

      
			return MP_RESULT_ERROR;
		}
    
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererOverlay::DrawFullscreen(MediaBuffer *buffer, int frameNumber, int invertFlag, int desktop)
{
	if(buffer && this->lpdd && this->lpddsPrimary && this->lpddsOverlay) {


		HRESULT         ddrval;
	    RECT            rs, rd;
		DDOVERLAYFX     ovfx;
		DDSURFACEDESC2  ddsd;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000, i;
		unsigned int    uDestSizeAlign, uSrcSizeAlign;
		DWORD           dwUpdateFlags;
 
		/*
		 * First copy the image into the 
		 * Overlays backbuffer...
		 */

		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddrval = this->lpddsOverlay->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
 
		if(FAILED(ddrval)) {

			return MP_RESULT_ERROR;
		}

		if(invertFlag) {

			for(i = 0; i < ddsd.dwHeight; i++) {


				memcpy((char *) ddsd.lpSurface + i*ddsd.lPitch, ((char *) buffer->GetData()) + (this->height - i - 1)*ddsd.dwWidth*this->bpp/8, ddsd.dwWidth*this->bpp/8);
			}
		}
		else {

			if(ddsd.dwWidth == ddsd.lPitch) {

				memcpy(ddsd.lpSurface, buffer->GetData(), this->width*this->height*this->bpp/8);
			}
			else {
	
				for(i = 0; i < ddsd.dwHeight; i++) {

					memcpy((char *) ddsd.lpSurface + i*ddsd.lPitch, ((char *) buffer->GetData()) + i*ddsd.dwWidth*this->bpp/8, ddsd.dwWidth*this->bpp/8);
				}
			}
		}

		this->lpddsOverlay->Unlock(NULL);

	    /*
	     * Get driver capabilities
	  	 */

		ZeroMemory(&capsDrv, sizeof(DDCAPS));
		capsDrv.dwSize    = sizeof(DDCAPS);

		ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
		
		if (FAILED(ddrval))
			return MP_RESULT_ERROR;
    
	    /* 
		 * We need to check the minimum stretch.  Many display adpators require that
		 * the overlay be stretched by a minimum amount.  The stretch factor will 
		 * usually vary with the display mode (including changes in refresh rate).
		 * The stretch factor is returned x1000.
         * 
		 */

		uStretchFactor1000 = capsDrv.dwMinOverlayStretch > 1000 ? capsDrv.dwMinOverlayStretch : 1000;
    
		/*
		 * Grab any alignment restrictions.  The DDCAPS struct contains a series of
		 * alignment fields that are not clearly defined. They are intended for
		 * overlay use.  It's important to observe alignment restrictions.
		 * Many adapters with overlay capabilities require the overlay image be
		 * located on 4 or even 8 byte boundaries, and have similar restrictions
		 * on the overlay width (for both source and destination areas).
		 * 
		 */

		uDestSizeAlign = capsDrv.dwAlignSizeDest;
	    uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;
    
	    dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    
		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;
    
		
		ZeroMemory(&ovfx, sizeof(DDOVERLAYFX));
		ovfx.dwSize     = sizeof(DDOVERLAYFX);

		switch(this->physicalDepth) {

		case 16:
				
			ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_16BPP;
			ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_16BPP;
    
			ovfx.dckDestColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_16BPP;
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_16BPP;
			break;

		case 24:
		case 32:
		default:
	
			ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_32BPP;
			ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_32BPP;
		
			ovfx.dckDestColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORKEY_32BPP;
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORKEY_32BPP;
			break;
		}	
		
	    rs.left   = 0; 
		rs.top    = 0; 
		rs.right  = this->width;
		rs.bottom = this->height;
    
		/*
		 * Apply any size alignment 
		 * restrictions if necessary.
		 *
		 */

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
			rs.right -= rs.right % uSrcSizeAlign;
    
		/*
		 * Compute the destination
		 */

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

		    rd.left   = (this->fullscreenWidth - (this->fullscreenHeight * width / this->height)) / 2; 
			rd.top    = 0; 
			rd.right  = rd.left + (this->fullscreenHeight * width / height);
			rd.bottom = this->fullscreenHeight; 
		}
		else {	

		    rd.left   = 0; 
			rd.top    = (this->fullscreenHeight - (this->fullscreenWidth * height / width)) / 2; 
			rd.right  = this->fullscreenWidth;
			rd.bottom = rd.top + (this->fullscreenWidth * height / width); 
		}

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
			rd.right = (int)((rd.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;

		 /*
		  * Now Update the Overlay
		  */

		ddrval = this->lpddsOverlay->UpdateOverlay(&rs, this->lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
    
		if(FAILED(ddrval)) {

      
			return MP_RESULT_ERROR;
		}
    
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRendererOverlay::Close()
{
	if(this->lpdd) {
		
		this->lpdd->RestoreDisplayMode();
		this->lpdd->SetCooperativeLevel(this->hwndPlayback, DDSCL_NORMAL);
	}

	if(this->lpddsOverlay) {
	
		this->lpddsOverlay->Release();
		this->lpddsOverlay = NULL;
	}

	if(this->lpddsPrimary) {
	
		this->lpddsPrimary->Release();
		this->lpddsPrimary = NULL;
	}

	if(this->lpdd) {
	
		this->lpdd->Release();
		this->lpdd = NULL;
	}

	this->invertFlag  = FALSE;

	return MP_RESULT_ERROR;
}
