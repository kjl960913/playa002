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
 * Authors: Damien Chavarria <darc at projectmayo dot com>                            *
 *                                                                                    *
 **************************************************************************************/
#include <ddraw.h>
#include "Playa.h"
#include "Skin.h"
#include "Playlist.h"
#include "MediaPlayback.h"
#include "DirDialog.h"
#include "SkinList.h"
#include "Resizer.h"
#include "DebugFile.h"

/*
 * Ressources files
 *
 */

#ifdef WIN32
#include "../build/win32/ressources.h"
#include "../build/win32/resource.h"
#endif

#include <math.h>
#include <commctrl.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

/*
 * Intervals for the timers
 */

#define TIMER_ID     1
#define TIMER_RATE  50

/*
 * Variables
 *
 * Please regroup that in a struct...
 *
 */

BOOL      openning_network;
DWORD     anonymous;
HWND      hwndDisplay;
BOOL      screenSaverActive;
int       action  = ACTION_NONE;
DWORD     count = 0;
DWORD     use_subtitles         = 1;
DWORD     no_resize             = 0;
char      skinPath[MAX_PATH];
COLORREF  backColor;
UINT      uTimerID;
RECT      clientRect, cwindowRect;
RECT      windowRect, fullwindowRect;
int       moveX = 0, moveY = 0;
Skin     *skin;
MediaPlayback *playback;
CDirDialog *dirChooser;
SkinList   *skinList;
Resizer   *resizer;
Playlist  *playlist;
char      Name[] = "The \"Playa\"";
char     *RecentFiles[5];
HWND      hwnd, about, urlW = NULL;
HMENU     popupMenu;
HACCEL    hAccel;
DWORD     id;
HINSTANCE hInstance;
int       showing_cursor;
int       compact_mode;
BOOL      openOK;
char     *url;
DWORD     firstStart;
options_t options, tmpOptions;
WCHAR     wallpaper[MAX_PATH];
WCHAR     pattern[MAX_PATH];

/*
 * Needed definitions
 */

/*
 * The Main message loop functions
 *
 */

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void             UpdateMainWindow();

/*
 * opens the current playlist position
 *
 */

void             OpenFileForPlaying(HWND hwnd);

/*
 * Rebuilds the playlist popup menu
 *
 */

void             ReBuildPlaylistMenu();

/**************************************************************************************
 *                                                                                    *
 *                              SAVE AND LOAD OPTIONS                                 *
 *                                                                                    *
 **************************************************************************************/

void LoadOptions() {

	HKEY     key;
	DWORD created, size;
	LONG     result;
	DWORD  type, i;
	char      file[256], name[5];
	
	/*
	 * Put the default options
	 *
	 */

	compact_mode = 0;
    strcpy(skinPath, "Default");

	options.change_fullscreen_res = 0;
	options.loop                  = 1;
	options.on_top                = 1;
	options.use_bilinear          = 0;
	options.aspect_ratio          = ASPECT_RATIO_FREE;
	options.disable_screen_saver  = 1;
	options.save_pos              = 1;
	options.posX                  = 100;
	options.posY                  = 30;
	options.postprocessing        = 60;
	options.startFullscreen       = 0;

	/*
	 * Init the recent file list 
	 */

	for(i=0; i < 5; i++) {

		RecentFiles[i] = NULL;
	}

	/*
	 * Open the registry key
	 */

	result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\DivXNetworks\\ThePlaya",
 			                0, "CONFIG", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
							NULL, &key, &created);

	if(result != ERROR_SUCCESS) {

		MP_ERROR("Couldn't load options");
		return;
	}

	switch(created) {

	case REG_CREATED_NEW_KEY:

		/*
		 * First time launch (we keep the default)
		 * 
		 */

		break;

	case REG_OPENED_EXISTING_KEY:

		/*
		 * We can read the values
 		 */

		size = 4;
		result = RegQueryValueEx(key, "UseBilinear", 0, &type, (BYTE *)&options.use_bilinear, &size);
		size = 4;
		result = RegQueryValueEx(key, "MaintainAspectRatio", 0, &type, (BYTE *)&options.aspect_ratio, &size);
		size = 4;
		result = RegQueryValueEx(key, "ChangeFullscreenRes", 0, &type, (BYTE *)&options.change_fullscreen_res, &size);
		size = 4;
		result = RegQueryValueEx(key, "UseSubtitles", 0, &type, (BYTE *)&use_subtitles, &size);
		
		size = 4;
		result = RegQueryValueEx(key, "Loop", 0, &type, (BYTE *)&options.loop, &size);
		size = 4;
		result = RegQueryValueEx(key, "OnTop", 0, &type, (BYTE *)&options.on_top, &size);
		size = 4;
		result = RegQueryValueEx(key, "PreventScreenSaver", 0, &type, (BYTE *)&options.disable_screen_saver, &size);

		size = 4;
		result = RegQueryValueEx(key, "SavePlayerPos", 0, &type, (BYTE *)&options.save_pos, &size);

		size = 4;
		result = RegQueryValueEx(key, "PostProcessing", 0, &type, (BYTE *)&options.postprocessing, &size);

		size = 4;
		result = RegQueryValueEx(key, "CustomAspectX", 0, &type, (BYTE *)&aspectRatios[ASPECT_RATIO_CUSTOM].xFactor, &size);

		size = 4;
		result = RegQueryValueEx(key, "CustomAspectY", 0, &type, (BYTE *)&aspectRatios[ASPECT_RATIO_CUSTOM].yFactor, &size);

		if(options.save_pos) {
			
			size = 4;
			result = RegQueryValueEx(key, "PlayerPosX", 0, &type, (BYTE *)&options.posX, &size);
			
			size = 4;
			result = RegQueryValueEx(key, "PlayerPosY", 0, &type, (BYTE *)&options.posY, &size);
		}

		size = MAX_PATH;
		result = RegQueryValueEx(key, "SkinPath", 0, &type, (BYTE *)skinPath, &size);

		/*
		 * And the recent files
		 */

		for(i=0; i < 5; i++) {

			sprintf(name, "File%d", i+1);

			size = 256;
			result = RegQueryValueEx(key, name, 0, &type, (BYTE *)&file, &size);

			if(result == ERROR_SUCCESS) {
			
				RecentFiles[i] = (char *) new char[size];
				strncpy(RecentFiles[i], file, size);
			}
		}

		break;

	default:
		break;
	}

	RegCloseKey(key);

}

void SaveOptions() {

	HKEY     key;
	LONG     result;
	DWORD created, i;
	char      name[5];

	options.postprocessing = playback->videoDecoder->decoreDecoder->GetQuality();

	/*
	 * Try to open the registry key
	 */

	result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\DivXNetworks\\ThePlaya",
							              0, "CONFIG", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
							              NULL, &key, &created);

	if(result != ERROR_SUCCESS) {

		MP_ERROR("Couldn't save options");
		
		RegCloseKey(key);
		return;
	}

	RegSetValueEx(key, "UseBilinear", 0, REG_DWORD, (BYTE *) &options.use_bilinear, 4);
	RegSetValueEx(key, "MaintainAspectRatio", 0, REG_DWORD, (BYTE *) &options.aspect_ratio, 4);
	RegSetValueEx(key, "ChangeFullscreenRes", 0, REG_DWORD, (BYTE *) &options.change_fullscreen_res, 4);
	RegSetValueEx(key, "UseSubtitles", 0, REG_DWORD, (BYTE *) &use_subtitles, 4);
	RegSetValueEx(key, "Loop", 0, REG_DWORD, (BYTE *) &options.loop, 4);
	RegSetValueEx(key, "OnTop", 0, REG_DWORD, (BYTE *) &options.on_top, 4);
	RegSetValueEx(key, "PreventScreenSaver", 0, REG_DWORD, (BYTE *) &options.disable_screen_saver, 4);

	RegSetValueEx(key, "SavePlayerPos", 0, REG_DWORD, (BYTE *) &options.save_pos, 4);
	RegSetValueEx(key, "PostProcessing", 0, REG_DWORD, (BYTE *) &options.postprocessing, 4);

	RegSetValueEx(key, "CustomAspectX", 0, REG_DWORD, (BYTE *) &aspectRatios[ASPECT_RATIO_CUSTOM].xFactor, 4);
	RegSetValueEx(key, "CustomAspectY", 0, REG_DWORD, (BYTE *) &aspectRatios[ASPECT_RATIO_CUSTOM].yFactor, 4);

	if(options.save_pos) {

		options.posX = windowRect.left;
		options.posY = windowRect.top;

		RegSetValueEx(key, "PlayerPosX", 0, REG_DWORD, (BYTE *) &options.posX, 4);
		RegSetValueEx(key, "PlayerPosY", 0, REG_DWORD, (BYTE *) &options.posY, 4);
	}

	if(strcmp(skinPath, "Default") != 0)
		RegSetValueEx(key, "SkinPath", 0, REG_SZ, (BYTE *) skinPath, strlen(skinPath));
	else 
		RegSetValueEx(key, "SkinPath", 0, REG_SZ, (BYTE *) "Default", strlen("Default"));
	
	/*
	 * The recent file list
	 */

	for(i=0; i < 5; i++) {
		
		sprintf(name, "File%d\0", i+1);

		if(RecentFiles[i] != NULL) {
		
			RegSetValueEx(key, name, 0, REG_SZ, 
							      (BYTE *) RecentFiles[i], strlen(RecentFiles[i]));
		}
		else {
		
			RegDeleteValue(key, name);
 		}
	}

	RegCloseKey(key);
}

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the custom aspect ratio                         *
 *                        -----------------------------------                         *
 **************************************************************************************/

static BOOL APIENTRY CustomAspectDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {

	switch(message) {

		case WM_INITDIALOG:

			char buffer[256];

			sprintf(buffer, "%d", aspectRatios[ASPECT_RATIO_CUSTOM].xFactor);
			SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_X), WM_SETTEXT, 0, (LPARAM) buffer);

			sprintf(buffer, "%d", aspectRatios[ASPECT_RATIO_CUSTOM].yFactor);
			SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_Y), WM_SETTEXT, 0, (LPARAM) buffer);

			return TRUE;

	  case WM_COMMAND:

        switch (LOWORD(wParam))
		{
		case IDC_CUSTOM_ASPECT_CANCEL:
			EndDialog(hDlg, TRUE);
			break;

		case IDC_CUSTOM_ASPECT_OK:
			EndDialog(hDlg, TRUE);

			DWORD xFactor, yFactor;
			char *data;
			SHORT lineLength;
		
			lineLength = (short) SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_X), EM_LINELENGTH, 0, 0);

			/*
			 * This f***ing bas***d windows API
			 * wants the size of the url buffer
			 * to be written into the first 2 bytes...
			 *
			 */

			data = (char *) new char[lineLength + 1];
			memcpy(data, &lineLength, 2);
	
			SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_X), EM_GETLINE, 0, (LONG)(LPVOID)data);
			data[lineLength] = '\0';

			sscanf(data, "%d", &xFactor);

			lineLength = (short) SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_Y), EM_LINELENGTH, 0, 0);

			/*
			 * This f***ing bas***d windows API
			 * wants the size of the url buffer
			 * to be written into the first 2 bytes...
			 *
			 */

			data = (char *) new char[lineLength + 1];
			memcpy(data, &lineLength, 2);
	
			SendMessage(GetDlgItem(hDlg, IDC_CUSTOM_ASPECT_Y), EM_GETLINE, 0, (LONG)(LPVOID)data);
			data[lineLength] = '\0';

			sscanf(data, "%d", &yFactor);

			aspectRatios[ASPECT_RATIO_CUSTOM].xFactor = xFactor;
			aspectRatios[ASPECT_RATIO_CUSTOM].yFactor = yFactor;

			/*
			 * Now the real stuff
			 */
			switch(options.aspect_ratio) {

				case ASPECT_RATIO_ORIGINAL:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_TV:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_FREE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_WIDE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_UNCHECKED);
					break;
			}

			options.aspect_ratio = ASPECT_RATIO_CUSTOM;
			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_CHECKED);
				
			RECT  src;

			GetClientRect(hwnd, &src);
			GetWindowRect(hwnd, &windowRect);

			if(compact_mode) {

				src.bottom = (src.right - src.left)*yFactor/xFactor;
			}
			else {

				src.bottom = (src.right - src.left - 15)*yFactor/xFactor + 115 + 22;
			}

			AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

			MoveWindow( hwnd, windowRect.left, 
						windowRect.top, 
						src.right - src.left, 
						src.bottom - src.top, TRUE);
				
			playback->SetVideoRect(skin->GetVideoRect());

			break;
		}

		case WM_DESTROY:
			return TRUE;
	}

	return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the General Preferences                         *
 *                        -----------------------------------                         *
 **************************************************************************************/


