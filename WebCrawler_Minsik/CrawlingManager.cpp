#include "pch.h"
#include "CrawlingManager.h"
#include <WinInet.h>
#include "base64.h"
#include "CppSQLite3U.h"

bool				CrawlingManager::hasInstance			= false;
CrawlingManager*	CrawlingManager::instance				= NULL;
const CString		CrawlingManager::DB_NAME				= _T("WebCrawlerMinsikDB.db");
const CString		CrawlingManager::BASE_URL_OLDNEWTHING	= _T("https://devblogs.microsoft.com/oldnewthing/");
const CString		CrawlingManager::BASE_URL_HERBSUTTER	= _T("https://herbsutter.com/");

CrawlingManager* CrawlingManager::getInstance() 
{
	if (nullptr == instance) 
	{
		instance	= new CrawlingManager();
		hasInstance = true;
	}
	return instance;
}

void CrawlingManager::initialize(void)
{
	m_crawledPageCount	= 0;
	m_lastCrawlPage		= -1;

	for (int ii = 0; ii < THREAD_COUNT; ++ii)
	{
		m_childThreadCount[ii] = 0;
	}

	initializeDB();
}

void CrawlingManager::initializeDB(void)
{
	m_mutexDB.Lock();

	CppSQLite3DB sdb;

	try
	{
		sdb.open(CrawlingManager::DB_NAME);

		sdb.execDML(_T("CREATE TABLE IF NOT EXISTS Article(title	TEXT NOT NULL,									\
														   url		TEXT PRIMARY KEY ON CONFLICT REPLACE NOT NULL,	\
														   date		TEXT NOT NULL,									\
														   contents TEXT NOT NULL,									\
														   imgSrc	TEXT NOT NULL);"));

		sdb.execDML(_T("CREATE INDEX IF NOT EXISTS contentsIdx ON Article(contents);"));
	}
	catch (CppSQLite3Exception& e)
	{
		AfxMessageBox(e.errorMessage());
	}

	sdb.close();
	m_mutexDB.Unlock();
}

