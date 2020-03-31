# WebCrawler


---------------------------------------
## 목차

* [1. 개발환경](#1-개발환경)
* [2. 빌드 및 실행](#2-빌드-및-실행)
* [3. 요구사항](#3-요구사항)
* [4. 개발결과](#4-개발결과)
* [5. 프로젝트 구조](#5-프로젝트-구조)
* [6. 사용된 오픈소스](#6-사용된-오픈소스)


---------------------------------------
### 1. 개발환경

* Windows 10
* VisualStudio2017
* C++
* MFC
* SQLite


---------------------------------------
### 2. 빌드 및 실행

* Debug 또는 Release, **32bit(x86)** 으로 빌드가능
* 실행파일 WebCrawler_Minsik.exe가 폴더 내 포함되어 있음


---------------------------------------
### 3. 요구사항

* 해당 사이트를 검색하는 Win32 Application을 구현 하시오(C++, MFC사용)
  * https://devblogs.microsoft.com/oldnewthing/
  * https://herbsutter.com/
  
* 크롤링
  * 해당 사이트의 블로그 글 리스트
  * 글 본문
  * 본문내 포함된 이미지 1장
  * 작업진행상태 UI에 표시
  * 크롤링한 데이터는 SQLite에 저장
  
* 검색
  * 검색어가 포함된 글 리스트 출력(SQLite에서 검색)
  * 이미지(썸네일)가 있는 경우 표시
  * 선택시 해당 글 url을 웹브라우저로 열기
  * 리스트 정렬기준
    * 1순위. 검색어 포함 개수
    * 2순위. 최신 작성일


---------------------------------------
### 4. 개발결과

<img src="https://user-images.githubusercontent.com/14919359/77992354-dffa5100-7360-11ea-819c-9c4d57eb8dc3.png" width="90%"></img>

* 1\. 크롤링 시작

* 2\. 크롤링 진행상황

* 3\. 키워드 입력 및 검색

* 4\. 이미지(썸네일)

* 5\. 검색결과 리스트
  * item 클릭시 이미지가 있다면 썸네일 표시
  * item 더블클릭시 iexplorer로 해당 url 오픈


---------------------------------------
### 5. 프로젝트 구조

* 크롤링 플로우
<img src="https://user-images.githubusercontent.com/14919359/78001497-b8ab8000-7370-11ea-8b00-bea303783fea.png"></img>

* 클래스 명세
  > * CWebCrawlerMinsikDlg
  >
  >   * OnBnClickedButtonStartCrawling(): 크롤링 시작 버튼
  >   * OnBnClickedButtonSearch(): 키워드 검색 버튼
  >   * OnNMClickListCrawlingResult(): 리스트 내 아이템 클릭(썸네일이 있다면 표시됨)
  >   * OnNMDblclkListCrawlingResult(): 리스트 내 아이템 더블클릭(url 웹브라우저로 열기)

  > * CrawlingManager(SingleTon)
  >
  >   * getDataByUrl(): url에서 데이터 string으로 얻어오기
  >   * makeImageObject(): imgSrc로 CImage 생성하기
  >   * insertArticleInfoToDB(): articleInfo를 DB에 넣기
  >   * loadArticleInfoListFromDB(): DB에서 articleInfoList 얻어오기
  >   *
  >   * CrawlMainPage_OldNewThing(): 메인 웹페이지를 읽고 총 페이지 수 계산
  >   * CrawlPages_OldNewThing(): 하나의 페이지를 읽고 총 글 수 계산
  >   * CrawlSingleArticle_OldNewThing(): 하나의 글을 읽고 DB저장
  >   *
  >   * CrawlMainPage_Herbsutter(): 위와 동일
  >   * crawlPages_Herbsutter(): 위와 동일
  >   * crawlSingleArticle_Herbsutter(): 위와 동일
  
* DB
  > * CREATE TABLE Article(title	TEXT NOT NULL, url TEXT PRIMARY KEY ON CONFLICT REPLACE NOT NULL, date TEXT NOT NULL, contents TEXT NOT NULL, imgSrc	TEXT NOT NULL);
  > * CREATE INDEX contentsIdx ON Article(contents);
  > * SELECT title, url, date, contents, imgSrc FROM Article WHERE contents LIKE '%검색어%';

  
---------------------------------------
### 6. 사용된 오픈소스

* HTML Reader: https://www.codeproject.com/Articles/6561/HTML-Reader-C-Class-Library
* cpp Base64: https://github.com/ReneNyffenegger/cpp-base64
* SQLite: https://www.sqlite.org/index.html
* CppSQLite: https://www.codeproject.com/Articles/6343/CppSQLite-C-Wrapper-for-SQLite
