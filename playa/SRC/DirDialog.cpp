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

#include "DirDialog.h"
#include <shlobj.h>

static int __stdcall BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  CDirDialog* pDirDialogObj = (CDirDialog*)lpData;

  if (uMsg == BFFM_INITIALIZED && !pDirDialogObj->m_strSelDir != NULL)
  {
  
  }
  else // uMsg == BFFM_SELCHANGED
  {

  }

  return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirDialog::CDirDialog()
{
	this->m_strPath    = (char *) new char[32768];
    this->m_strInitDir = NULL;
    this->m_strSelDir  = NULL;
    this->m_strTitle   = NULL;
}

CDirDialog::~CDirDialog()
{

}

int CDirDialog::DoBrowse()
{

  BROWSEINFO bInfo;
  LPITEMIDLIST pidl;
  ZeroMemory ( (PVOID) &bInfo,sizeof (BROWSEINFO));

  if (m_strInitDir != NULL)
  {
    OLECHAR       olePath[MAX_PATH];
    ULONG         chEaten;
    ULONG         dwAttributes;
    HRESULT       hr;
    LPSHELLFOLDER pDesktopFolder;

    // 
    // Get a pointer to the Desktop's IShellFolder interface. 
    //
    
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
    {
      //
      // IShellFolder::ParseDisplayName requires the file name be in Unicode.
      //

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strInitDir, -1,
                          olePath, MAX_PATH);

      //
      // Convert the path to an ITEMIDLIST.
      //
      hr = pDesktopFolder->ParseDisplayName(NULL,
                                            NULL,
                                            olePath,
                                            &chEaten,
                                            &pidl,
                                            &dwAttributes);
      bInfo.pidlRoot = pidl;

    }
  }
 
  bInfo.hwndOwner      = NULL;
  bInfo.pszDisplayName = m_strPath;
  bInfo.lpszTitle      = "Select Directory";
  bInfo.ulFlags        = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
  bInfo.lpfn           = BrowseCtrlCallback;
  bInfo.lParam         = (LPARAM)this;

  if ((pidl = SHBrowseForFolder(&bInfo)) == NULL)
  {
	  return 0;
  }

  m_iImageIndex = bInfo.iImage;

  if (SHGetPathFromIDList(pidl, m_strPath) == FALSE)
  {
    return 0;
  }

  return 1;
}
