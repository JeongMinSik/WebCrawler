#pragma once

#include "WebCrawler_MinsikDlg.h"
#include <set>

struct ThreadParam
{
	ThreadParam(const int threadID, CWebCrawlerMinsikDlg* dlg, const int startPage, const int pageCount)
		: m_threadID(threadID), m_mainDlg(dlg), m_startPage(startPage), m_pageCount(pageCount) {};
	CWebCrawlerMinsikDlg*	m_mainDlg;
	int						m_threadID;
	int						m_startPage;
	int						m_pageCount;
};

struct ChildThreadParam
{
	ChildThreadParam(const int threadID, CString& string) : m_parentThreadID(threadID), m_string(string) {};
	int						m_parentThreadID;
	CString					m_string;			// 처리해야 할 본문스트링 덩어리
};

class CrawlingManager 
{
private:
	CrawlingManager() { initialize();  };
	static bool				hasInstance;
	static CrawlingManager* instance;

public:
	static CrawlingManager* getInstance();
	virtual ~CrawlingManager() { hasInstance = false; };

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const CString	DB_NAME;
	CMutex					m_mutexDB;

	void					initialize(void);
	void					initializeDB(void);
	
	DWORD					getDataByUrl(const LPCWSTR& url, const bool isEncodingBase64, /*_Out_*/ CString& strData) const noexcept;
	bool					makeImageObject(const CString& imgSrc, /*_Out_*/ CImage& imageObject) const noexcept;
	
	bool					insertArticleInfoToDB(const ArticleInfo& articleInfo);
	void					loadArticleInfoListFromDB(const CString& keyword, /*_Out_*/ set<ArticleInfo>& articleInfoList);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// @remarks::	이 웹사이트는 페이지마다 약 10개의 본문URL에 대해 HTTP요청을 해야 함. (1page -> approximately 10 requests)
	//				따라서 페이지를 THREAD_COUNT만큼 나눈 후 페이지 당 10개의 자식스레드를 만들어서 처리.

	static const CString	BASE_URL_OLDNEWTHING;
	static const int		THREAD_COUNT = 6;

	static UINT				CrawlMainPage_OldNewThing(LPVOID param);			// param = CWebCrawlerMinsikDlg*	메인페이지 읽기
	static UINT				CrawlPages_OldNewThing(LPVOID param);				// param = ThreadParam*				n부터 m까지 페이지 읽기
	static UINT				CrawlSingleArticle_OldNewThing(LPVOID param);		// param = ChildThreadParam*		페이지 안에 있는 본문 하나 읽기

	CMutex					m_mutexPageCount;
	int						m_childThreadCount[CrawlingManager::THREAD_COUNT];	// 워커스레드별 살아있는(=처리 중인) 자식스레드 개수
	int						m_crawledPageCount;
	int						m_lastCrawlPage;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// @remarks::	이 웹사이트는 한 페이지에서 본문까지 다 읽을 수 있으므로 싱글스레드로 처리. (1page -> 1request)

	static const CString	BASE_URL_HERBSUTTER;

	static UINT				CrawlMainPage_Herbsutter(LPVOID param);												// 메인페이지 읽기
	void					crawlPages_Herbsutter(CWebCrawlerMinsikDlg*	mainDlg, const int lastPage) noexcept;	// 0 ~ lastPage까지 읽기
	bool					crawlSingleArticle_Herbsutter(const CString& articleString) noexcept;				// 페이지 안에 있는 본문 하나 읽기

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


};
