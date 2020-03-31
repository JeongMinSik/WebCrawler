
// WebCrawler_MinsikDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "WebCrawler_Minsik.h"
#include "WebCrawler_MinsikDlg.h"
#include "afxdialogex.h"

//
#include <set>
#include "LiteHTMLReader.h"
#include "HtmlElementCollection.h"
#include "CrawlingManager.h"
//

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWebCrawlerMinsikDlg 대화 상자

CWebCrawlerMinsikDlg::CWebCrawlerMinsikDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WEBCRAWLER_MINSIK_DIALOG, pParent)
	, m_strProgressOldNewThing(_T("OldNewThing Progress"))
	, m_strProgressHerbsutter(_T("Herbsutter Progress"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWebCrawlerMinsikDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_START_CRAWLING, m_btnStartCrawling);
	DDX_Text(pDX, IDC_STATIC_CRAWLING_PROGRESS, m_strProgressOldNewThing);
	DDX_Text(pDX, IDC_STATIC_CRAWLING_PROGRESS2, m_strProgressHerbsutter);
	DDX_Control(pDX, IDC_LIST_CRAWLING_RESULT, m_listResult);
	DDX_Control(pDX, IDC_STATIC_THUMBNAIL, m_thumbnailSpace);
	DDX_Control(pDX, IDC_PROGRESS_Crawling1, m_ctrlProgressOldNewThing);
	DDX_Control(pDX, IDC_PROGRESS_Crawling2, m_ctrlProgressHerbsutter);
}

BEGIN_MESSAGE_MAP(CWebCrawlerMinsikDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_CRAWLING, &CWebCrawlerMinsikDlg::OnBnClickedButtonStartCrawling)
	ON_MESSAGE(MESSAGE_UPDATE_DATA, OnUpdateData)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CWebCrawlerMinsikDlg::OnBnClickedButtonSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CRAWLING_RESULT, &CWebCrawlerMinsikDlg::OnNMDblclkListCrawlingResult)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CRAWLING_RESULT, &CWebCrawlerMinsikDlg::OnNMClickListCrawlingResult)
END_MESSAGE_MAP()


// CWebCrawlerMinsikDlg 메시지 처리기

BOOL CWebCrawlerMinsikDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// 리스트컨트롤 초기화
	{
		CRect rect;
		m_listResult.GetWindowRect(&rect);
		m_listResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		const int columnWidth = rect.Width() / ArticleColumnType::Count;
		m_listResult.InsertColumn(ArticleColumnType::Title, _T("Title"), LVCFMT_CENTER, static_cast<int>(rect.Width() * 0.60));
		m_listResult.InsertColumn(ArticleColumnType::Date, _T("Date"), LVCFMT_CENTER, static_cast<int>(rect.Width() * 0.125));
		m_listResult.InsertColumn(ArticleColumnType::KeywordCount, _T("KeywordCount"), LVCFMT_CENTER, static_cast<int>(rect.Width() * 0.125));
		m_listResult.InsertColumn(ArticleColumnType::Thumbnail, _T("Thumbnail"), LVCFMT_CENTER, static_cast<int>(rect.Width() * 0.125));

		m_selectedIndex = -1;
	}

	// CrawlingManager 초기화
	CrawlingManager::getInstance()->initialize();

	return TRUE;
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CWebCrawlerMinsikDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	// 썸네일 이미지
	if (false == m_thumbnailPicture.IsNull())
	{
		CRect	rect;
		m_thumbnailSpace.GetClientRect(rect);
		CDC* dc = m_thumbnailSpace.GetWindowDC();
		m_thumbnailPicture.Draw(dc->m_hDC, rect);
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CWebCrawlerMinsikDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 스레드 메시지에 의한 UI갱신
LRESULT CWebCrawlerMinsikDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	SetDlgItemText(IDC_STATIC_CRAWLING_PROGRESS, m_strProgressOldNewThing);
	SetDlgItemText(IDC_STATIC_CRAWLING_PROGRESS2, m_strProgressHerbsutter);
	m_ctrlProgressOldNewThing.UpdateWindow();
	m_ctrlProgressHerbsutter.UpdateWindow();

	return 0;
}