static BOOL APIENTRY PreferencesGeneralDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {

	switch(message) {
		case WM_INITDIALOG:

			SetWindowPos(hDlg, HWND_TOP, 15, 35, 0, 0, SWP_NOSIZE);

		    CheckDlgButton(hDlg, IDC_CHECK_LOOP,   tmpOptions.loop);
		    CheckDlgButton(hDlg, IDC_CHECK_ON_TOP, tmpOptions.on_top);
		    CheckDlgButton(hDlg, IDC_CHECK_DISABLE_SS, tmpOptions.disable_screen_saver);
		    CheckDlgButton(hDlg, IDC_CHECK_SAVE_POS, tmpOptions.save_pos);

			return TRUE;

	  case WM_COMMAND:

        switch (wParam)
		{
			case IDC_CHECK_LOOP:

				tmpOptions.loop = tmpOptions.loop ? 0 : 1;
				break;

			case IDC_CHECK_ON_TOP:

				tmpOptions.on_top = tmpOptions.on_top ? 0 : 1;
				break;

			case IDC_CHECK_DISABLE_SS:

				tmpOptions.disable_screen_saver = tmpOptions.disable_screen_saver ? 0 : 1;
				break;

			case IDC_CHECK_SAVE_POS:

				tmpOptions.save_pos = tmpOptions.save_pos ? 0 : 1;
				break;
		}

		case WM_DESTROY:
			return TRUE;
	}

	return FALSE;
}

/*
 * List of the preferences tab
 *
 */

static struct prefsTabs {
	LPTSTR	rsrc;
	char	*name;
	DLGPROC	dProc;
} tabs[]={
	{	MAKEINTRESOURCE(IDD_PREFS_GENERAL),	"General",	PreferencesGeneralDlgProc},
};

/*
 * Main preferences DlgProc
 *
 */

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the Main Preferences                            *
 *                        --------------------------------                            *
 **************************************************************************************/

static BOOL APIENTRY PreferencesDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {

	DWORD i;

	switch(message) {

	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
		{
			EndDialog (hDlg, TRUE);
			return (TRUE);
		}
		break;
	
	case WM_INITDIALOG:
			{
				HWND tab = GetDlgItem(hDlg, IDC_TAB);

				memcpy(&tmpOptions, &options, sizeof(options_t));

				for(i = 0; i < (sizeof tabs/sizeof tabs[0]); i++) {
					
					TC_ITEM ti;

					ti.mask		= TCIF_TEXT;
					ti.pszText	= tabs[i].name;

					TabCtrl_InsertItem(tab, i, &ti);
				}

				hwndDisplay = CreateDialogParam(hInstance, tabs[0].rsrc, hDlg, tabs[0].dProc, (LPARAM)NULL);
				ShowWindow(hwndDisplay, SW_SHOW);

				return TRUE;
			}
		
		case WM_NOTIFY: {
			NMHDR *nm = (LPNMHDR)lParam;

			switch(nm->code) {
			case TCN_SELCHANGE:
				{
					int iTab = TabCtrl_GetCurSel(nm->hwndFrom);

					if (iTab>=0) {
						if (hwndDisplay) DestroyWindow(hwndDisplay);
							hwndDisplay = CreateDialogParam(hInstance, tabs[iTab].rsrc, hDlg, tabs[iTab].dProc, NULL);
					}

					ShowWindow(hwndDisplay, SW_SHOW);
				}
				return TRUE;
			}
			}
			break;


	  case WM_COMMAND:

        switch (wParam)
		{
		  case IDC_PREFS_OK:
		
			  EndDialog(hDlg, TRUE);
			  
			  /*
			   * Save the options changes
			   */
				
			  memcpy(&options, &tmpOptions, sizeof(options_t));

			  /*
			   * And apply them
			   */

			  CheckMenuItem(popupMenu, (UINT)ID_LOOP, options.loop ? MF_CHECKED : MF_UNCHECKED);
		  	  CheckMenuItem(popupMenu, (UINT)ID_ON_TOP, options.on_top ? MF_CHECKED : MF_UNCHECKED);
		  	  CheckMenuItem(popupMenu, (UINT)ID_SCREENSAVER, options.disable_screen_saver ? MF_CHECKED : MF_UNCHECKED);

			  /*
			   * and Apply them
			   */

			  options.on_top = options.on_top ? 0 : 1;
			  SendMessage(hwnd, WM_COMMAND, (WPARAM)ID_ON_TOP, 0);

			  playback->SetLoop(options.loop);
			  
			  /*
			   * And end the dialog
			   */
			  
			  return TRUE;
			  break;

		  case IDC_PREFS_CANCEL:

			  EndDialog(hDlg, TRUE);
			  break;

		  default:
			  return 0;
		}
		break;


		case WM_DESTROY:
			return TRUE;
	}

	return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the playback properties                         *
 *                        -----------------------------------                         *
 **************************************************************************************/

static BOOL APIENTRY PlaybackPropertiesDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {

	switch(message) {
		case WM_INITDIALOG:
			
			if(playback->HasVideo()) {
			
				if(playback->input)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->input->GetName());
				
				if(playback->decaps)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->decaps->GetName());
				
				if(playback->videoDecoder)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->videoDecoder->GetName());
				
				if(playback->videoBuffer)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->videoBuffer->GetName());
				
				if(playback->videoRenderer)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->videoRenderer->GetName());
			}

			if(playback->HasAudio()) {
			
				if(playback->audioDecoder)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->audioDecoder->GetName());
				
				if(playback->audioRenderer)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->audioRenderer->GetName());
			}
			
			if(playback->hasSubtitles) {

				if(playback->subtitler)
					SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_ADDSTRING, 0, (LONG) playback->subtitler->GetName());
			}

			return TRUE;

		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE)
			{
				EndDialog (hDlg, TRUE);
				return (TRUE);
			}
			break;

		case WM_COMMAND:

	        switch (LOWORD(wParam))
			{
				case ID_PLAYBACK_CLOSE	:
				
					EndDialog (hDlg, TRUE);
					break;

				case IDC_PLAYBACK_CONFIGURE:
					{
						  int nItem;
						  nItem = SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_GETCURSEL, 0, 0); 

						  switch(nItem) {

						  case 0:
							  playback->input->Configure(hInstance, hDlg);
							  break;

						  case 1:
							  playback->decaps->Configure(hInstance, hDlg);
							  break;
						  
						  case 2:

							  playback->videoDecoder->Configure(hInstance, hDlg);
							  break;
						  
						  case 3:
							  playback->videoBuffer->Configure(hInstance, hDlg);
							  break;
						  
						  case 4:
							  playback->videoRenderer->Configure(hInstance, hDlg);
							  break;
						  
						  case 5:
							  playback->audioDecoder->Configure(hInstance, hDlg);
							  break;
						  
						  case 6:
							  playback->audioRenderer->Configure(hInstance, hDlg);
							  break;
						  
						  case 7:
							  playback->subtitler->Configure(hInstance, hDlg);
							  break;
						  }
					}
					break;

				case IDC_PLAYBACK_LIST:

					switch (HIWORD(wParam)) { 
                  
					  case LBN_SELCHANGE:
						
						  int nItem;
						  nItem = SendMessage(GetDlgItem(hDlg, IDC_PLAYBACK_LIST), LB_GETCURSEL, 0, 0); 

						  switch(nItem) {

						  case 0:
							  if(playback->input->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;

						  case 1:
							  if(playback->decaps->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;
						  
						  case 2:

							  if(playback->videoDecoder->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }

							  break;
						  
						  case 3:
							  if(playback->videoBuffer->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;
						  
						  case 4:
							  if(playback->videoRenderer->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;
						  
						  case 5:
							if(playback->audioDecoder->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;
						  
						  case 6:
							  if(playback->audioRenderer->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;
						  
						  case 7:
							  if(playback->subtitler->GetCaps() & MEDIA_CAPS_CAN_CONFIGURE) {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), TRUE);
							  }
							  else {

						 		  EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_CONFIGURE), FALSE);
							  }
							  break;

						  }
						  
						  break;
					}
					break;
			}
			break;

		case WM_DESTROY:
			return TRUE;
	}

	return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the file information dlg                        *
 *                        ------------------------------------                        *
 **************************************************************************************/

int APIENTRY PropDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{
  switch (wMsg)
    {
      case WM_INITDIALOG:

		  char freq[20];
		  char channels[2];
		  char video_size[50];
		  char duration[20];
		  char total_size[50];

		  DWORD time, hours, minutes, seconds;

		  if(playback->HasAudio() && playback->audioDecoder) {
			  _ultoa(playback->audioDecoder->GetAudioFormat()->nSamplesPerSec, freq, 10);
			  _ultoa(playback->audioDecoder->GetAudioFormat()->nChannels, channels, 10);
		  }

		  sprintf(video_size, "%dx%d", playback->GetVideoWidth(), playback->GetVideoHeight());
		  
		  if(playback->input)
			  sprintf(total_size, "%d bytes", playback->input->GetSize());

		  time = playback->GetTotalTime();

		  hours   = time/3600;
		  minutes = (time - hours*3600)/60;
		  seconds = (time - hours*3600 - minutes*60)/60;

		  sprintf(duration, "%.2d:%.2d:%.2d", hours, minutes, seconds);

		  
		  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME),  playback->GetFilename());
		  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILE_TYPE), playback->input->GetName());
		  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILE_SIZE), total_size);
		  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_DURATION),  duration);
		  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_DECAPS),  playback->decaps->GetName());

		  if(playback->HasVideo()) {

			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_CODEC), playback->videoDecoder->GetName());
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_SIZE), video_size);

			  switch(playback->videoRenderer->GetVideoMode()) {

				  case VIDEO_MODE_RGB16:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "RGB 16 bpp");
					  break;

				  case VIDEO_MODE_RGB24:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "RGB 24 bpp");
					  break;

				  case VIDEO_MODE_RGB32:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "RGB 32 bpp");
					  break;
				  case VIDEO_MODE_YUY2:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "YUY2 Overlay");
					  break;
				  case VIDEO_MODE_YUV12:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "YUV 12 Overlay");
					  break;
				  case VIDEO_MODE_UYVY:
					  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VIDEO_MODE), "UYVY Overlay");
					  break;
			  }

			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_VRENDERER), playback->videoRenderer->GetName());
		  }

		  if(playback->hasAudio) {
			  		  
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_CODEC), playback->audioDecoder->GetName());
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_FREQUENCY), freq);
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_CHANNELS), channels);
		  }
		  else {

			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_CODEC),     "Not Available");
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_FREQUENCY), "Not Available");
			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_AUDIO_CHANNELS),  "Not Available");
		  }

		  if(playback->hasSubtitles) {

			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SUB), playback->subtitler->GetName());
		  }
		  else {

			  SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SUB),  "Not Available");
		  }

		  return (0);
		break;

      case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
          {
          EndDialog (hDlg, TRUE);
          return (TRUE);
          }
        break;

	  case WM_COMMAND:

        switch (wParam)
		{
		  case IDB_PROPERTIES_OK:
			
			  EndDialog(hDlg, TRUE);
			  return TRUE;
			  break;

		  default:
			  return 0;
		}
		break;
	}

  return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                           DlgProc for the About Dialog                             *
 *                        -----------------------------------                         *
 **************************************************************************************/

