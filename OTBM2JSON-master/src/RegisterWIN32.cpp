////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

//--FILE DEFINITION-----------------------------------------------------------
//
/*! \file
   File name:  RegisterWIN32.cpp(CRegisterWIN32)

   Author:     Yves Lessard

   Date:       2001/12/19(YYYY/MM/DD)

   Library:		Win32

   Version:    1.01

   Usage:      implementation of the CRegistreWIN32 class
	
   Notes:		See Header infos.

*/
//----------------------------------------------------------------------------

//--FILE INCLUDES-------------------------------------------------------------
#include "RegisterWIN32.h"


//!--CONSTRUCTOR--------------------------------------------------------------
//
//  Method Name:  CRegisterWIN32()
//
/*!
    Notes         Constructor.
*/
//----------------------------------------------------------------------------
CRegisterWIN32::CRegisterWIN32()
{
	//** Default is HKEY_LOCAL_MACHINE
	m_RootKey = HKEY_LOCAL_MACHINE;
	m_hKey = NULL;
   m_pMyCaes = NULL;
	m_pMyCaes = new Rijndael;
   m_pszTemp = new char[MAX_SIZE];
}

//!--DESTRUCTOR----------------------------------------------------------------
//
// Method Name:  ~CRegisterWIN32()
//
/*!

   Notes         Destructor. Clean-up
*/			
//-----------------------------------------------------------------------------
CRegisterWIN32::~CRegisterWIN32()
{
	//** Clean up memory and Close Session
	if ( m_hKey )
		RegCloseKey(m_hKey);
   if(m_pMyCaes)
	   delete m_pMyCaes;
   if(m_pszTemp)
      delete []m_pszTemp;
}


///////////////////////////////////////////////////////////////////////////
//                         Private Functions
///////////////////////////////////////////////////////////////////////////



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   VerifyKey
//
/*!Access:        Private

   Parameters

   IN             Description

   <None>

   OUT            Description

   <BOOL>         TRUE if the Key founded.

   Notes:         
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::VerifyKey()
{
   return ( RegOpenKeyEx(m_RootKey, m_szPath.c_str(), 0L, KEY_ALL_ACCESS, &m_hKey) == ERROR_SUCCESS );
}



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   CreateKey
//
/*!Access:        Private

   Parameters

   IN             Description

   <None>

   OUT            Description

   <BOOL>         TRUE if succeeded else FALSE

   Notes:         We create the Key path.
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::CreateKey()
{
   Close();
	return (RegCreateKeyEx(m_RootKey, m_szPath.c_str(), 0, 0,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
						0, &m_hKey, 0) == ERROR_SUCCESS );
}



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   GetOSversion
//
/*!Access:        Private

   Parameters

   IN             Description

   <None>

   OUT            Description

   <int>          1 NT4 ou Win2000
                  0 = Win9x

   Notes:
*/
//--------------------------------------------------------------------------
int CRegisterWIN32::GetOSversion()
{
	int iResult = 0;
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osv))
	{
		// note: szCSDVersion =  service pack  release  
		switch(osv.dwPlatformId)
		{
			case VER_PLATFORM_WIN32_NT: 
				iResult=1;
				break;
			default:
				iResult=0;
				break;
		}   
	}
   return iResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   SetValue
