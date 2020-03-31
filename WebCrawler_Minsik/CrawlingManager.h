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
	CString					m_string;			// ó���ؾ� �� ������Ʈ�� ���
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
	// @remarks::	�� ������Ʈ�� ���������� �� 10���� ����URL�� ���� HTTP��û�� �ؾ� ��. (1page -> approximately 10 requests)
	//				���� �������� THREAD_COUNT��ŭ ���� �� ������ �� 10���� �ڽĽ����带 ���� ó��.

	static const CString	BASE_URL_OLDNEWTHING;
	static const int		THREAD_COUNT = 6;

	static UINT				CrawlMainPage_OldNewThing(LPVOID param);			// param = CWebCrawlerMinsikDlg*	���������� �б�
	static UINT				CrawlPages_OldNewThing(LPVOID param);				// param = ThreadParam*				n���� m���� ������ �б�
	static UINT				CrawlSingleArticle_OldNewThing(LPVOID param);		// param = ChildThreadParam*		������ �ȿ� �ִ� ���� �ϳ� �б�

	CMutex					m_mutexPageCount;
	int						m_childThreadCount[CrawlingManager::THREAD_COUNT];	// ��Ŀ�����庰 ����ִ�(=ó�� ����) �ڽĽ����� ����
	int						m_crawledPageCount;
	int						m_lastCrawlPage;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// @remarks::	�� ������Ʈ�� �� ���������� �������� �� ���� �� �����Ƿ� �̱۽������ ó��. (1page -> 1request)

	static const CString	BASE_URL_HERBSUTTER;

	static UINT				CrawlMainPage_Herbsutter(LPVOID param);												// ���������� �б�
	void					crawlPages_Herbsutter(CWebCrawlerMinsikDlg*	mainDlg, const int lastPage) noexcept;	// 0 ~ lastPage���� �б�
	bool					crawlSingleArticle_Herbsutter(const CString& articleString) noexcept;				// ������ �ȿ� �ִ� ���� �ϳ� �б�

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


};