int APIENTRY AboutDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{
  switch (wMsg)
    {
      case WM_INITDIALOG:
        return (0);
		break;

      case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
          {
          EndDialog (hDlg, TRUE);
          return (TRUE);
          }
        break;

	  case WM_COMMAND:

        switch (wParam)
		{
		  case IDB_ABOUT_OK:
			
			  EndDialog(hDlg, TRUE);
			  return TRUE;
			  break;

		  default:
			  return 0;
		}
		break;
	}

  return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                        DlgProc for the Skin Browser Dialog                         *
 *                        -----------------------------------                         *
 **************************************************************************************/

int APIENTRY SkinBrowserDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{
  switch (wMsg)
    {
      case WM_INITDIALOG:

		  int /*nItem, */i;
		  
		  /*
		   * Init the list of skins
		   */
			

		  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_ADDSTRING, 0, (LONG) "[Default Skin]");
		  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_SETCURSEL, 0, 0); 
		  
		  if(skinList->skinsDir != NULL) {

			  SendMessage(GetDlgItem(hDlg, IDC_COMBO_DIR), CB_ADDSTRING, 0, (LONG)skinList->skinsDir);
		      SendMessage(GetDlgItem(hDlg, IDC_COMBO_DIR), CB_SETCURSEL, 0, 0);
		  }

		  skinList->Scan();

		  for(i=0; i < skinList->getNumberOfSkins(); i++) {
		  
			  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_ADDSTRING, 0, (LONG) skinList->getSkinInfo(i)->name);

			  if(strcmp(skinList->getSkinInfo(i)->directory, skinPath) == 0) {

				  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_SETCURSEL, i+1, 0); 
			  }
		  }

		  return (0);
		break;

      case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
          {
          EndDialog (hDlg, TRUE);
          return (TRUE);
          }
        break;

	  case WM_COMMAND:

	    switch (LOWORD(wParam))  {
		  
          case IDC_SKIN_LIST: 
              
			  switch (HIWORD(wParam)) { 
                  
			  case LBN_SELCHANGE:
				  
				  int nItem;

				  nItem = SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_GETCURSEL, 0, 0); 

				  if(nItem == 0) {
					  
					  RECT rect;

					  skin->LoadDefault(hInstance, hwnd);
					  strcpy(skinPath, "Default");

					  UpdateMainWindow();
				  }
				  else {
					  
					  RECT rect;

					  skin->Load(skinList->getSkinInfo(nItem - 1)->directory, hwnd);
					  strcpy(skinPath, skinList->getSkinInfo(nItem  - 1)->directory);

					  UpdateMainWindow();
				  }

				  break;
			  }
		}

        switch (wParam)
		{
		  case IDB_SKINS_OK:
			
			  EndDialog(hDlg, TRUE);
			  return TRUE;
			  break;
		  
		  case ID_SKINS_CANCEL:
			
			  EndDialog(hDlg, TRUE);
			  return TRUE;
			  break;

		  case IDC_CHANGE_DIR:

			  dirChooser = new CDirDialog();

			  if(dirChooser->DoBrowse()) {
				
				skinList->SetDir(dirChooser->m_strPath);

				SendMessage(GetDlgItem(hDlg, IDC_COMBO_DIR), CB_RESETCONTENT, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_RESETCONTENT, 0, 0);

				SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_ADDSTRING, 0, (LONG) "[Default Skin]");
				SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_SETCURSEL, 0, 0); 
			
				if(skinList->skinsDir != NULL) {
	
				  SendMessage(GetDlgItem(hDlg, IDC_COMBO_DIR), CB_ADDSTRING, 0, (LONG)skinList->skinsDir);
			      SendMessage(GetDlgItem(hDlg, IDC_COMBO_DIR), CB_SETCURSEL, 0, 0);
			  }

			  skinList->Scan();

			  for(i=0; i < skinList->getNumberOfSkins(); i++) {
			  
				  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_ADDSTRING, 0, (LONG) skinList->getSkinInfo(i)->name);

				  if(strcmp(skinList->getSkinInfo(i)->directory, skinPath) == 0) {
	
					  SendMessage(GetDlgItem(hDlg, IDC_SKIN_LIST), LB_SETCURSEL, 0, 0); 
				  }
			  }	
			
			  }
			  break;

		  default:
			  return 0;
		}
		break;
	}

  return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                          DlgProc for the Open URL Dialog                           *
 *                        -----------------------------------                         *
 **************************************************************************************/

int APIENTRY UrlDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{

  switch (wMsg)
    {
      case WM_INITDIALOG:

		  SendMessage(GetDlgItem(hDlg, IDC_RADIO_HTTP), BM_SETCHECK, BST_CHECKED, 0);

		  EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_USERNAME), FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_PASSWORD), FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ANONYMOUS),     FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_SERVER),   FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FTP),           FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USERNAME),       FALSE);
		  EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASSWORD),       FALSE);

		  return (0);
		break;

      case WM_SYSCOMMAND:

        if (wParam == SC_CLOSE) {

            EndDialog (hDlg, TRUE);
            return (TRUE);
        }
        break;

	  case WM_COMMAND:

        switch (wParam)
		{

		case IDC_RADIO_HTTP:

		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_USERNAME), FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_PASSWORD), FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ANONYMOUS),     FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_SERVER),   FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FTP),           FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USERNAME),       FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASSWORD),       FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_EXAMPLE),             TRUE);
			
		    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_HTTP),  TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HTTP), TRUE);
			break;

		case IDC_RADIO_FTP:

		    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ANONYMOUS),     TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_SERVER),   TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FTP),    TRUE);
			
		    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_HTTP),  FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HTTP), FALSE);
		    EnableWindow(GetDlgItem(hDlg, IDC_EXAMPLE),     FALSE);

			anonymous = 1;
		    CheckDlgButton(hDlg, IDC_CHECK_ANONYMOUS, TRUE);
			break;

		case IDC_CHECK_ANONYMOUS:

			anonymous = anonymous ? 0 : 1;

			if(anonymous) {

			    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_USERNAME), FALSE);
			    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_PASSWORD), FALSE);
			    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USERNAME),       FALSE);
			    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASSWORD),       FALSE);
			}
			else {

			    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_USERNAME), TRUE);
			    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FTP_PASSWORD), TRUE);
			    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USERNAME),       TRUE);
			    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASSWORD),       TRUE);
			}

			break;

		case IDB_URL_OK:
			
			EndDialog(hDlg, TRUE);

			RECT rect;

			GetClientRect(hwnd, &rect);
			InvalidateRect(hwnd, &rect, TRUE); 
			UpdateWindow(hwnd);

			openOK = FALSE;

			if(SendMessage(GetDlgItem(hDlg, IDC_RADIO_HTTP), BM_GETCHECK, 0, 0) == BST_CHECKED) {

				SHORT lineLength;

				/*
				 * HTTP
				 */

				lineLength = (short) SendMessage(GetDlgItem(hDlg, IDC_EDIT_HTTP), EM_LINELENGTH, 0, 0);

				/*
				 * This f***ing bas***d windows API
				 * wants the size of the url buffer
				 * to be written into the first 2 bytes...
				 *
				 */

				url = (char *) new char[lineLength + 1];
				memcpy(url, &lineLength, 2);

				SendMessage(GetDlgItem(hDlg, IDC_EDIT_HTTP), EM_GETLINE, 0, (LONG)(LPVOID)url);
				url[lineLength] = '\0';

				if(strstr(url, "http://") != NULL || strstr(url, "HTTP://") != NULL) {

					openOK = TRUE;
					return 0;
				}
			}
			else {

				SHORT lineLength;

				/*
				 * FTP
				 */

				lineLength = (short) SendMessage(GetDlgItem(hDlg, IDC_RADIO_HTTP), EM_LINELENGTH, 0, 0);

				/*
				 * This f***ing bas***d windows API
				 * wants the size of the url buffer
				 * to be written into the first 2 bytes...
				 *
				 */

				url = (char *) new char[lineLength + 1];
				memcpy(url, &lineLength, 2);

				SendMessage(GetDlgItem(hDlg, IDC_EDIT_HTTP), EM_GETLINE, 0, (LONG)(LPVOID)url);
				url[lineLength] = '\0';

				if(strstr(url, "ftp://") != NULL || strstr(url, "FTP://") != NULL) {

					if(anonymous) {
						
						openOK = TRUE;
						return 0;
					}
					else {

						/*
						 * Not implemented yet.
						 *
						 */

					}
				}
			}
			  
			return TRUE;
			break;

		case IDB_URL_CANCEL:
			
			EndDialog(hDlg, TRUE);
			return TRUE;
			break;

		default:
		  return 0;
		}
		break;
	}

  return FALSE;
}

/**************************************************************************************
 *                                                                                    *
 *                                    CLEANUP                                         *
 *                                    -------                                         *
 **************************************************************************************/

void Cleanup()
{
	DWORD i;

	for(i=0; i < 5; i++) {
		
		if(RecentFiles[i] != NULL) {

			free(RecentFiles[i]);
		}
	}

	DestroyMenu(popupMenu);
	CoUninitialize();

	delete playback;
	delete skinList;
	delete resizer;
}

/**************************************************************************************
 *                                                                                    *
 * - ParseCmdLine():                                                                  *
 *                                                                                    *
 *   -- Parses the command line and override options                                  *
 *                                                                                    *
 **************************************************************************************/

void ParseCmdLine(char *lpszCmdParam)
{
	char *token;
	char *arg1;

	token = lpszCmdParam;

	while(*token != '\0' && (*token == ' ' || *token == '\t'))
		token++;

	while(*token != '\0') {

		if(*token == '/') {

			token++;
			if(*token == 'f') {

				options.startFullscreen = 1;
				token++;
			}

			continue;
		}

		if(*token == '"') {

			arg1 = ++token;

			while(*token != '\0' && *token != '"') {

				token++;
			}

			*token++ = '\0';

			playlist->AddItem(arg1);
		}

		token++;
	}
}

/**************************************************************************************
 *                                                                                    *
 * - ReBuildRecentFilesMenu():                                                        *
 *                                                                                    *
 *   -- Parses the command line and override options                                  *
 *                                                                                    *
 **************************************************************************************/