DWORD CrawlingManager::getDataByUrl(const LPCWSTR& url, const bool isEncodingBase64, /*_Out_*/CString& strData) const noexcept
{
	// @remarks:: WinInet ���� ����
	// Open -> openUrl( = Connect -> OpenRequest -> SendRequest) -> ReadFile -> ��� �ڵ� CloseHandle()

	if (false == strData.IsEmpty())
	{
		AfxMessageBox(_T("��� ��Ʈ���� ����־�� �մϴ�. �ڵ�����Դϴ�."));
		strData.Empty();
	}

	// Internet handle�� �ʱ�ȭ(entity�̸�, ��������, ��Ÿ�Ӽ�..)
	HINTERNET hSession = InternetOpen(_T("WebCrawler_Minsik"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (NULL == hSession)
	{
		CString msg;
		msg.Format(_T("InternetOpen Error: %u"), GetLastError());
		AfxMessageBox(msg);
		return 0;
	}

	// URL Open ( = Connect -> OpenRequest -> SendRequest)
	HINTERNET hUrl = InternetOpenUrl(hSession, url, NULL, 0, 0, 0);
	if (NULL == hUrl)
	{
		// ��û�� ������ �� �ִ�.
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hSession);
		return 0;
	}

	// ���� InternetOpenUrl �Լ��� �ѹ濡 ó���ǹǷ� �Ʒ� ���� �Լ� ��� �ּ�
	//// ���� ����
	//HINTERNET hConnect = InternetConnect(hSession,
	//									 _T("devblogs.microsoft.com"),	// URL
	//									 INTERNET_DEFAULT_HTTP_PORT,	// PORT
	//									 _T(""),						// FTP������̸�(HTTP������ "")
	//									 _T(""),						// FTP����ں��(HTTP������ "")
	//									 INTERNET_SERVICE_HTTP,			// ��������
	//									 0,
	//									 0);
	//if (NULL == hConnect)
	//{
	//	InternetCloseHandle(hConnect);
	//  InternetCloseHandle(hSession);
	//	return 0;
	//}
	//
	//// ���� Ȯ�� : ������ ������ �䱸 ������ �����. ���� ������ ���޵����� �ʴ´�
	//HINTERNET hHttpFile = HttpOpenRequest(hConnect,
	//									  _T("GET"),			//verb (GET, PUT, POST)
	//									  _T("/oldnewthing/"),	// ���ϸ�
	//									  HTTP_VERSION,
	//									  NULL,
	//									  NULL,
	//									  0,
	//									  0);
	//	if (NULL == hHttpFile)
	//{
	//	InternetCloseHandle(hHttpFile);
	//	InternetCloseHandle(hConnect);
	//  InternetCloseHandle(hSession);
	//	return 0;
	//}
	//
	//// ������� �䱸 ������ ������ ���� �ش�.
	//bool isSuccess = HttpSendRequest(hHttpFile, NULL, 0, 0, 0);
	//if (false == isSuccess)
	//{
	//	InternetCloseHandle(hHttpFile);
	//	InternetCloseHandle(hConnect);
	//  InternetCloseHandle(hSession);
	//	return 0;
	//}

	// ���� ��ü�뷮 Ȯ��
	DWORD		dwTotalFileSize			= 0;
	DWORD		dwAvaliableFileSize		= 0;
	DWORD		dwDataSize				= sizeof(dwTotalFileSize);
	const bool	hasTotalFileSize		= HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwTotalFileSize, &dwDataSize, NULL);

	if (true == hasTotalFileSize)
	{
		dwAvaliableFileSize = dwTotalFileSize;
	}
	else // ����� ������ ��üũ�Ⱑ ���� ��Ȳ
	{
		InternetQueryDataAvailable(hUrl, &dwAvaliableFileSize, 0, 0);
	}

	unsigned char*	readBuffer		= new unsigned char[dwAvaliableFileSize + 1];
	unsigned long	totalReadSize	= 0;

	while (true)
	{
		DWORD dwBytesRead = 0;

		//������ �д´� (�ڵ�, ���� ����, ���� ũ��, ���� ���� ũ��)
		const bool isRead = InternetReadFile(hUrl, readBuffer, dwAvaliableFileSize, &dwBytesRead);

		if (0 == dwBytesRead)
		{
			break;
		}

		if (false == isRead)
		{
			CString msg;
			msg.Format(_T("InternetReadFile Error: %u"), GetLastError());
			AfxMessageBox(msg);
			break;
		}

		totalReadSize += dwBytesRead;
		readBuffer[dwBytesRead] = 0;

		if (true == isEncodingBase64)
		{
			strData += CString(base64_encode(readBuffer, dwBytesRead).c_str());
		}
		else
		{
			strData += CString(readBuffer);
		}

		if (false == hasTotalFileSize)
		{
			// �޸� ���� �� �ѹ��� ���� �� �ִ� ���� ���ؼ� �ٽ� �Ҵ��Ѵ�.
			delete[] readBuffer;
			InternetQueryDataAvailable(hUrl, &dwAvaliableFileSize, 0, 0);
			readBuffer = new unsigned char[dwAvaliableFileSize + 1];
		}
	}

	delete[] readBuffer;

	InternetCloseHandle(hUrl);
	//InternetCloseHandle(hHttpFile);
	//InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	return totalReadSize;
}

