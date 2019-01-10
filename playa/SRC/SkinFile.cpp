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

#include "SkinFile.h"

/*
 * Skin config file class
 */

SkinFile::SkinFile(char *directory) {

	this->configFile = NULL;

	if(directory != NULL) {

		char *filename;

		filename = (char *) new char[strlen(directory) + 12];
		strcpy(filename, directory);

		this->configFile = fopen(strcat(filename, "\\config.txt"), "rt");

		free(filename);
	}

}

SkinFile::~SkinFile() {

}

int SkinFile::getColor(char *section) {

	if(this->configFile) {

		char buffer[256];
		DWORD found = 0;
		int r = 0, g = 0, b = 0;

		fseek(this->configFile, 0, SEEK_SET);

		while(!found) {

			fgets(buffer, 256, this->configFile);
			
			if(strstr(buffer, "[background]") != NULL) {

				fgets(buffer, 256, this->configFile);
				sscanf(buffer, "%d, %d, %d", &r, &g, &b);
			
				found = 1;
			}
		}

		return r + 256*g + 65536*b;
	}

	return 0;
}

void SkinFile::Close() {

	if(this->configFile) {

		fclose(this->configFile);
	}
}
