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
	// @remarks:: WinInet 진행 순서
	// Open -> openUrl( = Connect -> OpenRequest -> SendRequest) -> ReadFile -> 모든 핸들 CloseHandle()

	if (false == strData.IsEmpty())
	{
		AfxMessageBox(_T("결과 스트링은 비어있어야 합니다. 코드버그입니다."));
		strData.Empty();
	}

	// Internet handle을 초기화(entity이름, 접속유형, 기타속성..)
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
		// 요청이 거절될 수 있다.
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hSession);
		return 0;
	}

	// 위의 InternetOpenUrl 함수로 한방에 처리되므로 아래 세개 함수 모두 주석
	//// 서버 연결
	//HINTERNET hConnect = InternetConnect(hSession,
	//									 _T("devblogs.microsoft.com"),	// URL
	//									 INTERNET_DEFAULT_HTTP_PORT,	// PORT
	//									 _T(""),						// FTP사용자이름(HTTP에서는 "")
	//									 _T(""),						// FTP사용자비번(HTTP에서는 "")
	//									 INTERNET_SERVICE_HTTP,			// 서비스종류
	//									 0,
	//									 0);
	//if (NULL == hConnect)
	//{
	//	InternetCloseHandle(hConnect);
	//  InternetCloseHandle(hSession);
	//	return 0;
	//}
	//
	//// 접속 확인 : 서버에 전달할 요구 정보를 만든다. 실제 서버에 전달되지는 않는다
	//HINTERNET hHttpFile = HttpOpenRequest(hConnect,
	//									  _T("GET"),			//verb (GET, PUT, POST)
	//									  _T("/oldnewthing/"),	// 파일명
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
	//// 만들어진 요구 사항을 서버에 보내 준다.
	//bool isSuccess = HttpSendRequest(hHttpFile, NULL, 0, 0, 0);
	//if (false == isSuccess)
	//{
	//	InternetCloseHandle(hHttpFile);
	//	InternetCloseHandle(hConnect);
	//  InternetCloseHandle(hSession);
	//	return 0;
	//}

	// 파일 전체용량 확인
	DWORD		dwTotalFileSize			= 0;
	DWORD		dwAvaliableFileSize		= 0;
	DWORD		dwDataSize				= sizeof(dwTotalFileSize);
	const bool	hasTotalFileSize		= HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwTotalFileSize, &dwDataSize, NULL);

	if (true == hasTotalFileSize)
	{
		dwAvaliableFileSize = dwTotalFileSize;
	}
	else // 헤더에 파일의 전체크기가 없는 상황
	{
		InternetQueryDataAvailable(hUrl, &dwAvaliableFileSize, 0, 0);
	}

	unsigned char*	readBuffer		= new unsigned char[dwAvaliableFileSize + 1];
	unsigned long	totalReadSize	= 0;

	while (true)
	{
		DWORD dwBytesRead = 0;

		//정보를 읽는다 (핸들, 저장 버퍼, 읽을 크기, 실제 읽은 크기)
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
			// 메모리 해제 후 한번에 읽을 수 있는 양을 구해서 다시 할당한다.
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
		imageDataBase64 = imgSrc; // 못 읽었으면 imgSrc가 url이 아니라 imgSrc 그 자체라고 가정.
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
		AfxMessageBox(_T("StartCrawlingForOldnewthing getDataByUrl 실패"));
		return 1;
	}

	// find lastPage
	int lastPage = 0;
	{
		CLiteHTMLReader theReader;
		CHtmlElementCollection theElementCollectionHandler;
		theReader.setEventHandler(&theElementCollectionHandler);
		theElementCollectionHandler.InitWantedTag(_T("a"), _T("class"), _T("page-link"));// tag, attr, value 로 필터링
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
		msg.Format(_T("Oldnewthing 마지막 페이지가 비정상입니다. [lastPage:%d]"), lastPage);
		AfxMessageBox(msg);
		return 1;
	}

	{
		// 여기까진 싱글스레드라서 m_lastCrawlPage에 대해서 lock이 필요없다.
		CString progress;
		CrawlingManager::getInstance()->m_lastCrawlPage = lastPage;
		progress.Format(_T("Oldnewthing: %d / %d page"), CrawlingManager::getInstance()->m_crawledPageCount, CrawlingManager::getInstance()->m_lastCrawlPage);
		mainDlg->m_strProgressOldNewThing.SetString(progress);
		mainDlg->m_ctrlProgressOldNewThing.SetRange(0, CrawlingManager::getInstance()->m_lastCrawlPage);
		mainDlg->PostMessage(MESSAGE_UPDATE_DATA);
	}

	const int threadCount	= THREAD_COUNT - 1; // 마지막 스레드는 나눈 후 남은 찌꺼기페이지를 관리해야 함
	const int pagePerThread = lastPage / threadCount;

	if (0 < pagePerThread)
	{
		for (int ii = 0; ii < threadCount; ++ii)
		{
			AfxBeginThread(CrawlingManager::CrawlPages_OldNewThing, new ThreadParam(ii, mainDlg, ii*pagePerThread, pagePerThread));
		}
	}

	const int leftPage = (lastPage % threadCount) + 1; // 마지막페이지까지 포함하기 위해 + 1
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

		// 글 리스트 읽기
		CString articleListString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("id"), _T("most-recent"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("Oldnewthing 페이지[%d] 읽기 실패."), page);
				AfxMessageBox(msg);
				continue;
			}

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				CString msg;
				msg.Format(_T("Oldnewthing most-recent로 필터링 된 노드가 1개가 아닙니다. [page:%d, %d개]"), page, theElementCollectionHandler.GetNumElementsFiltered());
				AfxMessageBox(msg);
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, articleListString, true);
		}

		// 개별 글로 분류
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("header"), _T("class"), _T("entry-header"));
			if (0 == theReader.Read(articleListString))
			{
				CString msg;
				msg.Format(_T("Oldnewthing 페이지[%d]의 개별 글 읽기 실패."), page);
				AfxMessageBox(msg);
				continue;
			}

			// 글 하나당 자식스레드 1개씩 할당
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

				// Dirty Read :  모든 미니스레드가 작업을 완료할 때까지 대기
				while (0 != CrawlingManager::getInstance()->m_childThreadCount[currThreadID])
				{
					Sleep(300 * CrawlingManager::getInstance()->m_childThreadCount[currThreadID]); // 아직 살아있는 자식스레드 개수에 비례해서 대기시간 설정
				}
			}
		}

		// 페이지 한 장 완료
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
				progress.Format(_T("oldnewthing 완료! (총 %d page)"), CrawlingManager::getInstance()->m_lastCrawlPage);
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
		articleInfo.m_title.Replace('\'', '\\\'');	// 쿼리문에 '가 들어가면 안되므로
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
	CString originalContentsString; // 이미지를 위해서 비가공데이터 미리 백업
	{
		CString html;
		while (0 == CrawlingManager::getInstance()->getDataByUrl(articleInfo.m_url, false, html))
		{
			Sleep(1);
		}

		// 본문 덩어리 읽기
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("class"), _T("entry-content col-12 sharepostcontent"));
			theReader.Read(html);

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				AfxMessageBox(_T("Oldnewthing 본문 read 실패"));
				return 1; // 본문이 정확히 1개가 아니다?
			}

			theElementCollectionHandler.GetInnerHtml(0, html, true);
		}

		// 본문 내 text 읽기
		{
			CLiteHTMLReader theReader;
			CHtmlElementCollection theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("p"));
			theReader.Read(html);

			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // 제목+본문
			articleInfo.m_contents.MakeLower(); 			// 향후 검색을 위해 소문자화
			articleInfo.m_contents.Replace('\'', '\\\'');	// 쿼리문에 '가 들어가면 안되므로

			// 아래에서 할 imgSrc 작업을 위해서 원본텍스트도 저장
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

		while (true) // 읽혀지는 img가 있을 때까지 계속
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
			imgSrc.Remove(L' '); // imgSrc에 공백이 있으면 제대로 못 읽는다.

			startPos = imgSrc.Find(_T("data:"));
			if (-1 != startPos)
			{
				endPos = imgSrc.Find(L',', startPos);
				imgSrc = imgSrc.Mid(endPos + 1);
			}

			// 실제 로드 가능한 이미지 소스인지 확인
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

	// 부모스레드에게 나의 종료를 알림
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

	// 최대 페이지 찾기
	int maxPage		= 100; 	// 이 사이트는 무한 스크롤링으로 구현되어 있어서 적당한 크기로 이진검색해본다.
	int startPage	= 0;
	int lastPage	= maxPage;

	while (true)
	{
		if (startPage + 1 == lastPage) // 모든 이진검색 완료
		{
			if (lastPage == maxPage) // 범위내에 마지막 페이지에 애초에 없었을 경우, 탐색범위를 2배로 늘린다.
			{
				maxPage		*= 2;
				startPage	= lastPage;
				lastPage	= maxPage;
			}
			else // lastPage가 곧 마지막 페이지
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
			msg.Format(_T("herbsutter getDataByUrl [%d] 읽기 실패."), midPage);
			AfxMessageBox(msg);
			lastPage = midPage;
			continue;
		}

		// 마지막 페이지인지 nav 태그로 확인
		CString navigationString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("nav"), _T("class"), _T("navigation posts-navigation"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("herbsutter navigation [%d] 읽기 실패."), midPage);
				AfxMessageBox(msg);
				lastPage = midPage;
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, navigationString, true);
		}

		if (-1 == navigationString.Find(_T("Older posts"))) // 최대범위 이상의 페이지
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
			msg.Format(_T("herbsutter 페이지[%d] url 읽기 실패."), page);
			AfxMessageBox(msg);
			continue;
		}

		// 글 리스트 읽기
		CString articleListString;
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);
			theElementCollectionHandler.InitWantedTag(_T("main"), _T("id"), _T("main"));

			if (0 == theReader.Read(html))
			{
				CString msg;
				msg.Format(_T("herbsutter 페이지[%d] main 읽기 실패."), page);
				AfxMessageBox(msg);
				continue;
			}

			if (1 != theElementCollectionHandler.GetNumElementsFiltered())
			{
				CString msg;
				msg.Format(_T("site-main로 필터링 된 노드가 1개가 아닙니다. [%d개]"), theElementCollectionHandler.GetNumElementsFiltered());
				AfxMessageBox(msg);
				continue;
			}

			theElementCollectionHandler.GetInnerHtml(0, articleListString, true);
		}

		// 개별 글 하나씩 처리
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theReader.setEventHandler(&theElementCollectionHandler);

			theElementCollectionHandler.InitWantedTag(_T("article"));
			if (0 == theReader.Read(articleListString))
			{
				CString msg;
				msg.Format(_T("herbsutter 페이지[%d]의 개별 글 읽기 실패."), page);
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
	completeString.Format(_T("herbsutter.com 완료! (총 %d page)"), lastPage);
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
			return false; // 헤더가 정확히 1개가 아니다?
		}

		CString header;
		theElementCollectionHandler.GetInnerHtml(0, header, true);

		// url
		{
			startPos			= header.Find(_T("href="""), 0) + 6; // 6은 href="
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
			articleInfo.m_title.Replace('\'', '\\\'');	// 쿼리문에 '가 들어가면 안되므로
		}
	}

	// date
	{
		startPos			= articleString.Find(_T("datetime="""), 0) + 10; // 10은 datetime="
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
			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // 제목 + 본문
		}
		else // p 태그를 안 쓴 경우
		{
			CLiteHTMLReader			theReader;
			CHtmlElementCollection	theElementCollectionHandler;
			theElementCollectionHandler.InitWantedTag(_T("div"), _T("class"), _T("entry-content"));
			theReader.Read(articleString);
			articleInfo.m_contents = articleInfo.m_title + '\n' + theElementCollectionHandler.GetCharacters(); // 제목 + 본문
		}
		articleInfo.m_contents.MakeLower(); 			// 향후 검색을 위해 소문자화
		articleInfo.m_contents.Replace('\'', '\\\'');	// 쿼리문에 '가 들어가면 안되므로
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
			imgSrc.Remove(L' '); //공백제거

			startPos	= imgSrc.Find(_T("data:"));
			if (-1 != startPos)
			{
				endPos	= imgSrc.Find(L',', startPos);
				imgSrc	= imgSrc.Mid(endPos + 1);
			}

			// 실제 로드 가능한 이미지인지 확인
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