void ReBuildRecentFilesMenu()
{
   HMENU             menu;
   MENUITEMINFO      itemInfo;
   DWORD             i,count;
   char              buffer[MAX_PATH + 4];

   menu = GetSubMenu(popupMenu, 0);
   menu = GetSubMenu(menu, 5);

   if(RecentFiles[0] != NULL) {

	   count = GetMenuItemCount(menu);

	   for(i=0; i<count; i++)
		   RemoveMenu(menu, 0, MF_BYPOSITION);
   
	   /*
		 * Now add the files
		 */
   
	   for(i=0; i < 5; i++) {
	   
		   if(RecentFiles[i] != NULL) {

				sprintf(buffer, "%d - ", i+1);
				strcat(buffer, RecentFiles[i]);

			   itemInfo.cbSize = sizeof(MENUITEMINFO);
			   itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
			   itemInfo.fType  = MFT_STRING;
			   itemInfo.dwTypeData = buffer;
			   itemInfo.cch = strlen(buffer);
			   itemInfo.wID = ID_RECENT_FILE1 + i;
		   
			   InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);
			}
	   }
    itemInfo.cbSize = sizeof(MENUITEMINFO);
    itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
    itemInfo.fType  = MFT_SEPARATOR;
    itemInfo.dwTypeData = "";
    itemInfo.cch = strlen("");
    itemInfo.wID = ID_RECENT_FILE1 + i;
		   
	InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);

    itemInfo.cbSize = sizeof(MENUITEMINFO);
    itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
    itemInfo.fType  = MFT_STRING;
    itemInfo.dwTypeData = "Clear list";
	itemInfo.cch = strlen("Clear list");
	itemInfo.wID = ID_CLEAR_LIST;
		   
	InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);

   }
   else {

	   count = GetMenuItemCount(menu);

	   for(i=0; i<count; i++)
		   RemoveMenu(menu, 0, MF_BYPOSITION);

	   itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
    itemInfo.fType  = MFT_STRING;
    itemInfo.dwTypeData = "No recent files...";
    itemInfo.cch = strlen("No recent files...");
    itemInfo.wID = ID_RECENT_FILE1;
		   
	InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);
	EnableMenuItem(menu, ID_RECENT_FILE1, MF_GRAYED);
   }

}

/**************************************************************************************
 *                                                                                    *
 * - UpdateRecentFilesMenu():                                                         *
 *                                                                                    *
 *   -- Add (if OK) an entry to the file list                                         *
 *                                                                                    *
 **************************************************************************************/

void UpdateRecentFilesMenu(char *filename)
{
	DWORD i;
	DWORD insert = 1;

	for(i=0; i < 5; i++) {
				
		if(RecentFiles[i] != NULL && strcmp(RecentFiles[i], filename) == 0) {
			insert = 0;
		}
	}

	if(insert) {
		
		for(i=4; i > 0; i--) {
				
			RecentFiles[i] = RecentFiles[i - 1];
		}

		RecentFiles[0] = (char *) new char[(strlen(filename) + 1)];
		strncpy(RecentFiles[0], filename, strlen(filename));
		RecentFiles[0][strlen(filename)] = '\0';
	}
}


/**************************************************************************************
 *                                                                                    *
 * - PreparesDesktopMode():                                                           *
 *                                                                                    *
 *   -- Do the setup for the desktop mode                                             *
 *                                                                                    *
 **************************************************************************************/

void PrepareDesktopMode() {

	HRESULT hr;
	IActiveDesktop *pActiveDesktop;

	/*
	 * Active Desktop Stuff
	 * --------------------
	 */

	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
					      IID_IActiveDesktop, (void**)&pActiveDesktop);


	if(hr == S_OK) {

		wchar_t *wstr;

		pActiveDesktop->GetWallpaper((LPWSTR) &wallpaper, MAX_PATH, 0);
		pActiveDesktop->GetPattern((LPWSTR) &pattern, MAX_PATH, 0);

		wstr = (wchar_t *) new char[2*MAX_PATH];
		mbtowc(wstr, "None", MAX_PATH);
		pActiveDesktop->SetWallpaper((WCHAR *) wstr, 0);
		pActiveDesktop->SetPattern((WCHAR *) wstr, 0);
	
		pActiveDesktop->ApplyChanges(AD_APPLY_ALL);

		pActiveDesktop->Release();
	}

	/*
	 * Standard Stuff
	 * --------------
	 */

	COLORREF colorref;
	INT      background;

	background = COLOR_DESKTOP;
	colorref   = DD_OVERLAY_COLORREF;

	/*
	 * Get the Settings
	 */

	backColor = GetSysColor(COLOR_DESKTOP);
				
	/*
	 * And set ours
	 */

	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "", 0);
	SetSysColors(1, &background, &colorref);
}

/**************************************************************************************
 *                                                                                    *
 * - UnPreparesDesktopMode():                                                         *
 *                                                                                    *
 *   -- UnDo the setup for the desktop mode                                           *
 *                                                                                    *
 **************************************************************************************/

void UnPrepareDesktopMode() {

	HRESULT hr;
	IActiveDesktop *pActiveDesktop;

	/*
	 * Active Desktop Stuff
	 * --------------------
	 */

	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
					      IID_IActiveDesktop, (void**)&pActiveDesktop);


	if(hr == S_OK) {

		pActiveDesktop->SetWallpaper(wallpaper, 0);
		pActiveDesktop->SetPattern(pattern, 0);
	
		pActiveDesktop->ApplyChanges(AD_APPLY_ALL);

		pActiveDesktop->Release();
	}

	/*
	 * API stuff
	 */

	INT background = COLOR_DESKTOP;
		
	SetSysColors(1, &background, &backColor);
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, 0);

	playback->SetDesktopMode(FALSE);
}

/**************************************************************************************
 *                                                                                    *
 * - PrepareCompactMode():                                                            *
 *                                                                                    *
 *   -- Do the setup for the desktop mode                                             *
 *                                                                                    *
 **************************************************************************************/

void PrepareCompactMode() {

	HMENU        menu;
	MENUITEMINFO itemInfo;

	menu = GetSubMenu(popupMenu, 0);

	memset(&itemInfo, 0, sizeof(MENUITEMINFO));
	
	itemInfo.cbSize     = sizeof(MENUITEMINFO);
	itemInfo.fType      = MFT_STRING;
	itemInfo.fMask      = MIIM_TYPE;
	itemInfo.dwTypeData = "&Standard Mode\tAlt+C";
		
	SetMenuItemInfo(menu, ID_COMPACT, FALSE, &itemInfo);
}

/**************************************************************************************
 *                                                                                    *
 * - UnPrepareCompactMode():                                                          *
 *                                                                                    *
 *   -- UnDo the setup for the desktop mode                                           *
 *                                                                                    *
 **************************************************************************************/

void UnPrepareCompactMode() {

	HMENU        menu;
	MENUITEMINFO itemInfo;
	
	compact_mode = 0;
	skin->SetCompact(FALSE);

	/*
	 * Change the menu item text
	 */	

	menu = GetSubMenu(popupMenu, 0);

	memset(&itemInfo, 0, sizeof(MENUITEMINFO));

	itemInfo.cbSize     = sizeof(MENUITEMINFO);
	itemInfo.fType      = MFT_STRING;
	itemInfo.fMask      = MIIM_TYPE;
	itemInfo.dwTypeData = "&Compact Mode\tAlt+C";

	SetMenuItemInfo(menu, ID_COMPACT, FALSE, &itemInfo);

	/*
	 * And resize the window
	 */

	GetWindowRect(hwnd, &windowRect);
	
	MoveWindow( hwnd, windowRect.left - 7, 
				windowRect.top - 22, 
				cwindowRect.right - cwindowRect.left, 
				cwindowRect.bottom - cwindowRect.top, TRUE);
				
	playback->SetVideoRect(skin->GetVideoRect());
}

/**************************************************************************************
 *                                                                                    *
 * - ChangeMenuForNormalMode():                                                       *
 *                                                                                    *
 *   -- Re-Creates the playlist popup menu                                            *
 *                                                                                    *
 **************************************************************************************/

void ChangeMenuForNormalMode() 
{
	HMENU menu;

	menu = GetSubMenu(popupMenu, 0);

	/*
	 * The subtitles and Properties menus
	 */

	MENUITEMINFO itemInfo;
				
	/*
	 * Re-update the menu items
	 */

	EnableMenuItem(popupMenu, ID_COMPACT, MF_ENABLED);
	EnableMenuItem(popupMenu, ID_FULLSCREEN, MF_ENABLED);

	EnableMenuItem(popupMenu, (UINT)ID_ON_TOP,      MF_ENABLED);

	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_ENABLED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_FREE,     MF_ENABLED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_43,       MF_ENABLED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_169,      MF_ENABLED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM,   MF_ENABLED);

	menu = GetSubMenu(popupMenu, 0);

	memset(&itemInfo, 0, sizeof(MENUITEMINFO));
	
	itemInfo.cbSize     = sizeof(MENUITEMINFO);
	itemInfo.fType      = MFT_STRING;
	itemInfo.fMask      = MIIM_TYPE;
	itemInfo.dwTypeData = "&Desktop mode\tAlt+D";
		
	SetMenuItemInfo(menu, ID_DESKTOP, FALSE, &itemInfo);

	EnableMenuItem(popupMenu, (UINT)ID_PROPERTIES,      MF_ENABLED);
	EnableMenuItem(popupMenu, (UINT)ID_DESKTOP,         MF_ENABLED);
}

/**************************************************************************************
 *                                                                                    *
 * - ChangeMenuForFullscreenlMode():                                                  *
 *                                                                                    *
 *   -- Re-Creates the playlist popup menu                                            *
 *                                                                                    *
 **************************************************************************************/

void ChangeMenuForFullscreenMode() 
{
	HMENU        menu;
	MENUITEMINFO itemInfo;

	GetWindowRect(hwnd, &fullwindowRect);

	/*
	 * Disbale some menu items
	 */

	EnableMenuItem(popupMenu, ID_COMPACT, MF_GRAYED);
	EnableMenuItem(popupMenu, ID_FULLSCREEN, MF_GRAYED);
	
	EnableMenuItem(popupMenu, (UINT)ID_ON_TOP,      MF_GRAYED);

	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_GRAYED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_FREE,     MF_GRAYED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_43,       MF_GRAYED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_169,      MF_GRAYED);
	EnableMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM,   MF_GRAYED);

	menu = GetSubMenu(popupMenu, 0);

	memset(&itemInfo, 0, sizeof(MENUITEMINFO));
	
	itemInfo.cbSize     = sizeof(MENUITEMINFO);
	itemInfo.fType      = MFT_STRING;
	itemInfo.fMask      = MIIM_TYPE;
	itemInfo.dwTypeData = "Go back to normal mode\tAlt+Enter";
		
	SetMenuItemInfo(menu, ID_DESKTOP, FALSE, &itemInfo);
}

/**************************************************************************************
 *                                                                                    *
 * - ReBuildPlaylistMenu():                                                           *
 *                                                                                    *
 *   -- Re-Creates the playlist popup menu                                            *
 *                                                                                    *
 **************************************************************************************/

void ReBuildPlaylistMenu()
{
	HMENU menu;
	DWORD i;
	DWORD count;
	MENUITEMINFO itemInfo;

	menu = GetSubMenu(GetSubMenu(GetSubMenu(popupMenu, 0), 9), 0);

	count = GetMenuItemCount(menu);

	for(i=0; i < count; i++) {
						
		RemoveMenu(menu, 0, MF_BYPOSITION);				
	}

	/*
	 * Add the normal items
	 *
	 */

	itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
	itemInfo.fType  = MFT_STRING;
	itemInfo.dwTypeData = "Add Files...\tAlt+A";
	itemInfo.cch = strlen("Add Files...\tAlt+A");
	itemInfo.wID = ID_MENU_PLAYBACK_FILES_ADDFILES;
		   
	InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);

	itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask  = MIIM_DATA | MIIM_TYPE | MIIM_ID;
	itemInfo.fType  = MFT_SEPARATOR;
	itemInfo.dwTypeData = "-";
	itemInfo.cch = strlen("-");
	itemInfo.wID = 0;
		   
	InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);


	/*
	 * Now add the files
	 */
   
    for(i=0; i < playlist->GetItemCount(); i++) {
	   
		char *name;

		if(strrchr(playlist->GetItemAt(i)->filename, '\\') != NULL) {

			name = strrchr(playlist->GetItemAt(i)->filename, '\\') + 1;
		}
		else {

			name = playlist->GetItemAt(i)->filename;
		}

	  itemInfo.cbSize     = sizeof(MENUITEMINFO);
	  itemInfo.fMask      = MIIM_DATA | MIIM_TYPE | MIIM_ID;
	  itemInfo.fType      = MFT_STRING;
	  itemInfo.dwTypeData = name;
	  itemInfo.cch        = strlen(name);
	  itemInfo.wID        = ID_PLAYLIST1 + i;
	  itemInfo.dwItemData = i;
		   
	  InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &itemInfo);
   }

	/*
	 * And select the current one
	 *
	 */

	CheckMenuItem(menu, ID_PLAYLIST1 + playlist->GetCurrentPosition(), MF_CHECKED);
}

