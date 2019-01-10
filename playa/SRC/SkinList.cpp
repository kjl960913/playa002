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

#include "SkinList.h"
#include <windows.h>
#include <string.h>
#include <stdlib.h>

/* 
 * SkinInfo class
 */

SkinInfo::SkinInfo(char *directory) {

	this->directory = (char *) new char[strlen(directory)+1]; 

	this->directory = strcpy(this->directory, directory);
	this->directory[strlen(directory)] = '\0';

	this->name      = strrchr(this->directory, '\\') + 1;

	if(name == NULL) {

		name = directory;
	}
}

SkinInfo::~SkinInfo() {
	
	free(this->directory);
}

/* 
 * SkinList Class
 */

SkinList::SkinList() {
	
	this->totalSkins = 0;
	this->skins      = NULL;
	this->skinsDir   = NULL;

	/*
	 * Get the list of directories from the Registry
	 */

	HKEY     key;
	DWORD    created, size, type;
	LONG     result;
	


	/*
	 * Open the registry key
	 */

	result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\DivXNetworks\\ThePlaya",
							0, "CONFIG", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
							NULL, &key, &created);

	if(result != ERROR_SUCCESS) {

		MessageBox(NULL, "Couldn't load skins directories", "", MB_OK);
		return;
	}

	switch(created) {

	case REG_CREATED_NEW_KEY:

		/*
		 * First time launch (we keep the default)
		 * 
		 */

		this->skinsDir = NULL;
		
		break;

	case REG_OPENED_EXISTING_KEY:
		
		/*
		 * We can read the values
 		 */

		this->skinsDir = (char *) new char[MAX_PATH];

		size = MAX_PATH;
		result = RegQueryValueEx(key, "SkinsDir", 0, &type, (BYTE *)this->skinsDir, &size);

		if(result == ERROR_MORE_DATA) {

			this->skinsDir = (char *) realloc(this->skinsDir, size);
			result = RegQueryValueEx(key, "SkinsDir", 0, &type, (BYTE *)this->skinsDir, &size);
		}

		if(result != ERROR_SUCCESS) {

			free(this->skinsDir);
			this->skinsDir = NULL;
		}

		break;

	default:
		break;
	}

	RegCloseKey(key);
}

SkinList::~SkinList() {

	free(this->skinsDir);
}

void SkinList::Add(SkinInfo *skinInfo) {

	skinlist_t *node;

	node = (skinlist_t *) new skinlist_t;
	node->info     = skinInfo;
	node->next     = this->skins;

	this->skins = node;

	this->totalSkins++;
}

int SkinList::SetDir(char *dir) {

	this->skinsDir = dir;

	HKEY     key;
	DWORD    created;
	LONG     result;

	/*
	 * Try to open the registry key
	 */

	result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\DivXNetworks\\ThePlaya",
							0, "CONFIG", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
							NULL, &key, &created);

	if(result != ERROR_SUCCESS) {

		MessageBox(NULL, "Couldn't save skins dir", "", MB_OK);
		return 0;
	}

	RegSetValueEx(key, "SkinsDir", 0, REG_SZ, (BYTE *) this->skinsDir, strlen(this->skinsDir));

	return 1;
}

int SkinList::Scan() {

	this->totalSkins = 0;
	this->skins      = NULL;

	if(this->skinsDir != NULL && strcmp(this->skinsDir, "Default") != 0) {
	
		WIN32_FIND_DATA dirData;
		HANDLE          hFind = NULL, hSkin = NULL;
		char           *str;

		str = (char *) new char[(strlen(this->skinsDir))];
		strcpy(str, this->skinsDir);
		
		hFind = FindFirstFile(strcat(str, "\\*.*"), &dirData);

		if(hFind != NULL) {
			
			if(strcmp(dirData.cFileName, ".") != 0 && strcmp(dirData.cFileName, "..") != 0) {
	
				if(dirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

					this->Add(new SkinInfo(dirData.cFileName));
				}
			}

			while(FindNextFile(hFind, &dirData)) {
	
				if(strcmp(dirData.cFileName, ".") != 0 && strcmp(dirData.cFileName, "..") != 0) {

					if(dirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					
						char filename[MAX_PATH];
						
						strcpy(filename, this->skinsDir);
		
						strcat(strcat(filename, "\\"), dirData.cFileName);
	
						this->Add(new SkinInfo(filename));
					}
				}
			}

			FindClose(hFind);
		}
	}

	return 0;
}

int Reset() {

	return 0;
}

int SkinList::getNumberOfSkins() {

	return this->totalSkins;
}

SkinInfo *SkinList::getSkinInfo(int position) {

	if(position >= 0 && position < this->totalSkins) {

		int         i;
		skinlist_t *node;

		node = this->skins;

		for(i = 0; i < position; i++) {

			node = node->next;
		}

		return node->info;
	}

	return NULL;
}
