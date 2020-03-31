/**
 *	PROJECT - HTML Reader Class Library
 *
 *	LiteHTMLEntityResolver.h - Defines CLiteHTMLEntityResolver
 *
 *	Written By Gurmeet S. Kochar <gomzygotit@hotmail.com>
 *	Copyright (c) 2004. All rights reserved.
 *
 *	This code may be used in compiled form in any way you desire
 *	(including commercial use). The code may be redistributed
 *	unmodified by any means PROVIDING it is not sold for profit
 *	without the authors written consent, and providing that this
 *	notice and the authors name and all copyright notices remains
 *	intact. However, this file and the accompanying source code may
 *	not be hosted on a website or bulletin board without the authors
 *	written permission.
 *
 *	This file is provided "AS IS" with no expressed or implied warranty.
 *	The author accepts no liability for any damage/loss of business that
 *	this product may cause.
 *
 *	Although it is not necessary, but if you use this code in any of
 *	your application (commercial or non-commercial), please INFORM me
 *	so that I may know how useful this library is. This will encourage
 *	me to keep updating it.
 *
 *	HISTORY:
 *
 *	Version 1.0				Gurmeet S. Kochar (GSK)
 *	Mar 21, 2004			First release version.
 */
#ifndef __LITEHTMLENTITYRESOLVER_H__
#define __LITEHTMLENTITYRESOLVER_H__

 /*
  * Conditional Includes
  */
#ifndef __AFXTEMPL_H__
#	include <afxtempl.h>
#endif	// !__AFXTEMPL_H__

#ifndef __LITEHTMLCOMMON_H__
#	include "LiteHTMLCommon.h"
#endif	// !__LITEHTMLCOMMON_H__

#pragma warning(push, 4)

  /**
   * CLiteHTMLEntityResolver
   * This is a utility class that helps in resolving the character
   * and numeric entity references.
   *
   * @version 1.0 (Mar 17, 2004)
   * @author Gurmeet S. Kochar
   */
