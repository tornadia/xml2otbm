////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

//--FILE DEFINITION-----------------------------------------------------------
//
/*! \file
   File name:  RegisterWIN32.h(CRegisterWIN32)

   Author:     Yves Lessard

   Date:       2001/12/19(YYYY/MM/DD)

   Library:		Win32

   Version:    1.01

   Usage:      Some registry functions in WIN32 Environnment
	
   Notes:		Include Encrypting Function

   There are 2 ways to initialize a Session Key
   1) Open(RootKey,SubKey,Mode)	Open Session RootKey and Path
                                 By default Mode is Read Only
   2) Open(Subkey, Mode)         Rootkey = HKEY_LOCAL_MACHINE (default)


   Method(Public)       Description
   ------------------   --------------------------------------	
   Close	               Close a Session Key
   Write                Write a string value
   WriteInt             Write int value
   WriteDword           Write a DWORD value
   WriteArray           Write a BYTE array
   WriteStruct          For all other kind of Data
                           Float, Double, CRect, etc
   Read                 Read a String value
   ReadInt              Read int value
   ReadDword            Read a DWORD
   ReadArray            Read BYTE Array
   ReadStruct           Read all other kind of Data
   DeleteValue          Delete a Key Value
   DeleteKey            Delete a Key or SubKey (Recursive)

   Encrypting Section & Decrypting Section
   WriteEncrypt         Encrypt a String then Write to Registry
   ReadEncrypt          Read a Registry Key then Decrypt

      
   Modifications:
   Name        Date        Notes
   C.Vachon    2002/09/05  Encrypt/Decrypt as Binary into Registry
*/
//----------------------------------------------------------------------------
#if !defined(AFX_REGISTERWIN32_H__305C4B07_38D6_11D5_8068_0050BAB07D8B__INCLUDED_)
#define AFX_REGISTERWIN32_H__305C4B07_38D6_11D5_8068_0050BAB07D8B__INCLUDED_

#if _MSC_VER > 1001
#pragma once
#endif // _MSC_VER > 1001

//--FILE INCLUDES-------------------------------------------------------------
#include <windows.h>
#include <string>
#include "Rijndael.h"	   //** Encrypting Class

//--LOCAL DEFINITIONS---------------------------------------------------------
#define MAX_SIZE 1024    //** Maximum len of String


//--CLASS DEFINITION----------------------------------------------------------
//
//  Class name:   CRegisterWIN32
//
/*! Notes:        
*/
//----------------------------------------------------------------------------
class CRegisterWIN32  
{
public:
   //--CONSTRUCTOR-------------------------------------
   CRegisterWIN32();
   //--DESTRUCTOR--------------------------------------
   virtual ~CRegisterWIN32();

public:
   //--PUBLIC METHODS----------------------------------
  	BOOL  Open(HKEY _hKeyRoot, LPCTSTR _szPath, BOOL _bReadOnly = TRUE);
  	BOOL  Open(LPCTSTR _szPath, BOOL _bReadOnly = TRUE);
   void  Close();
   BOOL  DeleteKey(HKEY _hKeyRoot, LPCTSTR _szPath);
	BOOL  DeleteValue(LPCTSTR _szKey);

   BOOL  Write(LPCTSTR _szKey, LPCTSTR _szValue);
   BOOL  WriteInt(LPCTSTR _szKey, int _iVal);
   BOOL  WriteDword(LPCTSTR _szKey, DWORD _dwVal);
   BOOL  WriteArray(LPCTSTR _szKey, LPBYTE _pValue, DWORD _nLen);

   LPCTSTR  Read(LPCTSTR _szKey, LPCTSTR _szDefault="");
   int      ReadInt(LPCTSTR _szKey, int _iDefaultVal=0);
   DWORD    ReadDword(LPCTSTR _szKey, DWORD _dwDefaultVal=0);
   DWORD    ReadArray(LPCTSTR _szKey, LPBYTE _pValue);

   //** Encrypt/ Decrypt Function
  	BOOL     WriteEncrypt(LPCTSTR _szKey, LPCTSTR _szString, LPCTSTR _szPassKey= NULL);
	LPCTSTR  ReadEncrypt(LPCTSTR _szKey, LPCTSTR _szPassKey= NULL);

   //**************
	//  WriteStruct
	//**************
	template <class T>BOOL WriteStruct(LPCTSTR _szKey, T& _obj)
	{
      //** Translate data to Binary
      return ( RegSetValueEx(m_hKey, _szKey, 0, REG_BINARY,(LPBYTE)& _obj, 
                  sizeof(T)) == ERROR_SUCCESS );
	}

	//********************
	// ReadStruct
	//********************
	template <class T>BOOL ReadStruct(LPCTSTR _szKey, T& _obj)
	{
      //** Read the data
      DWORD dwLen = sizeof(T);
      return ( RegQueryValueEx(m_hKey, _szKey, NULL, NULL, (LPBYTE)& _obj,
                  &dwLen) == ERROR_SUCCESS );
	}

private:
   //--PRIVATE ATTRIBUTES------------------------------
	Rijndael*   m_pMyCaes;
	DWORD       m_dTemp;
	HKEY        m_RootKey;
	int         iOSversion;
	HKEY        m_hKey;
   std::string m_szPath;
   char*       m_pszTemp;
   std::string m_szTemp;

   //--PRIVATE METHODS---------------------------------
   BOOL  SetValue(LPCTSTR _szKey, DWORD _dVal);
   BOOL  CreateKey();
	BOOL  VerifyKey();
   BOOL  StringIN(LPCTSTR _szKey);
   BOOL  DeleteNTway(HKEY _hKey, LPCTSTR _szSubKey);
	int   GetOSversion();
	BOOL  GetValue(LPCTSTR _szKey);
};
#endif // !defined(AFX_REGISTERWIN32_H__305C4B07_38D6_11D5_8068_0050BAB07D8B__INCLUDED_)