// 크롤링 시작 버튼
void CWebCrawlerMinsikDlg::OnBnClickedButtonStartCrawling()
{
	// UI처리
	{
		m_strProgressOldNewThing.SetString(_T("Oldnewthing 작업 준비..."));
		m_strProgressHerbsutter.SetString(_T("Herbsutter 작업 준비..."));
		m_btnStartCrawling.EnableWindow(false);
		OnUpdateData(NULL, NULL);
	}

	// 워커스레드 생성
	{
		AfxBeginThread(CrawlingManager::CrawlMainPage_OldNewThing, this);
		AfxBeginThread(CrawlingManager::CrawlMainPage_Herbsutter, this);
	}
}

// 키워드 검색 버튼
void CWebCrawlerMinsikDlg::OnBnClickedButtonSearch()
{
	CString searchKeyword;
	GetDlgItemText(IDC_EDIT_KEYWORD, searchKeyword);

	if (true == searchKeyword.IsEmpty())
	{
		AfxMessageBox(_T("검색어를 입력하세요"));
		return;
	}

	searchKeyword.MakeLower(); // 키워드 소문자화(DB에 저장할 때 본문을 모두 소문자로 저장함)

	// DB로드
	set<ArticleInfo> articleInfoList;
	{
		CrawlingManager::getInstance()->loadArticleInfoListFromDB(searchKeyword, articleInfoList);

		if (true == articleInfoList.empty())
		{
			AfxMessageBox(_T("검색결과가 없습니다."));
			return;
		}
	}

	// 리스트 컨트롤에 표시
	{
		m_articleInfoList.clear();
		m_articleInfoList.reserve(articleInfoList.size());
		m_listResult.DeleteAllItems();

		int index = 0;
		for (const auto& info : articleInfoList)
		{
			m_articleInfoList.push_back(info);

			CString keyCount;
			keyCount.Format(_T("%d"), info.m_keywordCount);
			const CString hasThumbnail = (info.m_imgSrc.IsEmpty() ? _T("") : _T("O"));

			m_listResult.InsertItem(index, info.m_title);
			m_listResult.SetItemText(index, ArticleColumnType::Date, info.m_date);
			m_listResult.SetItemText(index, ArticleColumnType::KeywordCount, keyCount);
			m_listResult.SetItemText(index, ArticleColumnType::Thumbnail, hasThumbnail);

			++index;
		}
	}

	// 결과메시지
	{
		CString msg;
		msg.Format(_T("검색어[%s]에 대한 결과 %d건"), searchKeyword, static_cast<int>(m_articleInfoList.size()));
		AfxMessageBox(msg);
	}
}

// 리스트 item 더블클릭 -> 웹브라우저로 url 열기
void CWebCrawlerMinsikDlg::OnNMDblclkListCrawlingResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	NM_LISTVIEW* list = (NM_LISTVIEW*)(pNMHDR);

	if (static_cast<int>(m_articleInfoList.size()) <= list->iItem)
	{
		return;
	}

	const ArticleInfo& info = m_articleInfoList[list->iItem];
	ShellExecute(NULL, _T("open"), _T("iexplore.exe"), info.m_url, _T(""), SW_SHOW);

	*pResult = 0;
}

// 리스트 item 클릭 -> 썸네일 표시
void CWebCrawlerMinsikDlg::OnNMClickListCrawlingResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	NM_LISTVIEW* list = (NM_LISTVIEW*)(pNMHDR);

	if (m_selectedIndex == list->iItem)
	{
		return;
	}
	m_selectedIndex = list->iItem;

	bool isNeedRedraw = false;

	if (false == m_thumbnailPicture.IsNull())
	{
		m_thumbnailPicture.Destroy();
		isNeedRedraw = true;
	}

	if ((0 <= m_selectedIndex) && (m_selectedIndex < static_cast<int>(m_articleInfoList.size())))
	{
		const ArticleInfo& info = m_articleInfoList[m_selectedIndex];
		if (false == info.m_imgSrc.IsEmpty())
		{
			CrawlingManager::getInstance()->makeImageObject(info.m_imgSrc, m_thumbnailPicture);
			isNeedRedraw = true;
		}
	}

	if (true == isNeedRedraw)
	{
		//CRect rect;
		//m_thumbnailSpace.GetClientRect(rect);
		InvalidateRect(NULL);
	}

	*pResult = 0;
}
