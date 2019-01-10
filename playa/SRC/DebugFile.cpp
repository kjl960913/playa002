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

/*
 * A 'Debug' Class
 *
 */

#include "DebugFile.h"

DebugFile::DebugFile(char *filename)
{
	this->debugFile = fopen(filename, "w");
}

DebugFile::~DebugFile()
{
	fclose(this->debugFile);
}

void DebugFile::DebugInt(char *text, int d)
{
	fprintf(this->debugFile, text, d);
	fprintf(this->debugFile, "\n");
}

void DebugFile::DebugFloat(char *text, float d)
{
	fprintf(this->debugFile, text, d);
	fprintf(this->debugFile, "\n");
}
