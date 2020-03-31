
// WebCrawler_MinsikDlg.h: 헤더 파일
//

#pragma once

#include "HtmlElementCollection.h"

struct ArticleInfo
{
	ArticleInfo() : m_title(L""), m_url(L""), m_date(L""), m_contents(L""), m_imgSrc(L""), m_keywordCount(0) {};
	ArticleInfo(const CString& title, const CString& url, const CString& date, const CString& contents, const CString& imgSrc, int count) 
				: m_title(title), m_url(url), m_date(date), m_contents(contents), m_imgSrc(imgSrc), m_keywordCount(count) {};

	bool isValid(void) const
	{
		if (m_title.IsEmpty()) return false;
		if (m_url.IsEmpty()) return false;
		if (m_date.IsEmpty()) return false;
		if (m_contents.IsEmpty()) return false;
		//if (m_imgSrc.IsEmpty()) return false;
		return true;
	}

	bool operator <(const ArticleInfo& rhs) const
	{
		if (m_keywordCount == rhs.m_keywordCount)
		{
			return rhs.m_date < m_date; // 최신순
		}

		return rhs.m_keywordCount < m_keywordCount; // 내림차순
	}

	CString		m_title;
	CString		m_url;
	CString		m_date;
	CString		m_contents;
	CString		m_imgSrc;
	int			m_keywordCount;
};

enum ArticleColumnType
{
	Title = 0,
	Date,
	KeywordCount,
	Thumbnail,
	Count
};

// CWebCrawlerMinsikDlg 대화 상자
class CWebCrawlerMinsikDlg : public CDialogEx
{
// 생성입니다.
public:
	CWebCrawlerMinsikDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBCRAWLER_MINSIK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL			OnInitDialog();
	afx_msg void			OnPaint();			// 썸네일 그리기에 사용
	afx_msg HCURSOR			OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	#define MESSAGE_UPDATE_DATA WM_USER
	LRESULT					OnUpdateData(WPARAM wParam, LPARAM lParam);						// ui 갱신
	afx_msg void			OnBnClickedButtonStartCrawling();								// 크롤링 시작
	afx_msg void			OnBnClickedButtonSearch();										// 키워드 검색
	afx_msg void			OnNMClickListCrawlingResult(NMHDR *pNMHDR, LRESULT *pResult);	// 리스트 선택: 썸네일보기
	afx_msg void			OnNMDblclkListCrawlingResult(NMHDR *pNMHDR, LRESULT *pResult);	// 리스트 더블클릭: URL 열기

	CButton					m_btnStartCrawling;
	CString					m_strProgressOldNewThing;
	CString					m_strProgressHerbsutter;
	CProgressCtrl			m_ctrlProgressOldNewThing;
	CProgressCtrl			m_ctrlProgressHerbsutter;
	CListCtrl				m_listResult;
	CStatic					m_thumbnailSpace;
	CImage					m_thumbnailPicture;
	int						m_selectedIndex;
	vector<ArticleInfo>		m_articleInfoList;

};
