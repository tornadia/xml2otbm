////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

//--FILE DEFINITION-----------------------------------------------------------
//
/*! \file
   File name:  Rijndael.h

   Author:     Yves Lessard

   Date:       2000/04/27(YYYY/MM/DD)

   Library:		Win32.

   Version:    1.00

   Usage:      Rijndael (pronounced rain-dahl) is the block cipher algorithm 
               that has been selected by the U.S
	
   Notes:		I only use the 16 Bytes Block Encryption 


   Method(Public)       Description
   ------------------   --------------------------------------	
   MakeKey              The Encrytion Key (or password)
   EncryptBlock         Encrypt a string.
   DecryptBlock         Decrypt a string


   Modifications:
   Name     Date        Notes
*/
//----------------------------------------------------------------------------
#if !defined(AFX_AES_H__0AA65B08_0F8C_4754_BB64_BA53244300E3__INCLUDED_)
#define AFX_AES_H__0AA65B08_0F8C_4754_BB64_BA53244300E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <windows.h>

//--LOCAL DEFINITIONS---------------------------------------------------------
#define KEYLENGTH = 16;
typedef unsigned char BYTE;

//--CLASS DEFINITION----------------------------------------------------------
//
//  Class name:   Rijndael
//
/*! Notes:        Block cipher algorithm. 
*/
//----------------------------------------------------------------------------
class Rijndael  
{
public:
   //--PUBLIC METHODS----------------------------------
	BOOL DecryptBlock(LPCTSTR _szDataIn, LPTSTR _szDataOut);
	BOOL EncryptBlock(LPCTSTR _szString, LPTSTR _szResult);
	BOOL MakeKey(LPCTSTR _szKey);

   //--CONSTRUCTOR-------------------------------------
	Rijndael();
   //--DESTRUCTOR--------------------------------------
	virtual ~Rijndael();

private:
   //--PRIVATE ATTRIBUTES------------------------------
   int   m_KeyLength;
   int   m_Block_Size;
   BOOL  m_bKeyInit;     
	
	int   m_iROUNDS;     //Number of Rounds
   int** m_Kd;          //Decryption (m_Kd) round key
	int** m_Ke;          //Encryption (m_Ke) round key

   static const int sm_alog[256];
	static const int sm_log[256];
	static const char sm_S[256];
   static const char sm_Si[256];
   static const int sm_T1[256];
   static const int sm_T2[256];
   static const int sm_T3[256];
   static const int sm_T4[256];
   static const int sm_T5[256];
   static const int sm_T6[256];
   static const int sm_T7[256];
   static const int sm_T8[256];
   static const int sm_U1[256];
   static const int sm_U2[256];
   static const int sm_U3[256];
   static const int sm_U4[256];
   static const char sm_rcon[30];
   static const int sm_shifts[3][4][2];
};

#endif // !defined(AFX_AES_H__0AA65B08_0F8C_4754_BB64_BA53244300E3__INCLUDED_)