bool CrawlingManager::makeImageObject(const CString& imgSrc, /*_Out_*/ CImage& imageObject)  const noexcept
{
	CString imageDataBase64;
	if (0 == getDataByUrl(imgSrc, true, imageDataBase64))
	{
		imageDataBase64 = imgSrc; // �� �о����� imgSrc�� url�� �ƴ϶� imgSrc �� ��ü��� ����.
	}

	const string	imgBuffer	= base64_decode(string(CT2CA(imageDataBase64)));
	const int		totalSize	= imgBuffer.size();
	HGLOBAL			hGlobal		= GlobalAlloc(GMEM_MOVEABLE, totalSize);

	if (NULL != hGlobal)
	{
		char* data = (char*)GlobalLock(hGlobal);
		memcpy(data, imgBuffer.c_str(), totalSize);
		GlobalUnlock(hGlobal);

		LPSTREAM	stream = NULL;
		HRESULT		result = CreateStreamOnHGlobal(hGlobal, TRUE, &stream);

		if (SUCCEEDED(result) && stream)
		{
			result = imageObject.Load(stream);
			stream->Release();

			if (SUCCEEDED(result) && false == imageObject.IsNull())
			{
				return true;
			}
		}
	}

	return false;
}

bool CrawlingManager::insertArticleInfoToDB(const ArticleInfo& articleInfo)
{
	m_mutexDB.Lock();

	CppSQLite3DB	sdb;
	CString			query;
	try
	{
		sdb.open(CrawlingManager::DB_NAME);
		
		query.Format(_T("insert into Article (title, url, date, contents, imgSrc) values ('%s', '%s', '%s', '%s', '%s');")
						, articleInfo.m_title, articleInfo.m_url, articleInfo.m_date, articleInfo.m_contents, articleInfo.m_imgSrc);
		sdb.execQuery(query);
	}
	catch (CppSQLite3Exception& e)
	{
		AfxMessageBox(e.errorMessage() + query);
		sdb.close();
		m_mutexDB.Unlock();
		return false;
	}

	sdb.close();
	m_mutexDB.Unlock();
	return true;
}

void CrawlingManager::loadArticleInfoListFromDB(const CString& keyword, /*_Out_*/ set<ArticleInfo>& articleInfoList)
{
	m_mutexDB.Lock();

	CppSQLite3DB sdb;

	try
	{
		sdb.open(CrawlingManager::DB_NAME);

		CString query;
		query.Format(_T("select title, url, date, contents, imgSrc from Article where contents like '%%%s%%';"), keyword.GetString());
		CppSQLite3Query q = sdb.execQuery(query);

		while (false == q.eof())
		{
			const CString&	contents		= q.getStringField(3);
			int				keywordCount	= 0;
			int				startPos		= 0;
			while (true)
			{
				startPos = contents.Find(keyword, startPos);
				if (-1 == startPos)
				{
					break;
				}
				++startPos;
				++keywordCount;
			};

			articleInfoList.emplace(q.getStringField(0), q.getStringField(1), q.getStringField(2), q.getStringField(3), q.getStringField(4), keywordCount);

			q.nextRow();
		}
	}
	catch (CppSQLite3Exception& e)
	{
		AfxMessageBox(e.errorMessage());
	}

	sdb.close();
	m_mutexDB.Unlock();
}