/**************************************************************************************
 *                                                                                    *
 * - UpdateMainWindow():                                                              *
 *                                                                                    *
 *   -- Re-Paint the main window                                                      *
 *                                                                                    *
 **************************************************************************************/

void UpdateMainWindow()
{
	RECT rect;

	GetClientRect(hwnd, &rect);
	InvalidateRect(hwnd, &rect, TRUE); 
	UpdateWindow(hwnd);
}

/**************************************************************************************
 *                                                                                    *
 *                                      QUIT                                          *
 *                                    --------                                        *
 **************************************************************************************/

void Quit() 
{
	SaveOptions();
	playback->Close();

	if(playback->desktopMode) {

		UnPrepareDesktopMode();
	}
		
	if(options.disable_screen_saver) {

		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, screenSaverActive, NULL, 0);
	}

	Cleanup();

	PostQuitMessage(0);
}

/**************************************************************************************
 *                                                                                    *
 * - WinMain():                                                                       *
 *                                                                                    *
 *   -- Program Entry Point                                                           *
 *                                                                                    *
 **************************************************************************************/

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{ 
	POINT     pt;
	POINT     pt2;
    MSG       Msg;
    WNDCLASS  W;

	/*
	 * Keep a global instance
	 * for dialogs and ressources
	 *
	 */

	hInstance = hInst;

	showing_cursor = 1;
	firstStart     = 1;

	/*
	 * Load the options
	 *
	 */

	LoadOptions();

	/*
     * Init the video subsystem 
     *
     */

	playlist = new Playlist();
	playback = new MediaPlayback();
	skinList = new SkinList();
	resizer  = new Resizer();

	/*
	 * Parses the command line
	 * and ovveride options
	 *	
     */

	ParseCmdLine(lpszCmdParam);

	/*
	 * Set the default settings
	 *
	 */

	playback->SetLoop(options.loop);
	playback->videoDecoder->decoreDecoder->SetQuality(options.postprocessing);
	playback->SetDesktopMode(FALSE);

	/*
	 * Window size adjustmen
	 *
	 */

	windowRect.left   = 0;
	windowRect.right  = DEFAULT_SKIN_WIDTH;
	windowRect.top    = 0;
	windowRect.bottom = DEFAULT_SKIN_HEIGHT;

    AdjustWindowRect(&windowRect, WS_POPUP|WS_SIZEBOX, 0);

	/*
	 * Initialize the COM library
	 *
	 */

	CoInitialize(NULL);	

	/*
	 * Register the Window Class
	 * 
	 */

	memset(&W, 0, sizeof(WNDCLASS));
   
	W.style         = CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	W.lpfnWndProc   = WndProc;
	W.hInstance     = hInst;
	W.hbrBackground = (HBRUSH)(0);
	W.hCursor       = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR1));
	W.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDB_ICON));
	W.lpszClassName = Name;
	W.lpszMenuName  = NULL;

	RegisterClass(&W);

   /*
    * Load the menu and change 
	* it for recent file list
	*
    */

	popupMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));

	ReBuildRecentFilesMenu();
	ReBuildPlaylistMenu();

   /*
    * Create the main window
    *
    */

	hwnd = CreateWindow(Name, Name, WS_POPUP | WS_SIZEBOX, 
		                options.posX, options.posY, 
						windowRect.right - windowRect.left, 
						windowRect.bottom - windowRect.top, 
						NULL, NULL, hInst, NULL);
   /*
    * Set the window Rgn
    *
    */

	GetClientRect(hwnd, &clientRect);
	GetWindowRect(hwnd, &windowRect);

	pt.x = clientRect.left;
	pt.y = clientRect.top;

	ClientToScreen(hwnd, &pt);

	pt2.x = clientRect.right;
	pt2.y = clientRect.bottom;

	ClientToScreen(hwnd, &pt2);

	SetWindowRgn(hwnd, CreateRectRgn( pt.x  - windowRect.left, 
		  							  pt.y  - windowRect.top,
									  (windowRect.right - windowRect.left) - (windowRect.right - pt2.x),
									  (windowRect.bottom - windowRect.top) - (windowRect.bottom - pt2.y)), TRUE); 

	/*
     * Register for Drag n' Drop
     */

	DragAcceptFiles(hwnd, TRUE);

   /*
    * Start TIMER
    *
    */

	SetTimer(hwnd, TIMER_ID, TIMER_RATE, NULL);

	/*
	 * Load Accelerators
	 */

	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));	

	/*
	 * Set the icon
	 */

	SetClassLong(hwnd, GCL_HICON, (LONG) LoadIcon(hInst, MAKEINTRESOURCE(IDB_ICON))); 
 
	/*
	 * Menu items 
	 */

	CheckMenuItem(popupMenu, (UINT)ID_LOOP, options.loop ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(popupMenu, (UINT)ID_ON_TOP, options.on_top ? MF_CHECKED : MF_UNCHECKED);

	switch(options.aspect_ratio) {

		case ASPECT_RATIO_ORIGINAL:

			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_CHECKED);
			break;
			
		case ASPECT_RATIO_TV:

			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_CHECKED);
			break;

		case ASPECT_RATIO_WIDE:

			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_CHECKED);
			break;

		case ASPECT_RATIO_CUSTOM:

			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
			CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_CHECKED);
			break;
	}

   /*
    * Loads the skin
    *
    */

   skin = new Skin(hInst, hwnd);

   if(strcmp(skinPath, "Default") == 0) {

	   skin->LoadDefault(hInst, hwnd);
   }
   else {
	
	   skin->Load(skinPath, hwnd);
   }

   /*
    * Disable screen saver
    * And Get the current State
    *
    */
		
	screenSaverActive = FALSE;
	
	SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &screenSaverActive, 0);

	if(options.disable_screen_saver) {

		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);
   }

   /*
    * Let the Show Begin
	*
    */

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	GetWindowRect(hwnd, &windowRect);

	if(options.on_top)
		SetWindowPos(hwnd, (HWND) -1,  windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, TRUE);

	/*
	 * Message Loop
	 *
	 */

	while (TRUE)
    {

	   /*
		* Get the messages
		*/

	   if (!GetMessage(&Msg, NULL, 0, 0))
		return (int) Msg.wParam;

		if (!TranslateAccelerator(hwnd, hAccel, &Msg)) {
				
			TranslateMessage(&Msg); 
			DispatchMessage(&Msg); 
		} 
		else {
	           
			TranslateMessage(&Msg);
		    DispatchMessage(&Msg);
		}
   }

   return Msg.wParam;
}

/**************************************************************************************
 *                                                                                    *
 * - OpenFileForPlaying():                                                            *
 *                                                                                    *
 *   -- Opens the current playlist location                                           *
 *                                                                                    *
 **************************************************************************************/