class CLiteHTMLEntityResolver
{
private:
	class CCharEntityRefs : public CMap<CString, LPCTSTR, TCHAR, TCHAR>
	{
	public:
		CCharEntityRefs() : CMap<CString, LPCTSTR, TCHAR, TCHAR>(55)
		{
			/** Math Symbols */
			(*this)[_T("forall")] = _T('\x2200');
			(*this)[_T("part")] = _T('\x2202');
			(*this)[_T("exist")] = _T('\x2203');
			(*this)[_T("empty")] = _T('\x2205');
			(*this)[_T("nabla")] = _T('\x2207');
			(*this)[_T("isin")] = _T('\x2208');
			(*this)[_T("notin")] = _T('\x2209');
			(*this)[_T("ni")] = _T('\x220b');
			(*this)[_T("prod")] = _T('\x220f');
			(*this)[_T("sum")] = _T('\x2211');
			(*this)[_T("minus")] = _T('\x2212');
			(*this)[_T("lowast")] = _T('\x2217');
			(*this)[_T("radic")] = _T('\x221a');
			(*this)[_T("prop")] = _T('\x221d');
			(*this)[_T("infin")] = _T('\x221e');
			(*this)[_T("ang")] = _T('\x2220');
			(*this)[_T("and")] = _T('\x2227');
			(*this)[_T("or")] = _T('\x2228');
			(*this)[_T("cap")] = _T('\x2229');
			(*this)[_T("cup")] = _T('\x222a');
			(*this)[_T("int")] = _T('\x222b');
			(*this)[_T("there4")] = _T('\x2234');
			(*this)[_T("sim")] = _T('\x223c');
			(*this)[_T("cong")] = _T('\x2245');
			(*this)[_T("asymp")] = _T('\x2248');
			(*this)[_T("ne")] = _T('\x2260');
			(*this)[_T("equiv")] = _T('\x2261');
			(*this)[_T("le")] = _T('\x2264');
			(*this)[_T("ge")] = _T('\x2265');
			(*this)[_T("sub")] = _T('\x2282');
			(*this)[_T("sup")] = _T('\x2283');
			(*this)[_T("nsub")] = _T('\x2284');
			(*this)[_T("sube")] = _T('\x2286');
			(*this)[_T("supe")] = _T('\x2287');
			(*this)[_T("oplus")] = _T('\x2295');
			(*this)[_T("otimes")] = _T('\x2297');
			(*this)[_T("perp")] = _T('\x22a5');
			(*this)[_T("sdot")] = _T('\x22c5');

			/** Greek Letters */
			(*this)[_T("Alpha")] = _T('\x391');
			(*this)[_T("Beta")] = _T('\x392');
			(*this)[_T("Gamma")] = _T('\x393');
			(*this)[_T("Delta")] = _T('\x394');
			(*this)[_T("Epsilon")] = _T('\x395');
			(*this)[_T("Zeta")] = _T('\x396');
			(*this)[_T("Eta")] = _T('\x397');
			(*this)[_T("Theta")] = _T('\x398');
			(*this)[_T("Iota")] = _T('\x399');
			(*this)[_T("Kappa")] = _T('\x39a');
			(*this)[_T("Lambda")] = _T('\x39b');
			(*this)[_T("Mu")] = _T('\x39c');
			(*this)[_T("Nu")] = _T('\x39d');
			(*this)[_T("Xi")] = _T('\x39e');
			(*this)[_T("Omicron")] = _T('\x39f');
			(*this)[_T("Pi")] = _T('\x3a0');
			(*this)[_T("Rho")] = _T('\x3a1');
			(*this)[_T("Sigma")] = _T('\x3a3');
			(*this)[_T("Tau")] = _T('\x3a4');
			(*this)[_T("Upsilon")] = _T('\x3a5');
			(*this)[_T("Phi")] = _T('\x3a6');
			(*this)[_T("Chi")] = _T('\x3a7');
			(*this)[_T("Psi")] = _T('\x3a8');
			(*this)[_T("Omega")] = _T('\x3a9');
			(*this)[_T("alpha")] = _T('\x3b1');
			(*this)[_T("beta")] = _T('\x3b2');
			(*this)[_T("gamma")] = _T('\x3b3');
			(*this)[_T("delta")] = _T('\x3b4');
			(*this)[_T("epsilon")] = _T('\x3b5');
			(*this)[_T("zeta")] = _T('\x3b6');
			(*this)[_T("eta")] = _T('\x3b7');
			(*this)[_T("theta")] = _T('\x3b8');
			(*this)[_T("iota")] = _T('\x3b9');
			(*this)[_T("kappa")] = _T('\x3ba');
			(*this)[_T("lambda")] = _T('\x3bb');
			(*this)[_T("mu")] = _T('\x3bc');
			(*this)[_T("nu")] = _T('\x3bd');
			(*this)[_T("xi")] = _T('\x3be');
			(*this)[_T("omicron")] = _T('\x3bf');
			(*this)[_T("pi")] = _T('\x3c0');
			(*this)[_T("rho")] = _T('\x3c1');
			(*this)[_T("sigmaf")] = _T('\x3c2');
			(*this)[_T("sigma")] = _T('\x3c3');
			(*this)[_T("tau")] = _T('\x3c4');
			(*this)[_T("upsilon")] = _T('\x3c5');
			(*this)[_T("phi")] = _T('\x3c6');
			(*this)[_T("chi")] = _T('\x3c7');
			(*this)[_T("psi")] = _T('\x3c8');
			(*this)[_T("omega")] = _T('\x3c9');
			(*this)[_T("thetasym")] = _T('\x3d1');
			(*this)[_T("upsih")] = _T('\x3d2');
			(*this)[_T("piv")] = _T('\x3d6');

			/** Other Entities */
			(*this)[_T("OElig")] = _T('\x152');
			(*this)[_T("oelig")] = _T('\x153');
			(*this)[_T("Scaron")] = _T('\x160');
			(*this)[_T("scaron")] = _T('\x161');
			(*this)[_T("Yuml")] = _T('\x178');
			(*this)[_T("fnof")] = _T('\x192');
			(*this)[_T("circ")] = _T('\x2c6');
			(*this)[_T("tilde")] = _T('\x2dc');
			(*this)[_T("ensp")] = _T('\x2002');
			(*this)[_T("emsp")] = _T('\x2003');
			(*this)[_T("thinsp")] = _T('\x2009');
			(*this)[_T("zwnj")] = _T('\x200c');
			(*this)[_T("zwj")] = _T('\x200d');
			(*this)[_T("lrm")] = _T('\x200e');
			(*this)[_T("rlm")] = _T('\x200f');
			(*this)[_T("ndash")] = _T('\x2013');
			(*this)[_T("mdash")] = _T('\x2014');
			(*this)[_T("lsquo")] = _T('\x2018');
			(*this)[_T("rsquo")] = _T('\x2019');
			(*this)[_T("sbquo")] = _T('\x201a');
			(*this)[_T("ldquo")] = _T('\x201c');
			(*this)[_T("rdquo")] = _T('\x201d');
			(*this)[_T("bdquo")] = _T('\x201e');
			(*this)[_T("dagger")] = _T('\x2020');
			(*this)[_T("Dagger")] = _T('\x2021');
			(*this)[_T("bull")] = _T('\x2022');
			(*this)[_T("hellip")] = _T('\x2026');
			(*this)[_T("permil")] = _T('\x2030');
			(*this)[_T("prime")] = _T('\x2032');
			(*this)[_T("Prime")] = _T('\x2033');
			(*this)[_T("lsaquo")] = _T('\x2039');
			(*this)[_T("rsaquo")] = _T('\x203a');
			(*this)[_T("oline")] = _T('\x203e');
			(*this)[_T("euro")] = _T('\x20ac');
			(*this)[_T("trade")] = _T('\x2122');
			(*this)[_T("larr")] = _T('\x2190');
			(*this)[_T("uarr")] = _T('\x2191');
			(*this)[_T("rarr")] = _T('\x2192');
			(*this)[_T("darr")] = _T('\x2193');
			(*this)[_T("harr")] = _T('\x2194');
			(*this)[_T("crarr")] = _T('\x21b5');
			(*this)[_T("lceil")] = _T('\x2308');
			(*this)[_T("rceil")] = _T('\x2309');
			(*this)[_T("lfloor")] = _T('\x230a');
			(*this)[_T("rfloor")] = _T('\x230b');
			(*this)[_T("loz")] = _T('\x25ca');
			(*this)[_T("spades")] = _T('\x2660');
			(*this)[_T("clubs")] = _T('\x2663');
			(*this)[_T("hearts")] = _T('\x2665');
			(*this)[_T("diams")] = _T('\x2666');
			/** C0 Controls and Basic Latin */
			(*this)[_T("quot")] = _T('\x22');
			(*this)[_T("amp")] = _T('\x26');
			(*this)[_T("apos")] = _T('\x27');
			(*this)[_T("lt")] = _T('\x3C');
			(*this)[_T("gt")] = _T('\x3E');
			/** ISO 8859-1 (Latin-1) characters */
			(*this)[_T("nbsp")] = _T('\xA0');
			(*this)[_T("iexcl")] = _T('\xA1');
			(*this)[_T("cent")] = _T('\xA2');
			(*this)[_T("pound")] = _T('\xA3');
			(*this)[_T("current")] = _T('\xA4');
			(*this)[_T("yen")] = _T('\xA5');
			(*this)[_T("brvbar")] = _T('\xA6');
			(*this)[_T("sect")] = _T('\xA7');
			(*this)[_T("uml")] = _T('\xA8');
			(*this)[_T("copy")] = _T('\xA9');
			(*this)[_T("ordf")] = _T('\xAA');
			(*this)[_T("laquo")] = _T('\xAB');
			(*this)[_T("not")] = _T('\xAC');
			(*this)[_T("shy")] = _T('\xAD');
			(*this)[_T("reg")] = _T('\xAE');
			(*this)[_T("macr")] = _T('\xAF');
			(*this)[_T("deg")] = _T('\xB0');
			(*this)[_T("plusmn")] = _T('\xB1');
			(*this)[_T("sup2")] = _T('\xB2');
			(*this)[_T("sup3")] = _T('\xB3');
			(*this)[_T("acute")] = _T('\xB4');
			(*this)[_T("micro")] = _T('\xB5');
			(*this)[_T("para")] = _T('\xB6');
			(*this)[_T("middot")] = _T('\xB7');
			(*this)[_T("cedil")] = _T('\xB8');
			(*this)[_T("sup1")] = _T('\xB9');
			(*this)[_T("ordm")] = _T('\xBA');
			(*this)[_T("raquo")] = _T('\xBB');
			(*this)[_T("frac14")] = _T('\xBC');
			(*this)[_T("frac12")] = _T('\xBD');
			(*this)[_T("frac34")] = _T('\xBE');
			(*this)[_T("iquest")] = _T('\xBF');
			(*this)[_T("Agrave")] = _T('\xC0');
			(*this)[_T("Aacute")] = _T('\xC1');
			(*this)[_T("Acirc")] = _T('\xC2');
			(*this)[_T("Atilde")] = _T('\xC3');
			(*this)[_T("Auml")] = _T('\xC4');
			(*this)[_T("Aring")] = _T('\xC5');
			(*this)[_T("AElig")] = _T('\xC6');
			(*this)[_T("Ccedil")] = _T('\xC7');
			(*this)[_T("Egrave")] = _T('\xC8');
			(*this)[_T("Eacute")] = _T('\xC9');
			(*this)[_T("Ecirc")] = _T('\xCA');
			(*this)[_T("Euml")] = _T('\xCB');
			(*this)[_T("Igrave")] = _T('\xCC');
			(*this)[_T("Iacute")] = _T('\xCD');
			(*this)[_T("Icirc")] = _T('\xCE');
			(*this)[_T("Iuml")] = _T('\xCF');
			(*this)[_T("ETH")] = _T('\xD0');
			(*this)[_T("Ntilde")] = _T('\xD1');
			(*this)[_T("Ograve")] = _T('\xD2');
			(*this)[_T("Oacute")] = _T('\xD3');
			(*this)[_T("Ocirc")] = _T('\xD4');
			(*this)[_T("Otilde")] = _T('\xD5');
			(*this)[_T("Ouml")] = _T('\xD6');
			(*this)[_T("times")] = _T('\xD7');
			(*this)[_T("Oslash")] = _T('\xD8');
			(*this)[_T("Ugrave")] = _T('\xD9');
			(*this)[_T("Uacute")] = _T('\xDA');
			(*this)[_T("Ucirc")] = _T('\xDB');
			(*this)[_T("Uuml")] = _T('\xDC');
			(*this)[_T("Yacute")] = _T('\xDD');
			(*this)[_T("THORN")] = _T('\xDE');
			(*this)[_T("szlig")] = _T('\xDF');
			(*this)[_T("agrave")] = _T('\xE0');
			(*this)[_T("aacute")] = _T('\xE1');
			(*this)[_T("acirc")] = _T('\xE2');
			(*this)[_T("atilde")] = _T('\xE3');
			(*this)[_T("auml")] = _T('\xE4');
			(*this)[_T("aring")] = _T('\xE5');
			(*this)[_T("aelig")] = _T('\xE6');
			(*this)[_T("ccedil")] = _T('\xE7');
			(*this)[_T("egrave")] = _T('\xE8');
			(*this)[_T("eacute")] = _T('\xE9');
			(*this)[_T("ecirc")] = _T('\xEA');
			(*this)[_T("euml")] = _T('\xEB');
			(*this)[_T("igrave")] = _T('\xEC');
			(*this)[_T("iacute")] = _T('\xED');
			(*this)[_T("icirc")] = _T('\xEE');
			(*this)[_T("iuml")] = _T('\xEF');
			(*this)[_T("eth")] = _T('\xF0');
			(*this)[_T("ntilde")] = _T('\xF1');
			(*this)[_T("ograve")] = _T('\xF2');
			(*this)[_T("oacute")] = _T('\xF3');
			(*this)[_T("ocirc")] = _T('\xF4');
			(*this)[_T("otilde")] = _T('\xF5');
			(*this)[_T("ouml")] = _T('\xF6');
			(*this)[_T("divide")] = _T('\xF7');
			(*this)[_T("oslash")] = _T('\xF8');
			(*this)[_T("ugrave")] = _T('\xF9');
			(*this)[_T("uacute")] = _T('\xFA');
			(*this)[_T("ucirc")] = _T('\xFB');
			(*this)[_T("uuml")] = _T('\xFC');
			(*this)[_T("yacute")] = _T('\xFD');
			(*this)[_T("thorn")] = _T('\xFE');
			(*this)[_T("yuml")] = _T('\xFF');
		}
	};