UINT CrawlingManager::CrawlMainPage_OldNewThing(LPVOID param)
{
	CWebCrawlerMinsikDlg* mainDlg = static_cast<CWebCrawlerMinsikDlg*>(param);

	CString html;
	if (0 == CrawlingManager::getInstance()->getDataByUrl(CrawlingManager::BASE_URL_OLDNEWTHING, false, html))
	{
		AfxMessageBox(_T("StartCrawlingForOldnewthing getDataByUrl ����"));
		return 1;
	}

	// find lastPage
	int lastPage = 0;
	{
		CLiteHTMLReader theReader;
		CHtmlElementCollection theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theElementCollectionHandler.InitWantedTag(_T("a"), _T("class"), _T("page-link"));// tag, attr, value �� ���͸�
		theReader.Read(html);

		for (int ii = 0; ii < theElementCollectionHandler.GetNumElementsFiltered(); ++ii)
		{
			CString pageData;
			theElementCollectionHandler.GetInnerHtml(ii, pageData, true);
			const int startPos = pageData.ReverseFind('>');
			const int page = _ttoi(pageData.Right(pageData.GetLength() - startPos - 1));

			if (lastPage < page)
			{
				lastPage = page;
			}
		}
	}

	if (lastPage <= 0)
	{
		CString msg;
		msg.Format(_T("Oldnewthing ������ �������� �������Դϴ�. [lastPage:%d]"), lastPage);
		AfxMessageBox(msg);
		return 1;
	}

	{
		// ������� �̱۽������ m_lastCrawlPage�� ���ؼ� lock�� �ʿ����.
		CString progress;
		CrawlingManager::getInstance()->m_lastCrawlPage = lastPage;
		progress.Format(_T("Oldnewthing: %d / %d page"), CrawlingManager::getInstance()->m_crawledPageCount, CrawlingManager::getInstance()->m_lastCrawlPage);
		mainDlg->m_strProgressOldNewThing.SetString(progress);
		mainDlg->m_ctrlProgressOldNewThing.SetRange(0, CrawlingManager::getInstance()->m_lastCrawlPage);
		mainDlg->PostMessage(MESSAGE_UPDATE_DATA);
	}

	const int threadCount	= THREAD_COUNT - 1; // ������ ������� ���� �� ���� ����������� �����ؾ� ��
	const int pagePerThread = lastPage / threadCount;

	if (0 < pagePerThread)
	{
		for (int ii = 0; ii < threadCount; ++ii)
		{
			AfxBeginThread(CrawlingManager::CrawlPages_OldNewThing, new ThreadParam(ii, mainDlg, ii*pagePerThread, pagePerThread));
		}
	}

	const int leftPage = (lastPage % threadCount) + 1; // ���������������� �����ϱ� ���� + 1
	AfxBeginThread(CrawlingManager::CrawlPages_OldNewThing, new ThreadParam(threadCount, mainDlg, pagePerThread*threadCount, leftPage));

	return 0;
}

UINT CrawlingManager::CrawlPages_OldNewThing(LPVOID param)
{
	const ThreadParam*		threadParam = static_cast<ThreadParam*>(param);
	CWebCrawlerMinsikDlg*	mainDlg		= threadParam->m_mainDlg;

	for (int ii = 0; ii < threadParam->m_pageCount; ++ii)
	{
		const int	page = threadParam->m_startPage + ii;
		CString		html;
		CString		pageNum;
		pageNum.Format(_T("page/%d"), page);

		while (0 == CrawlingManager::getInstance()->getDataByUrl(CrawlingManager::BASE_URL_OLDNEWTHING + pageNum, false, html))
		{
			Sleep(1);
		}

		// �� ����Ʈ �б�
		CString articleListString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("id"), _T("most-recent"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("Oldnewthing ������[%d] �б� ����."), page);
				AfxMessageBox(msg);
				continue;
			}

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				CString msg;
				msg.Format(_T("Oldnewthing most-recent�� ���͸� �� ��尡 1���� �ƴմϴ�. [page:%d, %d��]"), page, theElementCollectionHandler.GetNumElementsFiltered());
				AfxMessageBox(msg);
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, articleListString, true);
		}

		// ���� �۷� �з�
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("header"), _T("class"), _T("entry-header"));
			if (0 == theReader.Read(articleListString))
			{
				CString msg;
				msg.Format(_T("Oldnewthing ������[%d]�� ���� �� �б� ����."), page);
				AfxMessageBox(msg);
				continue;
			}

			// �� �ϳ��� �ڽĽ����� 1���� �Ҵ�
			{
				const int currThreadID = threadParam->m_threadID;

				for (int ii = 0; ii < theElementCollectionHandler.GetNumElementsFiltered(); ++ii)
				{
					CString articleString;
					theElementCollectionHandler.GetInnerHtml(ii, articleString, true);
					AfxBeginThread(CrawlingManager::CrawlSingleArticle_OldNewThing, new ChildThreadParam(currThreadID, articleString));
				}

				CrawlingManager::getInstance()->m_mutexPageCount.Lock();
				CrawlingManager::getInstance()->m_childThreadCount[currThreadID] += theElementCollectionHandler.GetNumElementsFiltered();
				CrawlingManager::getInstance()->m_mutexPageCount.Unlock();

				// Dirty Read :  ��� �̴Ͻ����尡 �۾��� �Ϸ��� ������ ���
				while (0 != CrawlingManager::getInstance()->m_childThreadCount[currThreadID])
				{
					Sleep(300 * CrawlingManager::getInstance()->m_childThreadCount[currThreadID]); // ���� ����ִ� �ڽĽ����� ������ ����ؼ� ���ð� ����
				}
			}
		}

		// ������ �� �� �Ϸ�
		{
			CrawlingManager::getInstance()->m_mutexPageCount.Lock();

			CString progress;
			++(CrawlingManager::getInstance()->m_crawledPageCount);
			if (CrawlingManager::getInstance()->m_crawledPageCount < CrawlingManager::getInstance()->m_lastCrawlPage)
			{
				progress.Format(_T("Oldnewthing: %d / %d page"), CrawlingManager::getInstance()->m_crawledPageCount, CrawlingManager::getInstance()->m_lastCrawlPage);
			}
			else
			{
				progress.Format(_T("oldnewthing �Ϸ�! (�� %d page)"), CrawlingManager::getInstance()->m_lastCrawlPage);
			}
			mainDlg->m_strProgressOldNewThing.SetString(progress);
			mainDlg->m_ctrlProgressOldNewThing.SetPos(CrawlingManager::getInstance()->m_crawledPageCount);
			mainDlg->PostMessage(MESSAGE_UPDATE_DATA);

			CrawlingManager::getInstance()->m_mutexPageCount.Unlock();
		}
	}

	delete threadParam;

	return 0;
}