void OpenFileForPlaying(HWND hwnd) {

	char *filename;
	RECT rect, windowrect;

	/*
	 * Closes last playback
	 *
	 */

	playback->Close();
	filename = playlist->GetCurrentItem()->filename;

	/*
	 * If no file to open
	 * we simply returns... 
	 * 
	 */

	if(filename == NULL) {

		return;
	}

	openning_network = FALSE;

	/*
	 * We need to make that cleaner
	 *
	 */
	
	if(strstr(filename, "http://") != NULL ||
	   strstr(filename, "HTTP://") != NULL || 
	   strstr(filename, "FTP://")  != NULL ||
	   strstr(filename, "ftp://")  != NULL) {

		if(playback->OpenMediaSource(filename) == MP_RESULT_OK) {

			openning_network = TRUE;

			return;
		}

		MP_ERROR("The network location you selected could not be opened");

		return;
	}
	else {

	if(playback->OpenMedia(filename, hwnd) == MP_RESULT_OK) {

		/*
		 * First resize the window
		 */

		DWORD i, width, height;

		switch(options.aspect_ratio) {

		case ASPECT_RATIO_FREE:
		case ASPECT_RATIO_ORIGINAL:

			width  = playback->GetVideoWidth();
			height = playback->GetVideoHeight();
			break;

		case ASPECT_RATIO_TV:
		case ASPECT_RATIO_WIDE:
		case ASPECT_RATIO_CUSTOM:

			width  = playback->GetVideoWidth();
			height = width*aspectRatios[options.aspect_ratio].yFactor/aspectRatios[options.aspect_ratio].xFactor;
			break;
		}

		if(!playback->fullscreen) {
			GetWindowRect(hwnd, &windowrect); 
		}
		
		if(compact_mode) {

			rect.left   = 0;
			rect.top    = 0;
			rect.right  = width;
			rect.bottom = height;
		}
		else {

			rect.left   = 0;
			rect.top    = 0;
			rect.right  = width + 15;
			rect.bottom = height + 115 + 22;
		}

		AdjustWindowRect(&rect, WS_POPUP|WS_SIZEBOX, FALSE);

		fullwindowRect.right  = fullwindowRect.left + rect.right - rect.left;
		fullwindowRect.bottom = fullwindowRect.top  + rect.bottom - rect.top;

		if(!playback->fullscreen) {
	
			MoveWindow( hwnd, windowrect.left, windowrect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
			playback->SetVideoRect(skin->GetVideoRect());
		}

		/*
		 * Update recent file list
		 */
		
		UpdateRecentFilesMenu(filename);

		/*
		 * And update the menu
		 */

		if(!playback->IsInFullscreen()) {

			ChangeMenuForNormalMode();
			
			/*
			 * And the recent file list
			 */
		
			ReBuildRecentFilesMenu();
		}

		/*
		 * Set the volume
		 */

		playback->SetVolume(skin->GetVolume());
		
		/*
		 * Save/Set postprocessing
		 */

		options.postprocessing = playback->videoDecoder->GetQuality();
		playback->videoDecoder->SetQuality(options.postprocessing);

		/*
	 	 * Then play
		 */

		playback->Play();

		/*
		 * And update the window
		 */

		UpdateMainWindow();
	}
	else {

		/*
		 * File couldn't be opened
		 */

		MP_ERROR("The location you selected could not be opened");
	}
  }
}

/**************************************************************************************
 *                                                                                    *
 * - AddFilesToPlaylist():                                                            *
 *                                                                                    *
 *   -- Adds a file To the playlist                                                   *
 *                                                                                    *
 **************************************************************************************/

void AddFilesToPlaylist(HWND hwnd)
{
	OPENFILENAME ofn;
	char szFile[260];
				
	/*
	 * shows a file selector
	 */

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ZeroMemory(szFile, 260);

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = "All\0*.*\0AVI Files\0*.AVI\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = "";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
				
	if (GetOpenFileName(&ofn) == TRUE) {
		
		if(ofn.lpstrFile[strlen(ofn.lpstrFile) + 1] == NULL) {

			/*
			 * Only one file selected
			 *
			 */

			playlist->AddItem(ofn.lpstrFile);
		}
		else {

			/*
			 * Multiple selection
			 *
			 */

			DWORD index, i = 0;
			char  dir[1024];

			index = strlen(ofn.lpstrFile) + 1;

			do {
						
				strcpy(dir, ofn.lpstrFile);

				playlist->AddItem((LPSTR)strcat(strcat(dir, "\\"), ofn.lpstrFile + index));
				index += strlen(dir) - strlen(ofn.lpstrFile);
			}
			while(ofn.lpstrFile[index] != NULL);
		}

		if(!playback->HasVideo()) {

			OpenFileForPlaying(hwnd);
		}

		ReBuildPlaylistMenu();
	}
}


/**************************************************************************************
 *                                                                                    *
 * - FilesOpen():                                                                     *
 *                                                                                    *
 *   -- Open file(s) for payback                                                      *
 *                                                                                    *
 **************************************************************************************/

void FilesOpen(HWND hwnd)
{
	OPENFILENAME ofn;
	char szFile[260];
				
	/*
	 * shows a file selector
	 */

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ZeroMemory(szFile, 260);

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = "All\0*.*\0AVI Files\0*.AVI\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = "";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
				
	if (GetOpenFileName(&ofn) == TRUE) {
		
		playlist->Reset();
	
		if(ofn.lpstrFile[strlen(ofn.lpstrFile) + 1] == NULL) {

			/*
			 * Only one file selected
			 *
			 */

			playlist->AddItem(ofn.lpstrFile);
		}
		else {

			/*
			 * Multiple selection
			 *
			 */

			DWORD index, i = 0;
			char  dir[1024];

			index = strlen(ofn.lpstrFile) + 1;

			do {
						
				strcpy(dir, ofn.lpstrFile);

				playlist->AddItem((LPSTR)strcat(strcat(dir, "\\"), ofn.lpstrFile + index));
				index += strlen(dir) - strlen(ofn.lpstrFile);
			}
			while(ofn.lpstrFile[index] != NULL);
		}

		OpenFileForPlaying(hwnd);
		ReBuildPlaylistMenu();
	}
}


/**************************************************************************************
 *                                                                                    *
 *                            WNDPROC (Main Event Func)                               *
 *                            -------------------------                               *
 **************************************************************************************
 *                                                                                    *
 *                                                                                    *
 *                                                                                    *
 **************************************************************************************/

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	signed short xPos, yPos;
	RECT         r;

	switch(Message) {

		/*
		 * Message sent by the playback engine
		 * when the playbacks en (go to next file)
		 *
		 */

	case WM_PLAYA_PLAYBACK_END:

		if(playlist->GetItemCount() > 0) {

			if(playlist->GetCurrentPosition() < playlist->GetItemCount() - 1) {

				playlist->NextItem();
				OpenFileForPlaying(hwnd);
				ReBuildPlaylistMenu();
			}
			else {

				playlist->SetCurrentPosition(0);
			}
		}
		break;

	case WM_DESTROY:

		Quit();
		return TRUE;
		break;

	case WM_SYSCOMMAND:
        
		if (wParam == SC_CLOSE)
          {
			Quit();
			return (TRUE);
          }
        break;

	case WM_COMMAND	:

		RECT rect;
    
		switch(LOWORD(wParam)) {

		case ID_RECENT_FILE1:

			playlist->Reset();
			playlist->AddItem(RecentFiles[0]);

			OpenFileForPlaying(hwnd);
			ReBuildPlaylistMenu();
			break;

		case ID_RECENT_FILE2:

			playlist->Reset();
			playlist->AddItem(RecentFiles[1]);

			OpenFileForPlaying(hwnd);
			ReBuildPlaylistMenu();
			break;

		case ID_RECENT_FILE3:

			playlist->Reset();
			playlist->AddItem(RecentFiles[2]);

			OpenFileForPlaying(hwnd);
			ReBuildPlaylistMenu();
			break;

		case ID_RECENT_FILE4:

			playlist->Reset();
			playlist->AddItem(RecentFiles[3]);

			OpenFileForPlaying(hwnd);
			ReBuildPlaylistMenu();
			break;

		case ID_RECENT_FILE5:

			playlist->Reset();
			playlist->AddItem(RecentFiles[4]);

			OpenFileForPlaying(hwnd);
			ReBuildPlaylistMenu();
			break;

		case ID_MENU_ABOUT:

            DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
                       hwnd, (DLGPROC)AboutDlgProc);
						
			break;

		case ID_CLEAR_LIST:

			DWORD i;

			for(i=0; i<5; i++) {

				if(RecentFiles[i] != NULL) {

					free(RecentFiles[i]);
					RecentFiles[i] = NULL;
				}
			}

			ReBuildRecentFilesMenu();
			break;

		case ID_SKIN_BROWSER:

            DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_SKIN_BROWSER),
                       hwnd, (DLGPROC)SkinBrowserDlgProc);
						
			break;

		case ID_PLAYBACK_PROPERTIES:

            if(playback->HasVideo())
				DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_PLAYBACK_PROPERTIES),
					       hwnd, (DLGPROC)PlaybackPropertiesDlgProc);
						
			break;

		case ID_MENU_OPTIONS:

	            DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_PREFS),
		                   hwnd, (DLGPROC)PreferencesDlgProc);
			break;

		case ID_OPEN_URL:

            DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_URL_DIALOG),
                       hwnd, (DLGPROC)UrlDlgProc);

			if(openOK) {

				openOK = FALSE;

				playlist->Reset();
				playlist->AddItem(url);

				OpenFileForPlaying(hwnd);
			}
			break;

		case ID_PROPERTIES:

			if(playback->HasVideo()) {
				DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_PROPERTIES),
		                   hwnd, (DLGPROC)PropDlgProc);
			}
			break;

		case ID_OPEN_FILE:
			
			FilesOpen(hwnd);
			break;

		case ID_MENU_PLAYBACK_FILES_ADDFILES:

			AddFilesToPlaylist(hwnd);
			break;

		case ID_ON_TOP:

			if(!playback->IsInFullscreen()) {

				options.on_top = options.on_top == 1 ? 0 : 1;				
				CheckMenuItem(popupMenu, (UINT)wParam, options.on_top ? MF_CHECKED : MF_UNCHECKED);
			
				GetWindowRect(hwnd, &r);
			
				SetWindowPos(hwnd, options.on_top ? (HWND) -1 : (HWND) 1,  r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

				if(!options.on_top)
					SetWindowPos(hwnd, HWND_TOP,  r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
			}	
			break;

		case ID_LOOP:

			options.loop = options.loop == 1 ? 0 : 1;				
			CheckMenuItem(popupMenu, (UINT)wParam, options.loop ? MF_CHECKED : MF_UNCHECKED);
			
			playback->SetLoop(options.loop);
			
			break;

		case ID_SCREENSAVER:

			if(options.disable_screen_saver) {
			
				options.disable_screen_saver = 0;
				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, screenSaverActive, NULL, 0);

				CheckMenuItem(popupMenu, ID_SCREENSAVER, MF_UNCHECKED);
			}
			else {

				options.disable_screen_saver = 1;
				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, 0);

				CheckMenuItem(popupMenu, ID_SCREENSAVER, MF_CHECKED);
			}
			break;

		case ID_DESKTOP:

			if(playback->HasVideo()) {

			if(!playback->IsInFullscreen() && !playback->desktopMode) {

				if(playback->IsOverlay()) {
				
				if(compact_mode) {

					UnPrepareCompactMode();
				}

				ChangeMenuForFullscreenMode();

				/*
				 * Go ahead
				 */

				PrepareDesktopMode();

				playback->SetDesktopMode(TRUE);
				playback->SetFullscreen(TRUE, GetDesktopWindow());

				MoveWindow(hwnd, fullwindowRect.left, fullwindowRect.top, DEFAULT_SKIN_WIDTH, 22+115, TRUE);

				}
				else {

					MP_ERROR("The Desktop Mode requires support for Overlays. Your current video mode does not allow Overlay creation.");
				}
			}
			else {

				RECT rect;

				playback->SetFullscreen(FALSE, hwnd);
				ShowCursor(1);

				/*
				 * We need to resize the window to it's original size
				 */

				MoveWindow(hwnd, fullwindowRect.left, fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
						   fullwindowRect.bottom - fullwindowRect.top, TRUE);

				playback->SetVideoRect(skin->GetVideoRect());

				if(playback->desktopMode) {

					UnPrepareDesktopMode();
				}

				ChangeMenuForNormalMode();

				/*
				 * This is ugly
				 */

				Sleep(100);
				UpdateMainWindow();
			}
			}
			else {
			}
			break;

		case ID_COMPACT:

			if(!playback->IsInFullscreen()) {

				if(compact_mode == 0) {
   
					compact_mode = 1;

					skin->SetCompact(TRUE);
					playback->SetVideoRect(skin->GetVideoRect());

					/*
					 * Change the menu item text
					 */

					PrepareCompactMode();

					/*
					 * And resize the window
					 */
	
					if(playback->IsPlaying()) {
	
						RECT src;
	
						GetWindowRect(hwnd, &cwindowRect);
	
						src.left    = 0;
						src.right   = cwindowRect.right - cwindowRect.left - 15;
						src.top     = 0;
						src.bottom  = cwindowRect.bottom - cwindowRect.top - 115 - 22;
	
						AdjustWindowRect(&src, WS_POPUP | WS_SIZEBOX, 0);
	
						MoveWindow( hwnd, cwindowRect.left + 7, 
									cwindowRect.top + 22, 
									src.right - src.left, 
									src.bottom - src.top, TRUE);

					}
					else {
	
						RECT src;

						GetWindowRect(hwnd, &cwindowRect);

						src.left    = 0;
						src.right   = 280;
						src.top     = 0;
						src.bottom  = 235;	

						AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

						MoveWindow( hwnd, cwindowRect.left + 7, 
									cwindowRect.top + 22, 
									src.right - src.left, 
									src.bottom - src.top, TRUE);
					}

				}
				else {
   
					compact_mode = 0;
					skin->SetCompact(FALSE);

					/*
					 * Change the menu item text
					 */	

					UnPrepareCompactMode();

					/*
					 * And resize the window
					 */

					GetWindowRect(hwnd, &windowRect);
	
					MoveWindow( hwnd, windowRect.left - 7, 
								windowRect.top - 22, 
								cwindowRect.right - cwindowRect.left, 
								cwindowRect.bottom - cwindowRect.top, TRUE);
				
					playback->SetVideoRect(skin->GetVideoRect());
				}
			}
			break;

		case ID_PREVIOUS:

			playback->Rewind();
			break;

		case ID_STOP:

			if(playback->IsInFullscreen()) {
				
					playback->SetFullscreen(FALSE, hwnd);
					ShowCursor(1);

					/*
					 * We need to resize the window to it's original size
					 */

					MoveWindow(hwnd, fullwindowRect.left, fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
							   fullwindowRect.bottom - fullwindowRect.top, TRUE);

					ChangeMenuForNormalMode();
						
					/*
					 * Re-update the Desktop
					 */

					if(playback->desktopMode)
						playback->SetDesktopMode(FALSE);
			}

			playback->Stop(TRUE);
			break;
		
		case ID_PLAY:

			if(playback->HasVideo() || playback->HasAudio()) {
				
				if(playback->IsPaused()) {
					playback->Pause();
				}
				else {
					playback->Play();

					if(playback->IsOverlay()) {

						skin->Display(hwnd, playback);
					}
				}
			}
			else {

				/*
				 * Load files
				 */
			
				FilesOpen(hwnd);
			}

			break;
		
		case ID_PAUSE:

			if(playback->HasVideo() || playback->HasAudio()) {


				if(playback->rewind || playback->fastForward) {

					playback->Play();
				}
				else {

					if(playback->IsPaused() || playback->IsPlaying()) {
						playback->Pause();
						playback->MaintainImage();
					}
					else {
						playback->Play();	

					}

					if(playback->IsOverlay()) {
	
						skin->Display(hwnd, playback);
					}
				}
			}
			else {

			/*
			 * Load files
			 */
			
				FilesOpen(hwnd);
			}


			break;

		case ID_NEXT:

			playback->FastForward();
			break;

		case ID_NEXT_FILE:

			if(playlist->GetItemCount() > 0) {
				
				playlist->NextItem();
				OpenFileForPlaying(hwnd);
				ReBuildPlaylistMenu();
			}
			break;

		case ID_PREVIOUS_FILE:

			if(playlist->GetItemCount() > 0) {
			
				playlist->PreviousItem();
				OpenFileForPlaying(hwnd);
				ReBuildPlaylistMenu();
			}
			break;

		case ID_PLAYLIST1:
		case ID_PLAYLIST1+1:
		case ID_PLAYLIST1+2:
		case ID_PLAYLIST1+3:
		case ID_PLAYLIST1+4:
		case ID_PLAYLIST1+5:
		case ID_PLAYLIST1+6:
		case ID_PLAYLIST1+7:
		case ID_PLAYLIST1+8:
		case ID_PLAYLIST1+9:
		case ID_PLAYLIST1+10:
			
			if(playlist->GetItemCount() > 0) {
			
				playlist->SetCurrentPosition(LOWORD(wParam)-ID_PLAYLIST1);
				OpenFileForPlaying(hwnd);
				ReBuildPlaylistMenu();
			}
			break;
			
		case ID_EXIT:

			Quit();
			break;

		case ID_FULLSCREEN:

			if(playback->HasVideo()) {
			
				if(!playback->IsInFullscreen()) {

					ChangeMenuForFullscreenMode();

					GetWindowRect(hwnd, &fullwindowRect);

					/*
					 * Now make the window fill 
					 * the whole desktop, and on top  
					 */
				
					count          = 0;
					showing_cursor = 1;
	
					playback->SetFullscreen(TRUE, hwnd);

					SetWindowPos(hwnd, (HWND) -1,  fullwindowRect.left, 
							     fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
								 fullwindowRect.bottom - fullwindowRect.top, FALSE);
				
					MoveWindow(hwnd, -5, -5, GetSystemMetrics(SM_CXFULLSCREEN) + 20, GetSystemMetrics(SM_CYFULLSCREEN) + 100, TRUE);
				
					playback->SetVideoRect(skin->GetVideoRect());
				}
				else {

					RECT rect;

					playback->SetFullscreen(FALSE, hwnd);
					ShowCursor(1);

					/*
					 * We need to resize the window to it's original size
					 */

					MoveWindow(hwnd, fullwindowRect.left, fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
							   fullwindowRect.bottom - fullwindowRect.top, TRUE);


					ChangeMenuForNormalMode();
					
					/*
					 * Re-update the Desktop
					 */

					if(playback->desktopMode)
						playback->SetDesktopMode(FALSE);

					playback->SetVideoRect(skin->GetVideoRect());

					UpdateMainWindow();
				}
			}
			break;

		case ID_ORIGINAL_SIZE:

			if(!playback->IsInFullscreen()) {
			if(playback->IsPlaying()) {

				RECT src;

				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.left    = 0;
					src.right   = playback->GetVideoWidth();
					src.top     = 0;
					src.bottom  = playback->GetVideoHeight();
				}
				else {

					src.left    = 0;
					src.right   = playback->GetVideoWidth() + 15;
					src.top     = 0;
					src.bottom  = playback->GetVideoHeight() + 115 + 22;
				}

				AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);
	
				playback->SetVideoRect(skin->GetVideoRect());
			}
			}
			break;

			case ID_HALF_SIZE:

			if(!playback->IsInFullscreen()) {
				if(playback->IsPlaying()) {

				RECT src;

				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.left    = 0;
					src.right   = playback->GetVideoWidth()/2;
					src.top     = 0;
					src.bottom  = playback->GetVideoHeight()/2;
				}
				else {

					src.left    = 0;
					src.right   = playback->GetVideoWidth()/2 + 15;
					src.top     = 0;
					src.bottom  = playback->GetVideoHeight()/2 + 115 + 22;
				}

				AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);

				playback->SetVideoRect(skin->GetVideoRect());
				}
			}
			break;

			case ID_DOUBLE_SIZE:

			if(!playback->IsInFullscreen()) {
			  if(playback->IsPlaying()) {

				RECT src;

				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.left    = 0;
					src.right   = 2*playback->GetVideoWidth();
					src.top     = 0;
					src.bottom  = 2*playback->GetVideoHeight();
				}
				else {
					
					src.left    = 0;
					src.right   = playback->GetVideoWidth()*2 + 15;
					src.top     = 0;
					src.bottom  = playback->GetVideoHeight()*2 + 115 + 22;
				}

				AdjustWindowRect(&src, WS_OVERLAPPEDWINDOW, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);

				playback->SetVideoRect(skin->GetVideoRect());
			  }
			}
			break;

			case ID_ASPECT_FREE:

				/*
				 * Simply unset the aspect 
				 */

				if(!playback->IsInFullscreen()) {
			
				switch(options.aspect_ratio) {

				case ASPECT_RATIO_ORIGINAL:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_TV:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_WIDE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_CUSTOM:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_UNCHECKED);
					break;
				}

				options.aspect_ratio = ASPECT_RATIO_FREE;
				CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_CHECKED);
				
				}
				break;

			case ID_ASPECT_ORIGINAL:
				{
				
				if(!playback->IsInFullscreen()) {

					switch(options.aspect_ratio) {

				case ASPECT_RATIO_FREE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_TV:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_WIDE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_CUSTOM:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_UNCHECKED);
					break;
				}

				options.aspect_ratio = ASPECT_RATIO_ORIGINAL;
				CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_CHECKED);

				/*
				 * Now resize the window
				 */	

				RECT  src;

				GetClientRect(hwnd, &src);
				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.bottom = (src.right - src.left)*playback->GetVideoHeight()/playback->GetVideoWidth();
				}
				else {

					src.bottom = (src.right - src.left - 15)*playback->GetVideoHeight()/playback->GetVideoWidth() + 115 + 22;
				}

				AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);
				
				playback->SetVideoRect(skin->GetVideoRect());
				}
				}
				break;
			
			case ID_ASPECT_43:
				{
				
				/*
				 * Now resize the window
				 */	
			if(!playback->IsInFullscreen()) {

				switch(options.aspect_ratio) {

				case ASPECT_RATIO_ORIGINAL:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_FREE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_WIDE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_CUSTOM:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_UNCHECKED);
					break;
				}


				options.aspect_ratio = ASPECT_RATIO_TV;
				CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_CHECKED);

				RECT  src;

				GetClientRect(hwnd, &src);
				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.bottom = (src.right - src.left)*3/4;
				}
				else {
					
					src.bottom = (src.right - src.left - 15)*3/4 + 115 + 22;
				}

				AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);
				
				playback->SetVideoRect(skin->GetVideoRect());
				}
				}
				break;
			
			case ID_ASPECT_169:
				{
				
				/*
				 * Now resize the window
				 */	

			if(!playback->IsInFullscreen()) {

				switch(options.aspect_ratio) {

				case ASPECT_RATIO_ORIGINAL:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_TV:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_43, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_FREE:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_FREE, MF_UNCHECKED);
					break;

				case ASPECT_RATIO_CUSTOM:

					CheckMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM, MF_UNCHECKED);
					break;
				}

				options.aspect_ratio = ASPECT_RATIO_WIDE;
				CheckMenuItem(popupMenu, (UINT)ID_ASPECT_169, MF_CHECKED);
				
				RECT  src;

				GetClientRect(hwnd, &src);
				GetWindowRect(hwnd, &windowRect);

				if(compact_mode) {

					src.bottom = (src.right - src.left)*9/16;
				}
				else {

					src.bottom = (src.right - src.left - 15)*9/16 + 115 + 22;
				}

				AdjustWindowRect(&src, WS_POPUP|WS_SIZEBOX, 0);

				MoveWindow( hwnd, windowRect.left, 
							windowRect.top, 
							src.right - src.left, 
							src.bottom - src.top, TRUE);
				
				playback->SetVideoRect(skin->GetVideoRect());
				}
				}
				break;

			case ID_ASPECT_CUSTOM:
			{

				if(!playback->IsInFullscreen()) {
				
					DialogBox (hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_ASPECT_RATIO_DIALOG),
							   hwnd, (DLGPROC)CustomAspectDlgProc);
				}
			}
		}
	break;

	/*
	 * Timer
	 *
	 */

	case WM_TIMER:

		if(firstStart) {
	
			firstStart = 0;

			if(playlist->GetItemCount() > 0) {

				OpenFileForPlaying(hwnd);

				if(options.startFullscreen) {

						ChangeMenuForFullscreenMode();
						GetWindowRect(hwnd, &fullwindowRect);

						/*
						 * Now make the window fill 
						 * the whole desktop, and on top  
						 */
				
						count          = 0;
						showing_cursor = 1;
	
						playback->SetFullscreen(TRUE, hwnd);

						SetWindowPos(hwnd, (HWND) -1,  fullwindowRect.left, 
								     fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
									 fullwindowRect.bottom - fullwindowRect.top, FALSE);
					
						MoveWindow(hwnd, -5, -5, GetSystemMetrics(SM_CXFULLSCREEN) + 20, GetSystemMetrics(SM_CYFULLSCREEN) + 100, TRUE);
				
						playback->SetVideoRect(skin->GetVideoRect());
				}
			}
		}

		if(playback && playback->IsPlaying() && !(playback->IsInFullscreen() && !playback->desktopMode)) {

			if(action != ACTION_PROGRESS_CURSOR && !compact_mode) {
				skin->SetProgressValue(hwnd, playback->GetPlaybackProgress());
			}

			/*
			 * Do time display here
			 */

			if(!compact_mode)
				skin->DrawTime(hwnd, playback);
		}

		if(!playback->IsPlaying() && !playback->IsPaused() && !playback->IsInFullscreen()) {
	
			if(action != ACTION_PROGRESS_CURSOR && !compact_mode) {
				skin->SetProgressValue(hwnd, playback->GetPlaybackProgress());
			}
		}

		if(playback->IsBuffering())  {

			if(openning_network)
				playback->UpdateBuffering();

			skin->DrawBufferingState(hwnd, playback->bufferingProgress);
		
			if((playback->input->GetBufferSize() >= playback->input->GetBufferingSize()) && openning_network) {

				openning_network = FALSE;
				
				if(playback->OpenMediaFromSource(hwnd) == MP_RESULT_OK) {

					RECT windowrect;

					/*
					 * First resize the window
					 */

					DWORD i, width, height;

					switch(options.aspect_ratio) {

						case ASPECT_RATIO_FREE:
						case ASPECT_RATIO_ORIGINAL:

							width  = playback->GetVideoWidth();
							height = playback->GetVideoHeight();
							break;

						case ASPECT_RATIO_TV:

							width  = playback->GetVideoWidth();
							height = width*3/4;
							break;

						case ASPECT_RATIO_WIDE:

						width  = playback->GetVideoWidth();
						height = width*9/16;
						break;
					}
	
					if(!playback->fullscreen) {
						GetWindowRect(hwnd, &windowrect); 
					}
					
					if(compact_mode) {

						rect.left   = 0;
						rect.top    = 0;
						rect.right  = width;
						rect.bottom = height;
					}
					else {

						rect.left   = 0;
						rect.top    = 0;
						rect.right  = width + 15;
						rect.bottom = height + 115 + 22;
					}
			
					AdjustWindowRect(&rect, WS_POPUP|WS_SIZEBOX, FALSE);		

					fullwindowRect.right  = fullwindowRect.left + rect.right - rect.left;
					fullwindowRect.bottom = fullwindowRect.top  + rect.bottom - rect.top;

					if(!playback->fullscreen) {

						MoveWindow( hwnd, windowrect.left, windowrect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
						playback->SetVideoRect(skin->GetVideoRect());
					}

					/*
					 * Update recent file list
					 */

					UpdateRecentFilesMenu(playback->filename);

					/*
					 * And update the menu
					 */

					if(!playback->fullscreen) {


						ReBuildRecentFilesMenu();

						HMENU menu;

						menu = GetSubMenu(popupMenu, 0);

						/*
						 * The subtitles and Properties menus
						 */

						MENUITEMINFO itemInfo;
				
						/*
						 * Re-update the menu items
						 */

						EnableMenuItem(popupMenu, ID_COMPACT, MF_ENABLED);
						EnableMenuItem(popupMenu, ID_FULLSCREEN, MF_ENABLED);

						EnableMenuItem(popupMenu, (UINT)ID_ON_TOP,      MF_ENABLED);

						EnableMenuItem(popupMenu, (UINT)ID_ASPECT_ORIGINAL, MF_ENABLED);
						EnableMenuItem(popupMenu, (UINT)ID_ASPECT_FREE,     MF_ENABLED);
						EnableMenuItem(popupMenu, (UINT)ID_ASPECT_43,       MF_ENABLED);
						EnableMenuItem(popupMenu, (UINT)ID_ASPECT_169,      MF_ENABLED);
						EnableMenuItem(popupMenu, (UINT)ID_ASPECT_CUSTOM,   MF_ENABLED);

						menu = GetSubMenu(popupMenu, 0);

						memset(&itemInfo, 0, sizeof(MENUITEMINFO));
			
						itemInfo.cbSize     = sizeof(MENUITEMINFO);
						itemInfo.fType      = MFT_STRING;
						itemInfo.fMask      = MIIM_TYPE;
						itemInfo.dwTypeData = "&Desktop mode\tAlt+D";
		
						SetMenuItemInfo(menu, ID_DESKTOP, FALSE, &itemInfo);

						EnableMenuItem(popupMenu, (UINT)ID_PROPERTIES, MF_ENABLED);
						EnableMenuItem(popupMenu, (UINT)ID_DESKTOP, MF_ENABLED);
		
					}

					playback->SetVolume(skin->GetVolume());

					/*
				 	 * Then play
					 */

					playback->Play();

					/*
					 * And update the window
					 */

					UpdateMainWindow();
				}
				else {
					playback->Close();
				}
			}
		}

		if(playback->IsInFullscreen() && showing_cursor) {

			if(count <= 21)
				count++;

			/*
			 * We wait for 2 secs
			 */ 

			if(count >= 20) {

				if(!playback->desktopMode) {
					
					ShowCursor(0);
					showing_cursor = 0;
				}
			}
		}


		break;

	/*
	 * Key Press
	 *
	 */

	case WM_KEYUP:

		switch(wParam) {
			
			case VK_ESCAPE:

				if(playback->IsInFullscreen()) {

					RECT rect;

					playback->SetFullscreen(FALSE, hwnd);
					ShowCursor(1);

					/*
					 * We need to resize the window to it's original size
					 */

					MoveWindow(hwnd, fullwindowRect.left, fullwindowRect.top, fullwindowRect.right - fullwindowRect.left, 
							   fullwindowRect.bottom - fullwindowRect.top, TRUE);

					playback->SetVideoRect(skin->GetVideoRect());

					ChangeMenuForNormalMode();

					/*
					 * Re-update the Desktop
					 */
	
					if(playback->desktopMode)
						playback->SetDesktopMode(FALSE);

					/*
					 * This is ugly
					 */

					Sleep(100);
					UpdateMainWindow();
				}
				break;

			default:
				break;
		}
		break;

	case WM_LBUTTONDOWN:

		if(!(playback->IsInFullscreen() && !playback->desktopMode)) {

			xPos = LOWORD(lParam); 
			yPos = HIWORD(lParam);

			if(compact_mode) {

				action = ACTION_NONE;
			}
			else {

				action = skin->GetAction(xPos, yPos);
				skin->SetActionStart(hwnd, action);
			}
	
			if(action == ACTION_RESIZE) {

				if(!playback->desktopMode) {
					
					POINT pt;

					pt.x = xPos;
					pt.y = yPos;

					ClientToScreen(hwnd, &pt);

					moveX = pt.x;
					moveY = pt.y;
	
					GetWindowRect(hwnd, &windowRect);

					SetCapture(hwnd);
		
					resizer->Start(&pt, playback->GetVideoWidth(), playback->GetVideoHeight());
				}
				else {
					action = ACTION_NONE;
				}
			}

			if(action == ACTION_NONE) {

				/*
				 * We are moving the window
				 */

				action = ACTION_MOVING;

				moveX = xPos;
				moveY = yPos;

				GetWindowRect(hwnd, &windowRect);

				SetCapture(hwnd);
			}

			if(action == ACTION_VOLUME_CURSOR || action == ACTION_PROGRESS_CURSOR) {

				moveX = xPos;
				SetCapture(hwnd);
			}
		}
		break;

	case WM_LBUTTONUP:

		skin->SetActionEnd(hwnd, action);

		switch(action) {

		case ACTION_MENU:

			HMENU menu;

			POINT pt;

			pt.x = pt.y = 0;
			ClientToScreen(hwnd, &pt);

			xPos = pt.x + LOWORD(lParam); 
			yPos = pt.y + HIWORD(lParam);

			menu = GetSubMenu(popupMenu, 0);

			TrackPopupMenu( menu, 0, xPos, yPos, 0, hwnd, NULL);
			break;

		case ACTION_CLOSE:

			Quit();
			break;

		case ACTION_MINIMIZE:

			ShowWindow(hwnd, SW_MINIMIZE);
			break;

		case ACTION_STOP:

			playback->Stop(TRUE);
			break;

		case ACTION_PLAY:

			if(playback->HasVideo() || playback->HasAudio()) {
				
				if(playback->IsPaused()) {
					playback->Pause();
				}
				else {
					playback->Play();

					if(playback->IsOverlay()) {

						RECT rect;

						GetClientRect(hwnd, &rect);
						InvalidateRect(hwnd, &rect, TRUE);
 						UpdateWindow(hwnd);
					}
				}
			}
			else {

				/*
				 * Load files
				 */
			
				FilesOpen(hwnd);
			}
			break;

		case ACTION_PAUSE:

			if(playback->HasVideo() || playback->HasAudio()) {

				if(playback->IsPaused()) {
				
					playback->NextFrame();
				}
				else {
					playback->Pause();
				}
			}
			break;

		case ACTION_FORWARD:

			playback->FastForward();
			break;

		case ACTION_REWIND:

			playback->Rewind();
			break;

		case ACTION_LOAD:
			{
				FilesOpen(hwnd);
			}
			break;

		case ACTION_PROGRESS_CURSOR:

			ReleaseCapture();
			playback->Seek(skin->GetProgress());
			break;

		case ACTION_VOLUME_CURSOR:

			ReleaseCapture();
			playback->SetVolume(skin->GetVolume());
			break;

		case ACTION_VOLUME_BAR:

			moveX = LOWORD(lParam); 
			skin->SetVolumeCursorX(hwnd, moveX);

			playback->SetVolume(skin->GetVolume());
		
			break;

		case ACTION_PROGRESS_BAR:

			moveX = LOWORD(lParam); 
			skin->SetProgressCursorX(hwnd, moveX);
			playback->Seek(skin->GetProgress());
			break;

		case ACTION_NONE:
		default:
			break;
		}

		if(action == ACTION_MOVING) {

			ReleaseCapture();
			RECT rect;

			GetWindowRect(hwnd, &rect);

			SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, 
						 rect.right - rect.left, rect.bottom - rect.top, 
						 SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);

			GetWindowRect(hwnd, &windowRect);

			if(playback->IsPlaying()) {
			
				GetClientRect(hwnd, &rect);
				InvalidateRect(hwnd, &rect, TRUE);
 				UpdateWindow(hwnd);
			}
		}

		if(action == ACTION_RESIZE) {

			POINT *pt;

			pt = resizer->GetLastPoint();

			ReleaseCapture();
			resizer->Stop();

			MoveWindow( hwnd, windowRect.left, 
						windowRect.top, 
						windowRect.right + (pt->x - moveX) - windowRect.left, 
						windowRect.bottom + (pt->y - moveY) - windowRect.top, TRUE);

			GetWindowRect(hwnd, &windowRect);

			moveX = pt->x;
			moveY = pt->y;
		
			playback->SetVideoRect(skin->GetVideoRect());
		}

		action = ACTION_NONE;

		break;

	case WM_MOUSEMOVE:

		count = 0;
		ShowCursor(1);
		showing_cursor = 1;

		if(action == ACTION_MOVING) {

			RECT   r;
			POINT pt;

			xPos = LOWORD(lParam); 
			yPos = HIWORD(lParam);

			GetClientRect(hwnd, &r);

			pt.x = xPos;
			pt.y = yPos;

			ClientToScreen(hwnd, &pt);

			MoveWindow( hwnd, pt.x - moveX, 
						pt.y - moveY, 
						windowRect.right - windowRect.left, 
						windowRect.bottom - windowRect.top, TRUE);
		
			if(playback->IsPaused()) {

				playback->MaintainImage();
			}
		}

		if(action == ACTION_RESIZE) {

			POINT pt;

			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
	
			if(playback->HasVideo()) {
				resizer->Draw(hwnd, &pt, options.aspect_ratio);
			}
			else {
		
				resizer->Draw(hwnd, &pt, ASPECT_RATIO_FREE);
			}
		}

		if(action == ACTION_VOLUME_CURSOR) {

			moveX = LOWORD(lParam); 
			skin->SetVolumeCursorX(hwnd, moveX);

			playback->SetVolume(skin->GetVolume());
		}

		if(action == ACTION_PROGRESS_CURSOR) {

			moveX = LOWORD(lParam); 
			skin->SetProgressCursorX(hwnd, moveX);
		}

		break;

	case WM_RBUTTONDOWN:

		HMENU menu;
		POINT pt1;
		int   x, y;

		pt1.x = pt1.y = 0;
		ClientToScreen(hwnd, &pt1);

		x = pt1.x + LOWORD(lParam); 
		y = pt1.y + HIWORD(lParam);

		menu = GetSubMenu(popupMenu, 0);

		TrackPopupMenu( menu, 0, x, y, 0, hwnd, NULL);
		break;

	case WM_SIZE:

		POINT pt2;

		RECT  clientRect, windowRect;
		POINT other_pt;

		GetClientRect(hwnd, &clientRect);
		GetWindowRect(hwnd, &windowRect);

		other_pt.x = clientRect.left;
		other_pt.y = clientRect.top;

		ClientToScreen(hwnd, &other_pt);

		pt2.x = clientRect.right;
		pt2.y = clientRect.bottom;

		ClientToScreen(hwnd, &pt2);

		if(!compact_mode) {

		    SetWindowRgn(hwnd, CreateRectRgn( other_pt.x  - windowRect.left, 
				  							  other_pt.y  - windowRect.top,
											  (windowRect.right - windowRect.left) - (windowRect.right - pt2.x),
											  (windowRect.bottom - windowRect.top) - (windowRect.bottom - pt2.y)), TRUE); 
		}
		else {

			SetWindowRgn(hwnd, CreateRectRgn( 0, 
				  							  0,
											  (windowRect.right - windowRect.left),
											  (windowRect.bottom - windowRect.top)), TRUE); 

		}

		if(compact_mode)
			playback->SetVideoRect(skin->GetVideoRect());

		break;

	case WM_PAINT:

		skin->UpdateSize(hwnd);
		skin->Display(hwnd, playback);
		
		if(playback->IsPaused()) {

			playback->MaintainImage();
		}
		
		return 0;
		break;

	case WM_DROPFILES:

		char lpFilename[512];

		if(DragQueryFile( (HDROP) wParam, 0, lpFilename, 512) > 0)  {

			playlist->Reset();
			playlist->AddItem(lpFilename);

			OpenFileForPlaying(hwnd);
		}
		break;

	}

  return DefWindowProc(hwnd, Message, wParam, lParam);
}