	// Constructors
public:
	CLiteHTMLEntityResolver() { }

	// Operations
public:
	/**
	 * Resolves a character or numeric entity reference.
	 *
	 * @param rStr - entity to resolve
	 * @param ch - variable that will receive the result
	 *
	 * @return number of TCHARs used to resolve entity reference
	 * @since 1.0
	 * @author Gurmeet S. Kochar
	 */
	static UINT resolveEntity(LPCTSTR lpszEntity, TCHAR &chSubst)
	{
		ASSERT(m_CharEntityRefs.GetCount());
		ASSERT(AfxIsValidString(lpszEntity));

		LPCTSTR	lpszBegin = lpszEntity,
			lpszEnd = ::_tcschr(lpszEntity, _T(';'));
		TCHAR	chTemp = 0;

		// entity references always end with a semi-colon ';'
		if (lpszEnd == NULL)
			return (0);

		// skip leading white-space characters
		while (::_istspace(*lpszBegin))
			lpszBegin = ::_tcsinc(lpszBegin);

		// remaining string (including semi-colon) 
		// must be at least 4 characters in length
		if (lpszEnd - lpszBegin < 3)
			return (0U);

		// entity references always begin with an ampersand '&' symbol
		if (*lpszBegin != _T('&'))
			return (0U);
		lpszBegin = ::_tcsinc(lpszBegin);

		// numeric (decimal or hexadecimal) entity reference?
		if (*lpszBegin == _T('#'))
		{
			lpszBegin = ::_tcsinc(lpszBegin);
			chTemp = *lpszBegin;
			int	radix = (::_istdigit(chTemp) ? 10 :
				(chTemp == _T('x') ||
					chTemp == _T('X') ? 16 : 0));
			if (radix)
			{
				if (radix == 16)
					lpszBegin = ::_tcsinc(lpszBegin);

				unsigned long	ulNum = ::_tcstoul(lpszBegin, NULL, radix);
				chSubst = (TCHAR)ulNum;
				lpszEnd = ::_tcsinc(lpszEnd);
				return (lpszEnd - lpszEntity);
			}
		}

		// character entity reference?
		else
		{
			CString	strKey(lpszBegin, lpszEnd - lpszBegin);

			// because some character entity references are 
			// case-sensitive, we must fix them manually
			if (!strKey.CompareNoCase(_T("eth")) ||
				!strKey.CompareNoCase(_T("thorn")))
			{
				if (::_istupper(strKey[0]))
					strKey.MakeUpper();
				else
					strKey.MakeLower();
			}
			else if (!strKey.CompareNoCase(_T("Oslash")))
			{
				strKey.MakeLower();
				strKey.SetAt(0, _T('O'));
			}
			else if (!strKey.CompareNoCase(_T("AElig")))
			{
				strKey.MakeLower();
				strKey.SetAt(0, _T('A'));
				strKey.SetAt(1, _T('E'));
			}
			else
			{
				CString	strT = strKey.Mid(1);
				strKey.MakeLower();
				if (strT.CompareNoCase(_T("grave")) == 0 ||
					strT.CompareNoCase(_T("acute")) == 0 ||
					strT.CompareNoCase(_T("circ")) == 0 ||
					strT.CompareNoCase(_T("uml")) == 0 ||
					strT.CompareNoCase(_T("tilde")) == 0 ||
					strT.CompareNoCase(_T("cedil")) == 0 ||
					strT.CompareNoCase(_T("ring")) == 0)
				{
					strKey.SetAt(0, strT[0]);
				}
			}

			// is this a known entity reference?
			if (m_CharEntityRefs.Lookup(strKey, chTemp))
			{
				chSubst = chTemp;
				lpszEnd = ::_tcsinc(lpszEnd);
				return (lpszEnd - lpszEntity);
			}
		}

		return (0U);
	}

	// Data Members
private:
	static CCharEntityRefs	m_CharEntityRefs;
};
#pragma warning(pop)

#endif	// !__LITEHTMLENTITYRESOLVER_H__