UINT CrawlingManager::CrawlSingleArticle_OldNewThing(LPVOID param)
{
	const ChildThreadParam*	childeThreadParam	= static_cast<const ChildThreadParam*>(param);
	const CString&			articleString		= childeThreadParam->m_string;

	ArticleInfo	 articleInfo;
	int			 startPos		= 0;
	int			 endPos			= 0;

	// url
	{
		startPos			= articleString.Find(_T("href="), 0) + 6;
		endPos				= articleString.Find('"', startPos);
		articleInfo.m_url	= articleString.Mid(startPos, endPos - startPos);
	}

	// title
	{
		CString title;
		startPos	= articleString.Find('>', endPos) + 1;
		endPos		= articleString.Find(_T("</a>"), startPos);
		title		= articleString.Mid(startPos, endPos - startPos);

		CLiteHTMLReader theReader;
		CHtmlElementCollection theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theReader.Read(title);

		articleInfo.m_title = theElementCollectionHandler.GetCharacters();
		articleInfo.m_title.Replace('\'', '\\\'');	// �������� '�� ���� �ȵǹǷ�
	}

	// date
	{
		startPos	= articleString.Find(_T("date-mini"), endPos) + 11;
		endPos		= articleString.Find('<', startPos);
		const CString&		date			= articleString.Mid(startPos, endPos - startPos);
		const int			firstDelim		= date.Find(L'/');
		const int			secondDelim		= date.Find(L'/', firstDelim + 1);
		const CString&		month			= date.Left(firstDelim);
		const CString&		day				= date.Mid(firstDelim + 1, secondDelim - firstDelim - 1);
		const CString&		year			= _T("20") + date.Mid(secondDelim + 1);
		articleInfo.m_date	= year + _T('-') + month + _T('-') + day;
	}

	// contents
	CString originalContentsString; // �̹����� ���ؼ� �񰡰������� �̸� ���
	{
		CString html;
		while (0 == CrawlingManager::getInstance()->getDataByUrl(articleInfo.m_url, false, html))
		{
			Sleep(1);
		}

		// ���� ��� �б�
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("class"), _T("entry-content col-12 sharepostcontent"));
			theReader.Read(html);

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				AfxMessageBox(_T("Oldnewthing ���� read ����"));
				return 1; // ������ ��Ȯ�� 1���� �ƴϴ�?
			}

			theElementCollectionHandler.GetInnerHtml(0, html, true);
		}

		// ���� �� text �б�
		{
			CLiteHTMLReader theReader;
			CHtmlElementCollection theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("p"));
			theReader.Read(html);

			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // ����+����
			articleInfo.m_contents.MakeLower(); 			// ���� �˻��� ���� �ҹ���ȭ
			articleInfo.m_contents.Replace('\'', '\\\'');	// �������� '�� ���� �ȵǹǷ�

			// �Ʒ����� �� imgSrc �۾��� ���ؼ� �����ؽ�Ʈ�� ����
			for (int ii = 0; ii < theElementCollectionHandler.GetNumElementsFiltered(); ++ii)
			{
				CString contentsText;
				theElementCollectionHandler.GetInnerHtml(ii, contentsText, true);
				originalContentsString += contentsText;
			}
		}
	}

	// imgSrc
	{
		CString lowerString = originalContentsString;
		lowerString.MakeLower();
		int serchStartPos = 0;

		while (true) // �������� img�� ���� ������ ���
		{
			startPos = lowerString.Find(_T("<img"), serchStartPos);
			if (-1 == startPos)
			{
				break;
			}

			startPos = lowerString.Find(_T("src="""), startPos);
			if (-1 == startPos)
			{
				break;
			}

			startPos += 5; // src="
			endPos = originalContentsString.Find('"', startPos);
			CString imgSrc = originalContentsString.Mid(startPos, endPos - startPos);
			imgSrc.Remove(L' '); // imgSrc�� ������ ������ ����� �� �д´�.

			startPos = imgSrc.Find(_T("data:"));
			if (-1 != startPos)
			{
				endPos = imgSrc.Find(L',', startPos);
				imgSrc = imgSrc.Mid(endPos + 1);
			}

			// ���� �ε� ������ �̹��� �ҽ����� Ȯ��
			if (true == CrawlingManager::getInstance()->makeImageObject(imgSrc, CImage()))
			{
				articleInfo.m_imgSrc = imgSrc;
				break;
			}

			serchStartPos += endPos;
		}
	}

	if (articleInfo.isValid())
	{
		CrawlingManager::getInstance()->insertArticleInfoToDB(articleInfo);
	}
	else
	{
		CString msg;
		msg.Format(_T("Oldnewthing ArticleInfo invalid [url:%s]"), articleInfo.m_url);
		AfxMessageBox(msg);
	}

	// �θ𽺷��忡�� ���� ���Ḧ �˸�
	{
		CrawlingManager::getInstance()->m_mutexPageCount.Lock();
		--(CrawlingManager::getInstance()->m_childThreadCount[childeThreadParam->m_parentThreadID]);
		CrawlingManager::getInstance()->m_mutexPageCount.Unlock();
	}

	delete childeThreadParam;

	return 0;
}