//
/*!Access:        Private

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <DWORD>        Value as DWORD

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         We can write integer value
                  but we must cast it.
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::SetValue(LPCTSTR _szKey, DWORD _dVal)
{
	//** Write the Value
	return ( RegSetValueEx(m_hKey, _szKey,0,
					REG_DWORD, (CONST BYTE*)&_dVal, 
               sizeof(DWORD)) == ERROR_SUCCESS );
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   StringIN
//
/*!Access:        Private

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   OUT            Description

   <BOOL>         TRUE if succeeded.

   Notes:         String reading.
                  The result is in m_pszTemp.
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::StringIN(LPCTSTR _szKey)
{
	BOOL bResult = FALSE;
   DWORD dwType;
   DWORD dwSize = MAX_SIZE;

	if ( strlen(_szKey) > 0 )
	{
		bResult = ( RegQueryValueEx(m_hKey, _szKey, 0, &dwType,
                              (BYTE*)m_pszTemp, 
                              &dwSize) == ERROR_SUCCESS);
	}
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   GetValue
//
/*!Access:        Private

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         The value is store in m_dTemp
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::GetValue(LPCTSTR _szKey)
{
	BOOL bResult = FALSE;
   DWORD dwType;
	DWORD dwSize = sizeof(DWORD);

	bResult = ( RegQueryValueEx(m_hKey, _szKey, 0, &dwType,
						(BYTE*)&m_dTemp, &dwSize) == ERROR_SUCCESS );
	return bResult;
}



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   DeleteNTway
//
/*!Access:        Private

   Parameters

   IN             Description
   
   <HKEY>         The RootKey

   <LPCTSTR>      The Subkey to Delete

   OUT            Description

   <BOOL>         TRUE if SUcceeded.

   Notes:         Delete each Subkey  
                  This is a recursive Function .
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::DeleteNTway(HKEY _hKey, LPCTSTR _szSubKey)
{
	BOOL bResult = FALSE;
   LPTSTR pszTemp = new char[_MAX_PATH];
	HKEY  lhKey;
	FILETIME ft;
	LONG  lResult;
	DWORD dwKeyLen;
	
	//** If Path empty we get out
	if( strlen(_szSubKey) > 0)
   {
      if(RegOpenKeyEx(_hKey, _szSubKey, 0L, KEY_ENUMERATE_SUB_KEYS, 
               &lhKey) == ERROR_SUCCESS )
		{
			//** So far the Key exist
			do
			{
				dwKeyLen = _MAX_PATH;
				lResult = RegEnumKeyEx(lhKey, 0, pszTemp, &dwKeyLen, NULL, NULL,
						      NULL, &ft);
				switch (lResult)
				{
					case ERROR_NO_MORE_ITEMS:
						//** No more Subkey so delete the base
						if ( RegDeleteKey(_hKey ,_szSubKey) == ERROR_SUCCESS )
						{
							bResult= TRUE;
							break;
						}
						break;
					case ERROR_SUCCESS:
						if( DeleteNTway(lhKey, pszTemp) )
							bResult = TRUE;
						break;
				}
			}while( lResult == ERROR_SUCCESS);
		RegCloseKey(lhKey);
		}
   }
   if(pszTemp)
      delete []pszTemp;
	return bResult;
}




///////////////////////////////////////////////////////////////////////////
//                         Public Functions
///////////////////////////////////////////////////////////////////////////



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   Open
//
/*!Access:        Public

   Parameters

   IN             Description

   <HKEY>         Root Key

   <LPCTSTR>      Sub Key Name

   <BOOL>         TRUE ->Read Mode Only else Read/Write

   OUT            Description

   <BOOL>         TRUE if succeeded.

   Notes:         Open a registry Session (1st Method)  
                  Example:
                  m_MyRegTool.Open(HKEY_CURRENT_USER, "Software\\SevySoft\\Admin");
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::Open(HKEY _hKeyRoot, LPCTSTR _szPath, 
                          BOOL _bReadOnly)
{
	BOOL bResult = FALSE;
   m_szPath.erase();

	//** If a Session Key Opened then close it
   Close();

	//** 5 Roots Key choice 
	if ( _hKeyRoot == HKEY_CLASSES_ROOT || _hKeyRoot == HKEY_CURRENT_USER ||
		 _hKeyRoot == HKEY_LOCAL_MACHINE || _hKeyRoot == HKEY_USERS 
       || _hKeyRoot == HKEY_CURRENT_CONFIG ) 
		{
			//** Save RootKey for reference
			m_RootKey = _hKeyRoot;
			if ( strlen(_szPath) > 0 )
			{
				//** We have a path so save it
            m_szPath = _szPath;
				switch (_bReadOnly)
				{
				case TRUE:
					//** Read Mode Only
					bResult = VerifyKey();
					break;
				default:
					//** Else Read/Write
					if ( !VerifyKey() )
						//** Key not Found so create it
						bResult = CreateKey();
               break;
				}
			}
		}
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   Open
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      Sub Key Name

   <BOOL>         TRUE -> Read Mode (Default)

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         Open a registry Session (2nd Method) 
                  The Key is HKEY_LOCAL_MACHINE.
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::Open(LPCTSTR _szPath, BOOL _bReadOnly)
{
	BOOL bResult = FALSE;
   m_szPath.erase();

	//** If Session Key Opened then close it
   Close();

	//** Always use this one
	m_RootKey = HKEY_LOCAL_MACHINE;
	if ( strlen(_szPath) > 0 )
	{
      m_szPath = _szPath;
		switch (_bReadOnly)
		{
			case TRUE:
				//** Read only mode 
				bResult = VerifyKey();
				break;
			default:
				//** Read Write Mode
				if ( !VerifyKey() )
						//** If Path not Found Create it
						bResult = CreateKey();
            break;
		}
	}
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   Close
//
/*!Access:        Public

   Parameters

   IN             Description

   <None>

   OUT            Description

   <None>

   Notes:         Close a Registry Session.
*/
//--------------------------------------------------------------------------
void CRegisterWIN32::Close()
{
	if ( m_hKey )
		RegCloseKey(m_hKey);
	m_hKey = NULL;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   Write
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key

   <LPCTSTR>      The string value to write

   OUT            Description

   <BOOL>         TRUE if succeeded.

   Notes:         Must at least have 1 char to write.
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::Write(LPCTSTR _szKey, LPCTSTR _szValue)
{
	BOOL bResult = FALSE;

	DWORD dLen = strlen(_szValue);
	if ( dLen > 0)
	{
		dLen++;
		bResult = ( RegSetValueEx(m_hKey, _szKey,0,
						REG_SZ, (CONST BYTE*)_szValue,
						dLen ) == ERROR_SUCCESS );
	}
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   WriteInt
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <int>          The int val to write

   OUT            Description

   <BOOL>         TRUE if succeeded.

   Notes:
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::WriteInt(LPCTSTR _szKey, int _iVal)
{
	return SetValue(_szKey, (DWORD)_iVal);
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   WriteDword
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <DWORD>        The value.

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::WriteDword(LPCTSTR _szKey, DWORD _dwVal)
{
	return SetValue(_szKey, _dwVal);
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   WriteArray
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <LPBYTE>       The BYTE array

   <DWORD>        The Array size

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         Write a Binary value array
         Example: BYTE MonByte[10]={1,2,3,4,5,6,7,8,9,10}; 
                  MyReg.Write("BinVal", MonByte, 10);
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::WriteArray(LPCTSTR _szKey, LPBYTE _pValue, DWORD _nLen)
{
	return ( RegSetValueEx(m_hKey, _szKey, 0, REG_BINARY, _pValue, _nLen) == ERROR_SUCCESS );
}



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   Read
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name.

   <LPCTSTR>      Default value if not founded.

   OUT            Description

   <LPCTSTR>      The readed result.

   Notes
*/
//--------------------------------------------------------------------------
LPCTSTR CRegisterWIN32::Read(LPCTSTR _szKey, LPCTSTR _szDefault)
{

   m_szTemp.erase();
   m_szTemp = _szDefault;
   
   if ( StringIN(_szKey) )
      m_szTemp = m_pszTemp;

   return m_szTemp.c_str();
}



//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   ReadInt
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <int>          Default value

   OUT            Description

   <int>          The int result.

   Notes
*/
//--------------------------------------------------------------------------
int CRegisterWIN32::ReadInt(LPCTSTR _szKey, int _iDefaultVal)
{

	if ( GetValue(_szKey) )
		return (int)m_dTemp;
   else
      return _iDefaultVal;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   ReadDword
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <DWORD>        Default Value

   OUT            Description

   <DWORD>        The result value.

   Notes
*/
//--------------------------------------------------------------------------
DWORD CRegisterWIN32::ReadDword(LPCTSTR _szKey, DWORD _dwDefaultVal)
{
   if ( GetValue(_szKey) )
	   return m_dTemp;
   else
      return _dwDefaultVal;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   ReadArray
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name.

   <LPBYTE>       The Byte Array

   OUT            Description

   <DWORD>        The Number of BYTE readed         

   Notes:         Example: BYTE MyBytes[25];
					   MyReg.ReadArray("ArrayKey", MyBytes);
*/
//--------------------------------------------------------------------------
DWORD CRegisterWIN32::ReadArray(LPCTSTR _szKey, LPBYTE _pValue)
{
	if ( VerifyKey() )
	{
		if( RegQueryValueEx(m_hKey, _szKey, NULL, NULL, _pValue, &m_dTemp) == ERROR_SUCCESS )
         return m_dTemp;
      else
         return 0;
	}
   return 0;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   DeleteValue
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         Delete a Key Value 
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::DeleteValue(LPCTSTR _szKey)
{
	BOOL bResult = FALSE;
	//** The Key Value to Delete
   return ( RegDeleteValue(m_hKey, _szKey) == ERROR_SUCCESS );
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   DeleteKey
//
/*!Access:        Public

   Parameters

   IN             Description

   <HKEY>         The KeyRoot

   <LPCTSTR>      Sub Key Name

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         With WinNT we need a special Function 
                  You don't need to use Open for this Function
         Example: MyReg.DeleteKey(HKEY_CURRENT_USER, _T("Software\\SevySoft"));
                  The Function will extract the Path = Software
                  Then delete the Key SevySoft with all SubDir and Value Inside
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::DeleteKey(HKEY _hKeyRoot, LPCTSTR _szPath)
{
	BOOL bResult  = FALSE;

	LPTSTR pszSubKey = new char[_MAX_PATH];
	LPTSTR pszPath = new char[_MAX_PATH];
	LPTSTR pDest= NULL;
	LPTSTR pTemp = NULL;
	//** If Path empty we get out
	if( strlen(_szPath) > 0)
   {
      pDest= strrchr(_szPath, '\\');
		if (pDest)
		{
			//** Must extract the Last Key
			pTemp = pDest;
			pDest++;
			//** Copy the Key or SubKey to delete
			strcpy(pszSubKey, pDest);
			//** We must now extract the Path
			pDest = pTemp;
			int iCount=0;
			int Result=0;
			Result = pDest - _szPath ;
			do
			{
				pszPath[iCount] = _szPath[iCount];
				iCount++;
			}while ( iCount < Result );
			//** Add end of string mark
			pszPath[iCount]= '\0';
		}
		else
		{
			//** We want to delete a path
			strcpy(pszSubKey, _szPath);
			//** There is no path
			strcpy(pszPath, "");
		}

		HKEY hKey;
		//** Check if Path exist ...
		if ( RegOpenKeyEx(_hKeyRoot, pszPath, 0L, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
      {
         //** Check NT or 2000
		   if ( GetOSversion() == 1 )
		   {
			   //** NT system must use Recursive Delete
			   if( DeleteNTway(hKey, pszSubKey) )
				bResult = TRUE;
		   }
		   else
		   {
			   //** Windows 95 or 98
			if ( RegDeleteKey(hKey , pszSubKey) == ERROR_SUCCESS )
				bResult= TRUE;
		   }
         RegCloseKey(hKey);
       }
   }

   if(pszSubKey)  
      delete pszSubKey;
   if (pszPath)
      delete pszPath;
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   WriteEncrypt
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name

   <LPCTSTR>      The String to Write

   <LPCTSTR>      The Password Key

   OUT            Description

   <BOOL>         TRUE if Succeeded.

   Notes:         We use the Rijndael encryption
                  If no PassKey submited then take a GUID PassKey
*/
//--------------------------------------------------------------------------
BOOL CRegisterWIN32::WriteEncrypt(LPCTSTR _szKey, LPCTSTR _szString, 
                                  LPCTSTR _szPassKey)
{
	BOOL  bResult = FALSE;
   char  szDataOut[16]/*= {NULL}*/;
   DWORD dLen;

	char* pszPassWord= new char[MAX_SIZE];
	if ( strlen(_szKey) > 0)
	{
		if( _szPassKey == NULL || strlen(_szPassKey) == 0 )
			//** No Password so use default
			strcpy(pszPassWord, "{6A67930C-7679-4e91-8580-3AEC80AFF8E0}");
		else
			strcpy(pszPassWord, _szPassKey);

		if( m_pMyCaes->MakeKey(pszPassWord) )
		{
			//** So far the Key is generated
			if( m_pMyCaes->EncryptBlock(_szString, szDataOut) )
			{
				//** Encrypt Block always 16
				dLen =16;
				//** We are now ready to Write
				bResult = ( RegSetValueEx(m_hKey, _szKey,0,
						   REG_BINARY, (CONST BYTE*)szDataOut,
						   dLen ) == ERROR_SUCCESS );
			}
		}
	}
	delete []pszPassWord;
	return bResult;
}


//-METHOD IMPLEMENTATION----------------------------------------------------
//
// Method Name:   ReadEncrypt
//
/*!Access:        Public

   Parameters

   IN             Description

   <LPCTSTR>      The Key Name.

   <LPCTSTR>      The PassWord.

   OUT            Description

   <LPCTSTR>      The result String.

   Notes:
*/
//--------------------------------------------------------------------------
LPCTSTR CRegisterWIN32::ReadEncrypt(LPCTSTR _szKey, LPCTSTR _szPassKey)
{
	char* pszPassWord= new char[MAX_SIZE];
   DWORD dwLen;

	//if ( StringIN(_szKey) )
   if(RegQueryValueEx(m_hKey, _szKey, NULL, NULL,(BYTE*)m_pszTemp,&dwLen)== ERROR_SUCCESS )
	{
      m_szTemp = m_pszTemp;
		//** Check the PassKey
		if( _szPassKey == NULL || strlen(_szPassKey) == 0 )
			//** No Password so use default
			strcpy(pszPassWord, "{6A67930C-7679-4e91-8580-3AEC80AFF8E0}");
		else
			strcpy(pszPassWord, _szPassKey);

		if( m_pMyCaes->MakeKey(pszPassWord) )
		{
			//** Now Decrypt the String
			if( m_pMyCaes->DecryptBlock(m_szTemp.c_str(), m_pszTemp) )
            m_szTemp =  m_pszTemp;
         else
            m_szTemp.erase();
		}
	}
	delete [] pszPassWord;
   return m_szTemp.c_str();
}