UINT CrawlingManager::CrawlMainPage_Herbsutter(LPVOID param)
{
	CWebCrawlerMinsikDlg* mainDlg = static_cast<CWebCrawlerMinsikDlg*>(param);

	// �ִ� ������ ã��
	int maxPage		= 100; 	// �� ����Ʈ�� ���� ��ũ�Ѹ����� �����Ǿ� �־ ������ ũ��� �����˻��غ���.
	int startPage	= 0;
	int lastPage	= maxPage;

	while (true)
	{
		if (startPage + 1 == lastPage) // ��� �����˻� �Ϸ�
		{
			if (lastPage == maxPage) // �������� ������ �������� ���ʿ� ������ ���, Ž�������� 2��� �ø���.
			{
				maxPage		*= 2;
				startPage	= lastPage;
				lastPage	= maxPage;
			}
			else // lastPage�� �� ������ ������
			{
				CString progress;
				progress.Format(_T("Herbsutter: 0 / %d page"), lastPage);
				mainDlg->m_strProgressHerbsutter.SetString(progress);
				mainDlg->m_ctrlProgressHerbsutter.SetRange(0, lastPage);
				mainDlg->PostMessage(MESSAGE_UPDATE_DATA);
				break;
			}
		}

		const int midPage = ((startPage + lastPage) / 2);

		CString		html;
		CString		pageNum;
		pageNum.Format(_T("page/%d"), midPage);
		if (0 == CrawlingManager::getInstance()->getDataByUrl(CrawlingManager::BASE_URL_HERBSUTTER + pageNum, false, html))
		{
			CString msg;
			msg.Format(_T("herbsutter getDataByUrl [%d] �б� ����."), midPage);
			AfxMessageBox(msg);
			lastPage = midPage;
			continue;
		}

		// ������ ���������� nav �±׷� Ȯ��
		CString navigationString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("nav"), _T("class"), _T("navigation posts-navigation"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("herbsutter navigation [%d] �б� ����."), midPage);
				AfxMessageBox(msg);
				lastPage = midPage;
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, navigationString, true);
		}

		if (-1 == navigationString.Find(_T("Older posts"))) // �ִ���� �̻��� ������
		{
			lastPage = midPage;
		}
		else
		{
			startPage = midPage;
		}
	}

	CrawlingManager::getInstance()->crawlPages_Herbsutter(mainDlg, lastPage);

	return 0;
}

void CrawlingManager::crawlPages_Herbsutter(CWebCrawlerMinsikDlg* mainDlg, const int lastPage) noexcept
{
	for (int page = 0; page <= lastPage; ++page)
	{
		CString html;
		CString pageNum;
		pageNum.Format(_T("page/%d"), page);

		if (0 == CrawlingManager::getInstance()->getDataByUrl(CrawlingManager::BASE_URL_HERBSUTTER + pageNum, false, html))
		{
			CString msg;
			msg.Format(_T("herbsutter ������[%d] url �б� ����."), page);
			AfxMessageBox(msg);
			continue;
		}

		// �� ����Ʈ �б�
		CString articleListString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("main"), _T("id"), _T("main"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("herbsutter ������[%d] main �б� ����."), page);
				AfxMessageBox(msg);
				continue;
			}

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				CString msg;
				msg.Format(_T("site-main�� ���͸� �� ��尡 1���� �ƴմϴ�. [%d��]"), theElementCollectionHandler.GetNumElementsFiltered());
				AfxMessageBox(msg);
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, articleListString, true);
		}

		// ���� �� �ϳ��� ó��
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);

			theElementCollectionHandler.InitWantedTag(_T("article"));
			if (0 == theReader.Read(articleListString))
			{
				CString msg;
				msg.Format(_T("herbsutter ������[%d]�� ���� �� �б� ����."), page);
				AfxMessageBox(msg);
				continue;
			}

			for (int ii = 0; ii < theElementCollectionHandler.GetNumElementsFiltered(); ++ii)
			{
				CString articleString;
				theElementCollectionHandler.GetInnerHtml(ii, articleString, true);
				if (false == CrawlingManager::getInstance()->crawlSingleArticle_Herbsutter(articleString))
				{
					CString msg;
					msg.Format(_T("InsertArticleInfoForHerbsutter error [page:%d, article: %d]"), page, ii);
					AfxMessageBox(msg);
					continue;
				}
			}
		}

		CString progress;
		progress.Format(_T("Herbsutter: %d / %d page"), page, lastPage);
		mainDlg->m_strProgressHerbsutter.SetString(progress);
		mainDlg->m_ctrlProgressHerbsutter.SetPos(page);
		mainDlg->PostMessage(MESSAGE_UPDATE_DATA);
	}

	CString completeString;
	completeString.Format(_T("herbsutter.com �Ϸ�! (�� %d page)"), lastPage);
	mainDlg->m_strProgressHerbsutter.SetString(completeString);
	mainDlg->PostMessage(MESSAGE_UPDATE_DATA);
}

bool CrawlingManager::crawlSingleArticle_Herbsutter(const CString& articleString) noexcept
{
	ArticleInfo		articleInfo;
	int				startPos	= 0;
	int				endPos		= 0;

	// header (title + url)
	{
		CLiteHTMLReader			theReader;
		CHtmlElementCollection	theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theElementCollectionHandler.InitWantedTag(_T("header"));
		theReader.Read(articleString);

		if (1 != theElementCollectionHandler.GetNumElementsFiltered())
		{
			return false; // ����� ��Ȯ�� 1���� �ƴϴ�?
		}

		CString header;
		theElementCollectionHandler.GetInnerHtml(0, header, true);

		// url
		{
			startPos			= header.Find(_T("href="""), 0) + 6; // 6�� href="
			endPos				= header.Find('"', startPos);
			articleInfo.m_url	= header.Mid(startPos, endPos - startPos);
		}

		// title
		{
			CString		title;
			startPos	= header.Find(_T('>'), startPos) + 1;
			endPos		= header.Find(_T("/a>"), startPos) - 1;
			title		= header.Mid(startPos, endPos - startPos);

			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theReader.Read(title);

			articleInfo.m_title	= theElementCollectionHandler.GetCharacters();
			articleInfo.m_title.Replace('\'', '\\\'');	// �������� '�� ���� �ȵǹǷ�
		}
	}

	// date
	{
		startPos			= articleString.Find(_T("datetime="""), 0) + 10; // 10�� datetime="
		endPos				= articleString.Find('"', startPos);
		articleInfo.m_date	= articleString.Mid(startPos, endPos - startPos);
	}

	// contents
	{
		CLiteHTMLReader			theReader;
		CHtmlElementCollection	theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theElementCollectionHandler.InitWantedTag(_T("p"));
		theReader.Read(articleString);

		if (0 < theElementCollectionHandler.GetNumElementsFiltered())
		{
			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // ���� + ����
		}
		else // p �±׸� �� �� ���
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("class"), _T("entry-content"));
			theReader.Read(articleString);
			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // ���� + ����
		}
		articleInfo.m_contents.MakeLower(); 			// ���� �˻��� ���� �ҹ���ȭ
		articleInfo.m_contents.Replace('\'', '\\\'');	// �������� '�� ���� �ȵǹǷ�
	}

	// imgSrc
	{
		CLiteHTMLReader			theReader;
		CHtmlElementCollection	theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theElementCollectionHandler.InitWantedTag(_T("img"));
		theReader.Read(articleString);

		for (int ii = 0; ii < theElementCollectionHandler.GetNumElementsFiltered(); ++ii)
		{
			CString imgSrc;
			theElementCollectionHandler.GetOuterHtml(ii, imgSrc, true);
			startPos = imgSrc.Find(_T("src="""), 0);
			if (-1 == startPos)
			{
				continue;
			}

			startPos	+= 5; // src="
			endPos		= imgSrc.Find('"', startPos);
			imgSrc		= imgSrc.Mid(startPos, endPos - startPos);
			imgSrc.Remove(L' '); //��������

			startPos	= imgSrc.Find(_T("data:"));
			if (-1 != startPos)
			{
				endPos	= imgSrc.Find(L',', startPos);
				imgSrc	= imgSrc.Mid(endPos + 1);
			}

			// ���� �ε� ������ �̹������� Ȯ��
			if (true == makeImageObject(imgSrc, CImage()))
			{
				articleInfo.m_imgSrc = imgSrc;
				break;
			}
		}
	}

	if (articleInfo.isValid())
	{
		return insertArticleInfoToDB(articleInfo);
	}
	else
	{
		return false;
	}
}
