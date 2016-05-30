
// TINConstruction2View.cpp : CTINConstruction2View 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "TINConstruction2.h"
#endif

#include "TINConstruction2Doc.h"
#include "TINConstruction2View.h"

#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_api.h>
#include <ogr_core.h>
#include <ogrsf_frmts.h>


#include <vector>
#include <queue>
#include <map>
#include <iostream>
using namespace std;

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


// CTINConstruction2View

IMPLEMENT_DYNCREATE(CTINConstruction2View, CView)

BEGIN_MESSAGE_MAP(CTINConstruction2View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_READSHP, &CTINConstruction2View::OnReadShapefile)
	ON_COMMAND(ID_TINGENERATE, &CTINConstruction2View::OnTinGenerate)
	ON_COMMAND(ID_GLOBE, &CTINConstruction2View::OnGlobe)
	ON_COMMAND(ID_SELECT, &CTINConstruction2View::OnSelect)
	ON_COMMAND(ID_ZOOMIN, &CTINConstruction2View::OnZoomIn)
	ON_COMMAND(ID_ZOOMOUT, &CTINConstruction2View::OnZoomOut)
	ON_COMMAND(ID_PAN, &CTINConstruction2View::OnPan)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
//	ON_COMMAND(ID_NET_CONSTRUCT, &CTINConstruction2View::OnNetConstruction)
	ON_COMMAND(ID_STARTPNT, &CTINConstruction2View::OnStartPNT)
	ON_COMMAND(ID_PATH_CONSTRUCT, &CTINConstruction2View::OnPathConstruction)
	ON_COMMAND(ID_ENDPNT, &CTINConstruction2View::OnEndPNT)
	ON_COMMAND(ID_TIN_DENSIFY, &CTINConstruction2View::OnTinDensify)
END_MESSAGE_MAP()

vector<PNT> ReadShapefile(const char *filename, char *format) {
	OGRRegisterAll();
	OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(format);
	OGRDataSource *poDS = poDriver->Open(filename);
	vector<PNT> PNTSet;
	if (poDS == NULL) 
	{
		MessageBoxA(NULL, "open failed!\n", "Error", 0); 
	}
	else 
	{
		OGRLayer *poLayer = poDS->GetLayer(0);
		OGRFeature *poFeat = NULL;
		int idx = 0;
		while ((poFeat = poLayer->GetNextFeature()) != NULL) {
			OGRGeometry *poGeometry = poFeat->GetGeometryRef();
			if (poGeometry != NULL) {
				if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
					OGRPoint *poPoint = (OGRPoint *)poGeometry;
					PNT POINT;
					POINT.x = poPoint->getX();
					POINT.y = poPoint->getY();
					PNTSet.push_back(POINT);
					char str[100];
					sprintf_s(str, "%d : (%f, %f)\n", idx, poPoint->getX(), poPoint->getY());
					//MessageBoxA(NULL, str, "coordinate", 0);
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					OGRLineString *poLine = (OGRLineString *)poGeometry;
					int pnt_count = poLine->getNumPoints();
					for (int i = 0; i < pnt_count; i++) {
						OGRPoint *poPoint = new OGRPoint();
						poLine->getPoint(i, poPoint);

						PNT POINT;
						POINT.x = poPoint->getX();
						POINT.y = poPoint->getY();
						PNTSet.push_back(POINT);

						char str[100];
						sprintf_s(str, "%d : (%f, %f)\n", idx, poPoint->getX(), poPoint->getY());
						//MessageBoxA(NULL, str, "coordinate", 0);
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;
				 	 OGRLinearRing* poRing = poPolygon->getExteriorRing();
					 int pnt_count = poRing->getNumPoints();
					 OGRPoint *poPoint = new OGRPoint();
					 for (int i = 0; i < pnt_count; i++) {
						 poRing->getPoint(i, poPoint);
						 PNT POINT;
						 POINT.x = poPoint->getX();
						 POINT.y = poPoint->getY();

						 PNTSet.push_back(POINT);
						 char str[100];
						 sprintf_s(str, "%d : (%f, %f)\n", idx, poPoint->getX(), poPoint->getY());
						 //MessageBoxA(NULL, str, "coordinate", 0);
					 }
				}
			}
			else
			{
				printf("%d : no geometry !\n", idx);
			}
			OGRFeature::DestroyFeature(poFeat);
			idx++;
		}
	}
	OGRDataSource::DestroyDataSource(poDS);
	OGRCleanupAll();
	return PNTSet;
}

void ReadRasterData(const char *filename) {
	GDALAllRegister();
	GDALDataset *pData = (GDALDataset *)GDALOpen(filename, GA_ReadOnly);
	if (pData == NULL) {
		AfxMessageBox("ERRROR!");
		return;
	}

	CString str;
	int nWidth = pData->GetRasterXSize();
	int nHeight = pData->GetRasterYSize();
	str.AppendFormat("%d, %d", nWidth, nHeight);
	AfxMessageBox(str);

	GDALClose(pData);
}
// CTINConstruction2View 构造/析构

CTINConstruction2View::CTINConstruction2View()
{
	// TODO: 在此处添加构造代码
	OperateID = 0;  zoomratio = 1; 	Captured = FALSE;
	//m_hZoomIn = AfxGetApp()->LoadCursor(IDC_ZOOMIN);
	//m_hZoomOut = AfxGetApp()->LoadCursor(IDC_ZOOMOUT);
	//m_hPan = AfxGetApp()->LoadCursor(IDC_PAN);
	//m_hSelect = AfxGetApp()->LoadCursor(IDC_SELECT);
	pointNumber = 0; arcNumber = 0;
	Arc = new ArcSet[_MAX_ARCNUM_aMap];
	xmin = ymin = 1.E+256; xmax = ymax = -1.E+256;
	nFlagPoint = FALSE; nFlagArc = FALSE;

	PointData = NULL;
	CopyData = NULL;
	m_displayGrid = false;
	queuehead = NULL;
	PointCenter = NULL;
	CenterSize = 0;
	runTime[0] = 0; runTime[1] = 0; runTime[2] = 0; runTime[3] = 0; runTime[4] = 0; runTime[5] = 0;

	tinHead = tinEnd = tin = NULL;
	colors[0] = RGB(0, 0, 0);
	colors[1] = RGB(255, 0, 0);
	colors[2] = RGB(0, 255, 0);
	colors[3] = RGB(0, 0, 255);
	colors[4] = RGB(0, 255, 255);
	colors[5] = RGB(255, 0, 255);
	colors[6] = RGB(255, 255, 0);
	colors[7] = RGB(255, 255, 255);
	colors[8] = RGB(0, 127, 127);
	colors[9] = RGB(127, 0, 127);
	colors[10] = RGB(127, 127, 0);
	colors[11] = RGB(10, 200, 100);
	colors[12] = RGB(100, 200, 10);
	colors[13] = RGB(200, 100, 10);
	for (int i = 0; i<BlockGridSize; i++)
	{
		for (int j = 0; j<BlockGridSize; j++)
		{
			blockGrid[i][j].triangleNum = 0;
			blockGrid[i][j].triFirst = NULL;
		}
	}

	pStartPoint = pEndPoint = NULL;
	nStartTri = nEndTri = -1;
	pStartTri = pEndTri = NULL;
}

CTINConstruction2View::~CTINConstruction2View()
{
	ClearQueueMemory(queuehead, PointCenter);
	for (int i = 0; i<BlockGridSize; i++)
	{
		for (int j = 0; j<BlockGridSize; j++)
		{
			TRIANGLENODE *del = NULL;
			for (TRIANGLENODE* t = blockGrid[i][j].triFirst; t != NULL;)
			{
				del = t;
				t = t->next;
				delete del;
			}
		}
	}
}

BOOL CTINConstruction2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CTINConstruction2View 绘制

void CTINConstruction2View::OnDraw(CDC* pDC)
{
	CTINConstruction2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	DrawGraph(pDC);
	// TODO: 在此处为本机数据添加绘制代码
}


// CTINConstruction2View 打印

BOOL CTINConstruction2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CTINConstruction2View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CTINConstruction2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CTINConstruction2View 诊断

#ifdef _DEBUG
void CTINConstruction2View::AssertValid() const
{
	CView::AssertValid();
}

void CTINConstruction2View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTINConstruction2Doc* CTINConstruction2View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTINConstruction2Doc)));
	return (CTINConstruction2Doc*)m_pDocument;
}
#endif //_DEBUG

// CTINConstruction2View 消息处理程序

////////////////////////////////////////////////////////////////////////////


// 将wchar_t类型转化为char *类型
//char *CString2LPSTR(CString& cstr) {
//	int nlen = cstr.GetLength() + 1;
//	char *charstr = new char[nlen];
//	wcstombs_s(0, charstr, nlen, cstr, _TRUNCATE);
//	return charstr;
//}

void CTINConstruction2View::OnReadShapefile()
{
	// TODO: 在此添加命令处理程序代码
	CString  TheFileName;
	CFileDialog  FileDlg(TRUE);

	if (FileDlg.DoModal() == IDOK)
		TheFileName = FileDlg.GetPathName();
	else
		return;

	//char *filename = CString2LPSTR(TheFileName);
	vector<PNT> PNTSet = ReadShapefile(TheFileName, "ESRI Shapefile");
	
	pointNumber =  PNTSet.size();
	PointData = new MyPoint[pointNumber + 4];
	for (int i = 0; i<pointNumber; i++)
	{
		PointData[i].x = PNTSet[i].x;
		PointData[i].y = PNTSet[i].y;
		PointData[i].ID = i;
		PointData[i].label = false;  //表示点未被用
	}
	
	nFlagPoint = true;
	CalcBoundPoint();
	CalcBoundGraph();
	ZoomFull();
	AfxMessageBox(_T("读取点数据完毕！"));
	RefreshScreen();

	CString str;
	str.Format("Count: %d\n", PNTSet.size());
	AfxMessageBox(str);
	PNTSet.clear();
}

void CTINConstruction2View::OnTINNoGroup()
{
	clock_t start, finish, startTotal, finishTotal;
	start = clock();
	startTotal = clock();
	FBNODE *basef = NULL, *basel = NULL; //底壳
	TREENODE *root = NULL, *frontf = NULL, *frontl = NULL; //前壳
	MyPoint *Data = new MyPoint[pointNumber];
	memcpy(Data, PointData, pointNumber * sizeof(MyPoint));
	GeneDelaunay(Data, pointNumber, &tinHead, &tinEnd, &root, &frontf, &frontl, &basef, &basel);
	ClearPartionMemory(frontf, basef);
	finish = clock();
	runTime[1] = (double)(finish - start) / CLOCKS_PER_SEC;
	start = clock();
	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
	delete Data;
	finish = clock();
	runTime[2] = (double)(finish - start) / CLOCKS_PER_SEC;
	finishTotal = clock();
	runTime[4] = (double)(finishTotal - startTotal) / CLOCKS_PER_SEC;
	RefreshScreen();
}

// 刷新绘图区

void CTINConstruction2View::RefreshScreen()
{
	CRect rect;
	GetClientRect(&rect);
	InvalidateRect(&rect);
}

void CTINConstruction2View::OnDelaunayTri()
{
	
}


void CTINConstruction2View::OnTinGenerate()
{
	// TODO: 在此添加命令处理程序代码
	clock_t start, finish, startTotal, finishTotal;
	start = clock();
	startTotal = clock();
	FBNODE *basef = NULL, *basel = NULL; //底壳
	TREENODE *root = NULL, *frontf = NULL, *frontl = NULL; //前壳
	MyPoint *Data = new MyPoint[pointNumber];
	memcpy(Data, PointData, pointNumber * sizeof(MyPoint));
	GeneDelaunay(Data, pointNumber, &tinHead, &tinEnd, &root, &frontf, &frontl, &basef, &basel);
	ClearPartionMemory(frontf, basef);
	finish = clock();
	runTime[1] = (double)(finish - start) / CLOCKS_PER_SEC;
	start = clock();
	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
	delete Data;
	finish = clock();
	runTime[2] = (double)(finish - start) / CLOCKS_PER_SEC;
	finishTotal = clock();
	runTime[4] = (double)(finishTotal - startTotal) / CLOCKS_PER_SEC;
	RefreshScreen();
}


void CTINConstruction2View::GetScreenPoint(PNT *pt)
{
	CRect  Rect;
	GetClientRect(&Rect);
	ClientHeight = Rect.Height();
	ClientWidth = Rect.Width();
	//计算屏幕坐标
	pt->x = (pt->x - xcenter) / zoomratio + ClientWidth / 2;
	pt->y = (pt->y - ycenter) / zoomratio + ClientHeight / 2;
	pt->y = ClientHeight - pt->y;
}
void CTINConstruction2View::GetMapPoint(PNT * pt)
{
	//计算地图坐标
	pt->x = zoomratio*(pt->x - ClientWidth / 2) + xcenter;
	pt->y = zoomratio*(ClientHeight / 2 - pt->y) + ycenter;
}
void CTINConstruction2View::ZoomFull()
{
	//计算客户区的大小	
	CRect  Rect;
	GetClientRect(&Rect);
	ClientHeight = Rect.Height();
	ClientWidth = Rect.Width();

	//全局显示
	xcenter = xmin + dx / 2.;  ycenter = ymin + dy / 2.;
	double zx = dx / ClientWidth, zy = dy / ClientHeight;
	zoomratio = (zx > zy) ? zx : zy;
}
//--------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//////////////////////文件操作//////////////////////////////////////////////
void CTINConstruction2View::LoadFile(int Type)
{
	CString  TheFileName;
	CFileDialog  FileDlg(TRUE);

	if (FileDlg.DoModal() == IDOK)
		TheFileName = FileDlg.GetPathName();
	else
		return;

	FILE * fp = NULL;
	fopen_s(&fp, TheFileName, "r+");
	if (fp == NULL) return;
	CClientDC dc(this);
	if (Type == 1)
	{
		fscanf_s(fp, "%d", &pointNumber);
		PointData = new MyPoint[pointNumber + 4];
		for (int i = 0; i<pointNumber; i++)
		{
			fscanf_s(fp, "%d%lf%lf", &PointData[i].ID, &PointData[i].x, &PointData[i].y);
			PointData[i].ID = i;
			PointData[i].label = false;  //表示点未被用
		}

		nFlagPoint = true;
		CalcBoundPoint();
		CalcBoundGraph();
		ZoomFull();
		AfxMessageBox(_T("读取点数据完毕！"));
		RefreshScreen();
	}
	if (Type == 2)
	{
		char s1[50], s2[50];
		int ArcID, pNo, pNum = 0, aNum = 0;
		PNT * pts = new PNT[_MAX_PNUM_anArc];
		fscanf_s(fp, "%d", &ArcID);
		for (;;)
		{
			if (feof(fp))break;
			fscanf_s(fp, "%s%s", &s1, &s2);
			if (s1[0] == 'E')
			{
				Arc[aNum].pNum = pNum;
				Arc[aNum].pts = new PNT[pNum];
				for (pNo = 0; pNo<pNum; pNo++)
				{
					Arc[aNum].pts[pNo].x = pts[pNo].x;
					Arc[aNum].pts[pNo].y = pts[pNo].y;
				}
				aNum++;  ArcID = atoi(s2);  pNum = 0;
				if (s2[0] == 'E')break;
			}
			else { pts[pNum].x = atof(s1); pts[pNum++].y = atof(s2); }
		}
		arcNumber = aNum;
		nFlagArc = true;
		int k = Arc[0].pNum;
		CalcBoundArc();
		CalcBoundGraph();
		ZoomFull();
		RefreshScreen();
	}
	fclose(fp);
}
//计算线数据的外包
void CTINConstruction2View::CalcBoundArc()
{
	xminArc = yminArc = 1.E+256;  xmaxArc = ymaxArc = -1.E+256;
	for (int i = 0; i<arcNumber; i++)
		for (int j = 0; j<Arc[i].pNum; j++)
		{
			if (Arc[i].pts[j].x>xmaxArc)
				xmaxArc = Arc[i].pts[j].x;
			if (Arc[i].pts[j].x<xminArc)
				xminArc = Arc[i].pts[j].x;
			if (Arc[i].pts[j].y>ymaxArc)
				ymaxArc = Arc[i].pts[j].y;
			if (Arc[i].pts[j].y<yminArc)
				yminArc = Arc[i].pts[j].y;
		}

}
//计算点数据的外包
void CTINConstruction2View::CalcBoundPoint()
{
	xminPoint = yminPoint = 1.E+256; xmaxPoint = ymaxPoint = -1.E+256;
	for (int i = 0; i<pointNumber; i++)
	{
		if (PointData[i].x<xminPoint) xminPoint = PointData[i].x;
		if (PointData[i].x>xmaxPoint) xmaxPoint = PointData[i].x;
		if (PointData[i].y<yminPoint) yminPoint = PointData[i].y;
		if (PointData[i].y>ymaxPoint) ymaxPoint = PointData[i].y;
	}
	CString str;
	str.Format("%.3f, %.3f\n%.3f, %.3f", xminPoint, xmaxPoint, yminPoint, ymaxPoint);
	AfxMessageBox(str);
}
void CTINConstruction2View::CalcBoundGraph()
{


	if (nFlagPoint == TRUE)
	{
		if (xminPoint<xmin) xmin = xminPoint;
		if (xmaxPoint>xmax) xmax = xmaxPoint;
		if (yminPoint<ymin) ymin = yminPoint;
		if (ymaxPoint>ymax) ymax = ymaxPoint;
	}
	if (nFlagArc == TRUE)
	{
		if (xminArc<xmin) xmin = xminArc;
		if (xmaxArc>xmax) xmax = xmaxArc;
		if (yminArc<ymin) ymin = yminArc;
		if (ymaxArc>ymax) ymax = ymaxArc;
	}
	dx = xmax - xmin;  dy = ymax - ymin;
}
//-------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
//////////////////////图形显示操作//////////////////////////////////////////
void CTINConstruction2View::DrawGraph(CDC*pDC)
{
	DrawPoint(pDC, PointData, pointNumber);
	
	//	DrawPoint(pDC, PointCenter, CenterSize, false);
	DrawArc(pDC);
	DrawBinaryLeaf(pDC);
	if (m_displayGrid)
	{
		DrawGrid(pDC);
	}

	DrawTin(pDC, PointData);
}

void CTINConstruction2View::DrawGrid(CDC* pDC)
{
	CPen  NewPen;
	NewPen.CreatePen(PS_DASH, 1, RGB(0, 255, 0));
	CPen *OldPen = pDC->SelectObject(&NewPen);
	//起始点坐标
	double xStart = dXmin, yStart = dYmin;
	//终止点坐标
	double xEnd = dXmax, yEnd = dYmax;
	//每一小格网的长、宽
	//	double xcol_wide = dDx;
	//	double yrow_wide = dDy;
	double xcol_wide = (xEnd - xStart) / 100.0;
	double yrow_wide = (yEnd - yStart) / 100.0;
	//先画行
	for (int row = 0; row <= 100; row++)
	{
		PNT P1 = { xStart, yStart + yrow_wide*row };
		PNT P2 = { xEnd, yStart + yrow_wide*row };
		GetScreenPoint(&P1); GetScreenPoint(&P2);
		pDC->MoveTo(P1.x, P1.y);
		pDC->LineTo(P2.x, P2.y);
	}
	//画列
	for (int col = 0; col <= 100; col++)
	{
		PNT P1 = { xStart + xcol_wide*col, yStart };
		PNT P2 = { xStart + xcol_wide*col, yEnd };
		GetScreenPoint(&P1); GetScreenPoint(&P2);
		pDC->MoveTo(P1.x, P1.y);
		pDC->LineTo(P2.x, P2.y);
	}
	pDC->SelectObject(OldPen);
}

void CTINConstruction2View::DrawBinaryLeaf(CDC* pDC)
{
	CPen  NewPen;
	NewPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
	CPen *OldPen = pDC->SelectObject(&NewPen);
	for (Queue * pList = queuehead; pList != NULL; pList = pList->next)
	{
		BinaryTree *root = pList->treenode;
		if (root == NULL)
		{
			return;
		}
		//起始点坐标
		double xStart = dXmin + root->nStartCol * dDx, yStart = dYmin + root->nStartLine * dDy;
		//终止点坐标
		double xEnd = dXmin + (root->nEndCol + 1) * dDx, yEnd = dYmin + (root->nEndLine + 1) * dDy;

		PNT P1 = { xStart, yStart };
		PNT P2 = { xEnd, yEnd };
		GetScreenPoint(&P1); GetScreenPoint(&P2);
		P1.x += 1, P1.y += 1;
		P2.x -= 1, P2.y -= 1;
		pDC->MoveTo(P1.x, P1.y);	pDC->LineTo(P2.x, P1.y);
		pDC->MoveTo(P2.x, P1.y);	pDC->LineTo(P2.x, P2.y);
		pDC->MoveTo(P2.x, P2.y);	pDC->LineTo(P1.x, P2.y);
		pDC->MoveTo(P1.x, P2.y);	pDC->LineTo(P1.x, P1.y);
	}
	pDC->SelectObject(OldPen);
}

void CTINConstruction2View::RefreshPoint(CDC *pDC, bool IsScreenPoint, double x, double y, COLOR PRGB, COLOR BRGB, int radius)
{
	PNT P = { x,y };
	if (!IsScreenPoint) {
		GetScreenPoint(&P);
	}
	pDC->SetPixel(P.x, P.y, PRGB);
	pDC->Ellipse(P.x - radius, P.y - radius, P.x + radius, P.y + radius);
}

/*
	
*/
void CTINConstruction2View::DrawPoint(CDC* pDC, MyPoint *Data, int counts, COLOR PRGB, COLOR BRGB, int radius)
{
	CPen  NewPen;
	CPen *OldPen = NULL;
	CBrush brush(colors[BRGB]);
	CBrush *pOldBrush = NULL;
	NewPen.CreatePen(PS_SOLID, 1, colors[PRGB]);
	OldPen = pDC->SelectObject(&NewPen);
	pOldBrush = pDC->SelectObject(&brush);
	// 绘制三角网的顶点
	for (int i = 0; i < counts; i++)
		RefreshPoint(pDC, 0, Data[i].x, Data[i].y, PRGB, BRGB, radius);

	CBrush brush2(colors[RED]);
	pDC->SelectObject(&brush2);
	// 绘制起点
	if (pStartPoint) {
		RefreshPoint(pDC, 0, pStartPoint->x, pStartPoint->y, PRGB, RED, 4);
	}
	// 绘制终点
	CBrush brush3(colors[GREEN]);
	pDC->SelectObject(&brush3);
	if (pEndPoint) {
		RefreshPoint(pDC, 0, pEndPoint->x, pEndPoint->y, PRGB, GREEN, 4);
	}

	pDC->SelectObject(OldPen);
	pDC->SelectObject(pOldBrush);
}

void CTINConstruction2View::DrawArc(CDC* pDC)
{
	CPen  NewPen;
	NewPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	CPen *OldPen = pDC->SelectObject(&NewPen);
	for (int i = 0; i<arcNumber; i++)
		RefreshARC(pDC, Arc[i]);
	pDC->SelectObject(OldPen)->DeleteObject();
}
void CTINConstruction2View::RefreshARC(CDC *pDC, ArcSet arc)
{
	for (int i = 0; i<arc.pNum - 1; i++)
		RefreshLine(pDC, arc.pts[i], arc.pts[i + 1]);
}
void CTINConstruction2View::RefreshLine(CDC *pDC, PNT pt1, PNT pt2)
{
	//利用原画笔绘制线段pt1->pt2(地图坐标)
	GetScreenPoint(&pt1);  GetScreenPoint(&pt2);
	double x1, x2, y1, y2, R = 4000., RxR = 16.E+6, Dx, Dy, Dxy, Theta, kxy, x, y, Rxy;
	int IDA = 0, IDB = 0, ID_Draw = 0;
	x1 = pt1.x; y1 = pt1.y;  x2 = pt2.x; y2 = pt2.y;
	if (x1*x1 + y1*y1 <= RxR)IDA = 1;
	if (x2*x2 + y2*y2 <= RxR)IDB = 1;
	if (IDA + IDB == 2)ID_Draw = 1;
	else if (IDA + IDB == 1)
	{
		if (IDA == 1) { Dx = x2 - x1; Dy = y2 - y1; }
		if (IDB == 1) { Dx = x1 - x2; Dy = y1 - y2; }
		if (Dx == 0.)    Theta = (Dy >= 0.) ? _PI / 2. : -_PI / 2.;
		else          Theta = atan(fabs(Dy / Dx));
		if (Dx>0 && Dy<0)Theta *= -1;
		if (Dx<0)      Theta = (Dy >= 0.) ? (_PI - Theta) : (_PI + Theta);
		if (IDA == 1) { x2 = x1 + R*cos(Theta) * 2;  y2 = y1 + R*sin(Theta) * 2; }
		else if (IDB == 1) { x1 = x2 + R*cos(Theta) * 2;  y1 = y2 + R*sin(Theta) * 2; }
		ID_Draw = 1;
	}
	else
	{
		Dx = x2 - x1;  Dy = y2 - y1;  Dxy = x2*y1 - x1*y2;
		if (Dxy*Dxy<R*R*(Dx*Dx + Dy*Dy))
		{
			Rxy = Dx*Dx + Dy*Dy;   x = -Dy*Dxy / Rxy;   y = Dx*Dxy / Rxy;
			if (isIn_Qujian(x, x1, x2) && isIn_Qujian(y, y1, y2))
			{
				ID_Draw = 1;
				if (fabs(Dy) >= fabs(Dx))
				{
					kxy = Dx / Dy; Dy = R*3.; Dx = kxy*Dy; x1 = x + Dx; y1 = y + Dy; x2 = x - Dx; y2 = y - Dy;
				}
				else
				{
					kxy = Dy / Dx;  Dx = R*3.; Dy = kxy*Dx; x1 = x + Dx; y1 = y + Dy; x2 = x - Dx; y2 = y - Dy;
				}
			}
		}
	}
	if (ID_Draw == 1) { pDC->MoveTo((int)x1, (int)y1); pDC->LineTo((int)x2, (int)y2); }
}
bool CTINConstruction2View::isIn_Qujian(double x, double x1, double x2)
{
	if (x1 >= x2)
	{
		if (x >= x2&&x <= x1)
		{
			return true;
		}
	}
	else
	{
		if (x >= x1&&x <= x2)
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//1.初始化栅格场,该栅格场起辅助作用
//void CTINConstruction2View::InitialGrid(int &Line, int &Col, int &ndivision)
//{
//	int i = 0, j = 0;
//	int C = 0, L = 0;
//	double x = 0, y = 0;
//	CGridDlg dlgGrid;
//	int nsqrt = sqrt(pointNumber);
//	dlgGrid.m_nLineSize = dlgGrid.m_nColumnSize = nsqrt * 4;
//	dlgGrid.m_nDivision = 2;
//	if (dlgGrid.DoModal() == IDOK)
//	{
//		Line = dlgGrid.m_nLineSize;
//		Col = dlgGrid.m_nColumnSize;
//		ndivision = dlgGrid.m_nDivision;
//	}
//	else
//		return;
//	dXdelt = dx, dYdelt = dy;
//	dXmin = xmin - dXdelt*0.01;
//	dXmax = xmax + dXdelt*0.01;
//	dYmin = ymin - dYdelt*0.01;
//	dYmax = ymax + dYdelt*0.01;
//	dXdelt = dXmax - dXmin;
//	dYdelt = dYmax - dYmin;
//	if (dXdelt < dYdelt)
//	{
//		dDx = dXdelt / Col;
//		Line = dYdelt / dDx;
//		dDy = dYdelt / Line;
//	}
//	else if (dXdelt > dYdelt)
//	{
//		dDy = dYdelt / Line;
//		Col = dXdelt / dDy;
//		dDx = dXdelt / Col;
//	}
//	else
//	{
//		dDx = dXdelt / Col, dDy = dYdelt / Line;
//	}
//}

//2.将点集分配到栅格场格网中
GridField** CTINConstruction2View::CraeteGridField(MyPoint *Data, int num, int Line, int Col, double dxmin, double dymin, double ddx, double ddy)
{
	int i = 0, j = 0;
	int C = 0, L = 0;
	double x = 0, y = 0;
	//初始化二维数组
	GridField **GridArray = new GridField*[Line];
	for (i = 0; i < Line; i++)
	{
		GridArray[i] = new GridField[Col];
	}
	for (i = 0; i < Line; i++)
		for (j = 0; j < Col; j++)
		{
			GridArray[i][j].pNum = 0;
			GridArray[i][j].ptId = NULL;
			GridArray[i][j].bvisited = false;
		}
	//计算每个点所在的行列号
	for (i = 0; i < num; i++)
	{
		x = Data[i].x;    y = Data[i].y;
		C = (int)((x - dxmin) / ddx);
		L = (int)((y - dymin) / ddy);
		Data[i].nC = C;
		Data[i].nL = L;
		GridArray[L][C].pNum++;    //一个栅格中可以有一个点、多个点，也可以没有点
	}
	//重新开辟数组大小
	for (i = 0; i < Line; i++)
		for (j = 0; j < Col; j++)
			if (GridArray[i][j].pNum != 0)    //当栅格中的点数不为零时，分配空间存储点
			{
				GridArray[i][j].ptId = new int[GridArray[i][j].pNum];     //何时释放内存？？？？？？？
				GridArray[i][j].pNum = 0;
			}
	//将原始点集分配到对应的格网中
	for (i = 0; i < num; i++)
	{
		C = Data[i].nC;
		L = Data[i].nL;
		GridArray[L][C].ptId[GridArray[L][C].pNum] = Data[i].ID;
		GridArray[L][C].pNum++;
	}
	return GridArray;
}

//3.自适应分组
Queue* CTINConstruction2View::CreateQueue(GridField **GridArray, int Line, int Col, int ndivision, int num)
{
	BinaryTree *treeroot = GreateTreeNode(0, Line - 1, 0, Col - 1, num, 0);
	Queue *queuefirst = NULL, *queuelast = NULL;
	queuefirst = queuelast = GreateQueueElement(treeroot);
	Queue *List = new Queue;
	AdaptiveDivision(queuefirst, queuelast, ndivision, GridArray, List);
	queuelast->next = List->next;
	delete List;
	return queuefirst;
}

//创建队列的节点
Queue* CTINConstruction2View::GreateQueueElement(BinaryTree *element)
{
	Queue *pCreateElement = new Queue;
	pCreateElement->treenode = element;
	pCreateElement->next = NULL;
	return pCreateElement;
}

//建立二叉树的节点
BinaryTree* CTINConstruction2View::GreateTreeNode(int startLine, int endLine, int startCol, int endCol, int pNum, int level)
{
	BinaryTree *pCreateNode = new BinaryTree;
	pCreateNode->nStartLine = startLine;  pCreateNode->nStartCol = startCol;
	pCreateNode->nEndLine = endLine;      pCreateNode->nEndCol = endCol;
	pCreateNode->npNum = pNum;            pCreateNode->nLevel = level;
	return pCreateNode;
}

//自适应分组
void CTINConstruction2View::AdaptiveDivision(Queue *&head, Queue *&rear, int ndivision, GridField **GridArray, Queue *List)
{
	while (head != NULL)
	{
		BinaryTree *pTreenode = head->treenode;
		if (pTreenode->nLevel == ndivision)       //达到分裂次数时停止
		{
			return;
		}
		int pNumTotal = pTreenode->npNum;
		if (pNumTotal < 2)       //不足两个点时
		{
			if (head == rear)
			{
				return;
			}
			Queue *pTemp = head;
			head = head->next;
			pTemp->next = List->next;
			List->next = pTemp;
			continue;
		}
		int nCurrentTotal = 0;
		double xdist = (pTreenode->nEndCol - pTreenode->nStartCol + 1) * dDx;
		double ydist = (pTreenode->nEndLine - pTreenode->nStartLine + 1) * dDy;
		if (xdist > ydist && pTreenode->nStartCol != pTreenode->nEndCol)
		{
			bool NotInColGroup = true;
			int nCol = pTreenode->nStartCol;
			for (int i = pTreenode->nStartLine; i <= pTreenode->nEndLine; i++)
			{
				nCurrentTotal += GridArray[i][nCol].pNum;
			}
			if (nCurrentTotal == pNumTotal)
			{
				NotInColGroup = false;
			}
			while (NotInColGroup)
			{
				int nextColTotal = 0;
				nCol++;
				for (int i = pTreenode->nStartLine; i <= pTreenode->nEndLine; i++)
					nextColTotal += GridArray[i][nCol].pNum;
				if (nCurrentTotal == pNumTotal)
				{
					NotInColGroup = false;
					break;
				}
				if (fabs(pNumTotal - 2 * nCurrentTotal) < fabs(pNumTotal - 2 * (nCurrentTotal + nextColTotal)))
				{
					break;
				}
				nCurrentTotal += nextColTotal;
			}
			if (NotInColGroup)
			{
				BinaryTree *LeftLeaf = GreateTreeNode(pTreenode->nStartLine, pTreenode->nEndLine, pTreenode->nStartCol,
					nCol - 1, nCurrentTotal, pTreenode->nLevel + 1);
				BinaryTree *RightLeaf = GreateTreeNode(pTreenode->nStartLine, pTreenode->nEndLine, nCol, pTreenode->nEndCol,
					pNumTotal - nCurrentTotal, pTreenode->nLevel + 1);
				Queue *pElement = GreateQueueElement(LeftLeaf);
				rear->next = pElement; rear = pElement;
				pElement = GreateQueueElement(RightLeaf);
				rear->next = pElement; rear = pElement;
			}
			else
			{
				if (head == rear)
				{
					return;
				}
				Queue *pTemp = head;
				head = head->next;
				pTemp->next = List->next;
				List->next = pTemp;
				continue;
			}
		}
		else if (xdist > ydist && pTreenode->nStartCol == pTreenode->nEndCol)
		{
			if (head == rear)
			{
				return;
			}
			Queue *pTemp = head;
			head = head->next;
			pTemp->next = List->next;
			List->next = pTemp;
			continue;
		}
		else if (xdist <= ydist && pTreenode->nStartLine != pTreenode->nEndLine)
		{
			bool NotInLineGroup = true;
			int nLin = pTreenode->nStartLine;
			for (int i = pTreenode->nStartCol; i <= pTreenode->nEndCol; i++)
			{
				nCurrentTotal += GridArray[nLin][i].pNum;
			}
			if (nCurrentTotal == pNumTotal)
			{
				NotInLineGroup = false;
			}
			while (NotInLineGroup)
			{
				int nextLinTotal = 0;
				nLin++;
				for (int i = pTreenode->nStartCol; i <= pTreenode->nEndCol; i++)
					nextLinTotal += GridArray[nLin][i].pNum;
				if (nCurrentTotal == pNumTotal)
				{
					NotInLineGroup = false;
					break;
				}
				if (fabs(pNumTotal - 2 * nCurrentTotal) < fabs(pNumTotal - 2 * (nCurrentTotal + nextLinTotal)))
				{
					break;
				}
				nCurrentTotal += nextLinTotal;
			}
			if (NotInLineGroup)
			{
				BinaryTree *LeftLeaf = GreateTreeNode(pTreenode->nStartLine, nLin - 1, pTreenode->nStartCol,
					pTreenode->nEndCol, nCurrentTotal, pTreenode->nLevel + 1);
				BinaryTree *RightLeaf = GreateTreeNode(nLin, pTreenode->nEndLine, pTreenode->nStartCol, pTreenode->nEndCol,
					pNumTotal - nCurrentTotal, pTreenode->nLevel + 1);
				Queue *pElement = GreateQueueElement(LeftLeaf);
				rear->next = pElement; rear = pElement;
				pElement = GreateQueueElement(RightLeaf);
				rear->next = pElement; rear = pElement;
			}
			else
			{
				if (head == rear)
				{
					return;
				}
				Queue *pTemp = head;
				head = head->next;
				pTemp->next = List->next;
				List->next = pTemp;
				continue;
			}
		}
		else if (xdist <= ydist && pTreenode->nStartLine == pTreenode->nEndLine)
		{
			if (head == rear)
			{
				return;
			}
			Queue *pTemp = head;
			head = head->next;
			pTemp->next = List->next;
			List->next = pTemp;
			continue;
		}
		Queue *pTemp = head;
		head = head->next;
		delete pTemp;
		delete pTreenode;
	}
}

//4.提取每块的近似中心点
void CTINConstruction2View::AbstractCenterPoint(GridField **GridArray, Queue *head, MyPoint *&Data, int &num, MyPoint *OriginalData)
{
	int L = 0, C = 0;
	num = 0;
	for (Queue *q = head; q != NULL; q = q->next)
	{
		num++;
	}
	Data = new MyPoint[num + 4];
	num = 0;
	for (Queue *q = head; q != NULL; q = q->next)
	{
		BinaryTree *pTreeLeaf = q->treenode;
		L = (pTreeLeaf->nStartLine + pTreeLeaf->nEndLine) / 2;
		C = (pTreeLeaf->nStartCol + pTreeLeaf->nEndCol) / 2;
		if (GridArray[L][C].pNum != 0)
		{
			Data[num].ID = GridArray[L][C].ptId[0];
			Data[num].x = OriginalData[Data[num].ID].x;
			Data[num].y = OriginalData[Data[num].ID].y;
			OriginalData[Data[num].ID].label = true;
			num++;
			continue;
		}
		GridArray[L][C].bvisited = true;
		QueueForCenter *abstractfirst = NULL, *abstractlast = NULL;
		abstractlast = abstractfirst = GreateElementForCenter(L, C);
		while (abstractfirst != NULL)
		{
			if (GridArray[abstractfirst->nL][abstractfirst->nC].pNum > 0)
			{
				Data[num].ID = GridArray[abstractfirst->nL][abstractfirst->nC].ptId[0];
				Data[num].x = OriginalData[Data[num].ID].x;
				Data[num].y = OriginalData[Data[num].ID].y;
				OriginalData[Data[num].ID].label = true;
				num++;
				while (abstractfirst != NULL)
				{
					QueueForCenter *pdelete = abstractfirst;
					abstractfirst = abstractfirst->next;
					delete pdelete;
				}
				break;
			}
			if (abstractfirst->nC - 1 >= pTreeLeaf->nStartCol && GridArray[abstractfirst->nL][abstractfirst->nC - 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL, abstractfirst->nC - 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nC + 1 <= pTreeLeaf->nEndCol && GridArray[abstractfirst->nL][abstractfirst->nC + 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL, abstractfirst->nC + 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nL - 1 >= pTreeLeaf->nStartLine && GridArray[abstractfirst->nL - 1][abstractfirst->nC].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL - 1, abstractfirst->nC);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nL + 1 <= pTreeLeaf->nEndLine && GridArray[abstractfirst->nL + 1][abstractfirst->nC].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL + 1, abstractfirst->nC);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			//四个角点
			if (abstractfirst->nC - 1 >= pTreeLeaf->nStartCol && abstractfirst->nL - 1 >= pTreeLeaf->nStartLine
				&& GridArray[abstractfirst->nL - 1][abstractfirst->nC - 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL - 1, abstractfirst->nC - 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nC - 1 >= pTreeLeaf->nStartCol && abstractfirst->nL + 1 <= pTreeLeaf->nEndLine
				&& GridArray[abstractfirst->nL + 1][abstractfirst->nC - 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL + 1, abstractfirst->nC - 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nC + 1 <= pTreeLeaf->nEndCol && abstractfirst->nL + 1 <= pTreeLeaf->nEndLine
				&& GridArray[abstractfirst->nL + 1][abstractfirst->nC + 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL + 1, abstractfirst->nC + 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			if (abstractfirst->nC + 1 <= pTreeLeaf->nEndCol && abstractfirst->nL - 1 >= pTreeLeaf->nStartLine
				&& GridArray[abstractfirst->nL - 1][abstractfirst->nC + 1].bvisited == false)
			{
				QueueForCenter *pElement = GreateElementForCenter(abstractfirst->nL - 1, abstractfirst->nC + 1);
				InsertElementForCenter(abstractlast, pElement, GridArray);
			}
			QueueForCenter *pdelete = abstractfirst;
			abstractfirst = abstractfirst->next;
			delete pdelete;
		}
	}
}
void CTINConstruction2View::InsertElementForCenter(QueueForCenter *&rear, QueueForCenter *insertelement, GridField **GridArray)
{
	rear->next = insertelement;
	rear = insertelement;
	GridArray[insertelement->nL][insertelement->nC].bvisited = true;
}
QueueForCenter* CTINConstruction2View::GreateElementForCenter(int L, int C)
{
	QueueForCenter *pElement = new QueueForCenter;
	pElement->nC = C;
	pElement->nL = L;
	pElement->next = NULL;
	return pElement;
}
//5.删除内存
void CTINConstruction2View::ClearGridFieldMemory(GridField **GridArray, int Line, int Col)
{
	for (int i = 0; i < nLineSize; i++)
		for (int j = 0; j < nColSize; j++)
			if (GridArray[i][j].pNum != 0)
			{
				delete GridArray[i][j].ptId;
			}
	for (int i = 0; i < nLineSize; i++)
	{
		delete[]GridArray[i];
	}
	delete GridArray;
}
void CTINConstruction2View::ClearQueueMemory(Queue *head, MyPoint *Data)
{
	while (head != NULL)
	{
		Queue *pdeleteQueue = head;
		BinaryTree *pdeleteTree = pdeleteQueue->treenode;
		head = head->next;
		delete pdeleteTree;
		delete pdeleteQueue;
	}
	if (Data != NULL)
	{
		delete[]Data;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////自适应分组///////////////////////////////////////////////////////////////
//void CTINConstruction2View::OnBinaryGroup()
//{
//	int ndivision = 0;
//	InitialGrid(nLineSize, nColSize, ndivision);  //初始化辅助栅格网
//	GridField **GridArray = CraeteGridField(PointData, pointNumber, nLineSize, nColSize, dXmin, dYmin, dDx, dDy);
//	queuehead = CreateQueue(GridArray, nLineSize, nColSize, ndivision, pointNumber);
//	AbstractCenterPoint(GridArray, queuehead, PointCenter, CenterSize, PointData);
//	ClearGridFieldMemory(GridArray, nLineSize, nColSize);
//	RefreshScreen();
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




//****************************************************
//-------------Tin扫描线算法--------------------------
void CTINConstruction2View::swap(MyPoint r[], int i, int j)
{
	double x, y, xuhao;
	x = r[i].x; y = r[i].y; xuhao = r[i].ID;
	r[i].x = r[j].x; r[i].y = r[j].y; r[i].ID = r[j].ID;
	r[j].x = x; r[j].y = y; r[j].ID = xuhao;
}

//快速排序，以x坐标
//参数：point——点数据集
void CTINConstruction2View::qs_x(MyPoint *point, int left, int right)
{
	int i, j, x, y, z;
	i = left; j = right;
	x = point[(left + right) / 2].x;
	int ID;
	do {
		while ((point[i].x>x) && (i<right)) i++;
		while ((x>point[j].x) && (j>left))  j--;
		if (i <= j)
		{
			y = point[i].x;	z = point[i].y; ID = point[i].ID;
			point[i].x = point[j].x; point[i].ID = point[j].ID;
			point[i].y = point[j].y; 	point[j].x = y;  point[j].y = z; point[j].ID = ID;
			i++; j--;
		}
	} while (i <= j);
	if (left<j)
		qs_x(point, left, j);
	if (i<right)
		qs_x(point, i, right);
}
int CTINConstruction2View::Partition(MyPoint r[], int first, int end)
{
	int i = first, j = end;     //初始化
	while (i<j)
	{
		while (i<j&&r[i].y <= r[j].y) j--;  //右侧扫描
		if (i<j)
		{
			swap(r, i, j);       //将较小记录交换到前面
			i++;
		}

		while (i<j&&r[i].y <= r[j].y) i++;    //左侧扫描
		if (i<j)
		{
			swap(r, i, j);   //将较大记录交换到后面
			j--;
		}
	}
	return i;   // i为轴值记录的最终位置
}
void CTINConstruction2View::QuickSort(MyPoint r[], int first, int end)
{
	if (first<end)
	{
		int pivot = Partition(r, first, end);
		QuickSort(r, first, pivot - 1);
		QuickSort(r, pivot + 1, end);
	}
}
//
void CTINConstruction2View::AddNewPointer(TRIANGLE *tinNode1, TRIANGLE *tinNode2)
{
	int q1, q2, no;
	if (HaveSameEdgeScan(tinNode1->ID1, tinNode1->ID2, tinNode1->ID3, tinNode2->ID1, tinNode2->ID2, tinNode2->ID3, &q1, &q2))
	{
		no = TheNumber(q1, q2, tinNode1->ID1, tinNode1->ID2, tinNode1->ID3);
		if (no == 1)  tinNode1->p1tin = tinNode2;
		else if (no == 2) tinNode1->p2tin = tinNode2;
		else if (no == 3) tinNode1->p3tin = tinNode2;
		no = TheNumber(q1, q2, tinNode2->ID1, tinNode2->ID2, tinNode2->ID3);
		if (no == 1)  tinNode2->p1tin = tinNode1;
		else if (no == 2) tinNode2->p2tin = tinNode1;
		else if (no == 3) tinNode2->p3tin = tinNode1;
	}
	else
		AfxMessageBox("AddNewPointer出错！");
}
//加入新三角形到三角网中
//参数：a，b，c——三角形顶点编号
//		p——相关三角形
//		tinlast——三角网的尾指针
void CTINConstruction2View::AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast)
{
	TRIANGLE *tinNew = new TRIANGLE;
	tinNew->back = NULL;  tinNew->next = NULL;
	tinNew->p1tin = NULL; tinNew->p2tin = NULL; tinNew->p3tin = NULL;
	tinNew->ID1 = a;   tinNew->ID2 = b;   tinNew->ID3 = c;
	int SeqNum = (*tinlast)->g_SeqNum;
	tinNew->g_SeqNum = ++SeqNum;
	tinNew->p1tin = NULL;  tinNew->p2tin = NULL;   tinNew->p3tin = NULL;
	(*tinlast)->next = tinNew;  tinNew->back = *tinlast;
	*tinlast = tinNew;
	int q1, q2, no;
	if (HaveSameEdgeScan(tinNew->ID1, tinNew->ID2, tinNew->ID3, p->ID1, p->ID2, p->ID3, &q1, &q2))
	{
		no = TheNumber(q1, q2, tinNew->ID1, tinNew->ID2, tinNew->ID3);
		if (no == 1)  tinNew->p1tin = p;
		else if (no == 2) tinNew->p2tin = p;
		else if (no == 3) tinNew->p3tin = p;
		no = TheNumber(q1, q2, p->ID1, p->ID2, p->ID3);
		if (no == 1)  p->p1tin = tinNew;
		else if (no == 2) p->p2tin = tinNew;
		else if (no == 3) p->p3tin = tinNew;
	}
	else
		AfxMessageBox("AddNewTin出错！");
}
int CTINConstruction2View::TheOtherPoint(int p1, int p2, int a, int b, int c)
{
	if ((p1 == a&&p2 == b) || (p1 == b&&p2 == a))
	{
		return c;
	}
	else if ((p1 == a&&p2 == c) || (p1 == c&&p2 == a))
	{
		return b;
	}
	else if ((p1 == b&&p2 == c) || (p1 == c&&p2 == b))
	{
		return a;
	}
	else
		return -1;
}
//加入新三角形到三角网中
//参数：a，b，c——三角形顶点编号
//		p——相关三角形
//		tinlast——三角网的尾指针
void CTINConstruction2View::AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast, MyPoint *original)
{
	TRIANGLE *tinNew = new TRIANGLE;
	tinNew->back = NULL;
	tinNew->next = NULL;
	tinNew->ID1 = a;
	tinNew->ID2 = b;
	tinNew->ID3 = c;
	int SeqNum = (*tinlast)->g_SeqNum;
	tinNew->g_SeqNum = ++SeqNum;
	tinNew->p1tin = NULL;
	tinNew->p2tin = NULL;
	tinNew->p3tin = NULL;
	(*tinlast)->next = tinNew;
	tinNew->back = *tinlast;
	*tinlast = tinNew;
	int q1, q2, no;			//q1和q2相同边的两个顶点
	int q3, q4;
	if (HaveSameEdgeScan(tinNew->ID1, tinNew->ID2, tinNew->ID3, p->ID1, p->ID2, p->ID3, &q1, &q2))
	{
		no = TheNumber(q1, q2, tinNew->ID1, tinNew->ID2, tinNew->ID3);
		if (no == 1)  tinNew->p1tin = p;
		else if (no == 2) tinNew->p2tin = p;
		else if (no == 3) tinNew->p3tin = p;
		no = TheNumber(q1, q2, p->ID1, p->ID2, p->ID3);
		if (no == 1)  p->p1tin = tinNew;
		else if (no == 2) p->p2tin = tinNew;
		else if (no == 3) p->p3tin = tinNew;
		q3 = TheOtherPoint(q1, q2, p->ID1, p->ID2, p->ID3);
		q4 = TheOtherPoint(q1, q2, tinNew->ID1, tinNew->ID2, tinNew->ID3);
		if (PointInCircle(original, q4, p->ID1, p->ID2, p->ID3))
		{
			EdgeChange(q3, q4, q1, q2, &p, &tinNew);
		}

	}
	else
		AfxMessageBox("AddNewTin出错！");
}
//处理第一个三角形中存在共线的情况
//参数：a，b，c——所加入三角形顶点的序号
//		p——三角形的头指针，通过p将共线点的三角形加入的三角网中
//		tinlast——三角网的尾指针
void CTINConstruction2View::FirAddTin(int a, int b, int c, TRIANGLE **p, TRIANGLE **tinlast)
{
	TRIANGLE *tinNew = new TRIANGLE;
	tinNew->back = NULL;  tinNew->next = NULL;
	tinNew->ID1 = a;   tinNew->ID2 = b;   tinNew->ID3 = c;
	int SeqNum = (*tinlast)->g_SeqNum;
	tinNew->g_SeqNum = ++SeqNum;
	tinNew->p1tin = NULL;  tinNew->p2tin = NULL;   tinNew->p3tin = NULL;
	(*tinlast)->next = tinNew;  tinNew->back = *tinlast;
	*tinlast = tinNew;
	if (*p != NULL)
	{
		int n = TheNumber(a, b, a, b, c);
		if (n == 1) tinNew->p1tin = *p;    //
		else if (n == 2) tinNew->p2tin = *p;
		else if (n == 3) tinNew->p3tin = *p;
		else
			AfxMessageBox("FirAddTin出错！");
		n = TheNumber(a, b, (*p)->ID1, (*p)->ID2, (*p)->ID3);
		if (n == 1) (*p)->p1tin = tinNew;
		else if (n == 2) (*p)->p2tin = tinNew;
		else if (n == 3) (*p)->p3tin = tinNew;
		else
			AfxMessageBox("FirAddTin出错！");
	}
	*p = tinNew;
}
//清除前壳和底壳的内存
void CTINConstruction2View::ClearPartionMemory(TREENODE *treefirst, FB *fbfirst)
{
	while (treefirst != NULL)
	{
		TREENODE	*treeNode = treefirst;
		treefirst = treefirst->next;
		delete treeNode;
	}
	while (fbfirst != NULL)
	{
		FB *fbNode = fbfirst;
		fbfirst = fbfirst->next;
		delete fbNode;
	}
}
void CTINConstruction2View::ClearTinMemory(TRIANGLE **tin)
{
	TRIANGLE *t;
	while ((*tin) != NULL)
	{
		t = *tin;
		*tin = (*tin)->next;
		delete t;
	}
}
//加入第一个三角形
//参数：tinfirst——三角网的头指针；tinlast——尾指针
//		a,b,c——第一个三角形的三个顶点序号
void CTINConstruction2View::AddFirstTin(TRIANGLE **tinfirst, TRIANGLE **tinlast, int a, int b, int c)
{
	TRIANGLE *tinNew = new TRIANGLE;
	tinNew->ID1 = a;    tinNew->ID2 = b;    tinNew->ID3 = c;
	tinNew->p1tin = NULL;    tinNew->p2tin = NULL;  tinNew->p3tin = NULL;
	tinNew->g_SeqNum = 0;
	tinNew->next = NULL;   tinNew->back = NULL;
	*tinfirst = *tinlast = tinNew;
}
void CTINConstruction2View::TreeDelete(TREENODE *p, int no, TREENODE **Root)
{
	TREENODE *f, *a, *b, *s, *h;
	TREENODE *root = *Root;
	f = p->parent;
	if (p->lchild == NULL&&p->rchild == NULL)  h = NULL;
	else if (p->lchild == NULL) h = p->rchild;
	else if (p->rchild == NULL) h = p->lchild;
	else
	{
		a = p->lchild; b = p->rchild;
		if (b->lchild == NULL) { h = b; b->lchild = a; a->parent = b; }
		else
		{
			h = b;
			s = h;
			while (h->lchild)
			{
				s = h; h = h->lchild;
			}
			s->lchild = h->rchild;
			if (h->rchild != NULL) h->rchild->parent = s;
			h->rchild = b;
			if (b != NULL) b->parent = h;
			h->lchild = a;
			if (a != NULL) a->parent = h;
		}
	}
	if (f == NULL) { root = h; if (h != NULL) h->parent = NULL; *Root = root; }
	else if (f->lchild == p) { f->lchild = h; if (h != NULL) h->parent = f; }
	else { f->rchild = h; if (h != NULL) h->parent = f; }
}
//将新加入的点生成的三角形的边加入的前壳中
//参数：Point——点数据集
//		p——当前节点
//		tin——三角网的尾指针
//		no——节点的编号
//		flag——插入标志
void CTINConstruction2View::TreeInsert(MyPoint *Point, TREENODE *p, TRIANGLENODE *tin, int no, int flag)    //不包括root
{
	TREENODE *r;
	if (p != NULL)
	{
		r = new TREENODE;
		r->no = no;
		r->lchild = r->rchild = r->parent = NULL;
		r->tri = tin;
		if (Point[p->no].x>Point[no].x)
		{
			p->lchild = r; r->parent = p;
		}
		else
		{
			p->rchild = r; r->parent = p;
		}
		if (flag == 0)
		{
			r->next = p->next;
			r->back = p;
			p->next->back = r;
			p->next = r;
		}
		else
		{
			r->back = p->back;
			r->next = p;
			p->back->next = r;
			p->back = r;
		}
	}
}
void CTINConstruction2View::TreeSearch(MyPoint *Point, TREENODE **root, TREENODE **f, int no)
{
	TREENODE *p, *q;
	q = NULL;
	p = *root;
	while (p)
	{
		q = p;
		if (Point[no].x<Point[p->no].x)
			p = p->lchild;
		else
			p = p->rchild;
	}
	*f = q;
}
int CTINConstruction2View::SameEdge(int P1, int P2, int PA, int PB)
{
	int Same = -1;  if (P1 == PA&&P2 == PB)Same = 1;
	else if (P1 == PB&&P2 == PA)Same = 1;
	return Same;
}

//点c在直线ab上方就返回
int CTINConstruction2View::PointUpLine(MyPoint *Point, int a, int b, int c)
{
	int t = 0;
	if (Point[a].x == Point[b].x)
		t = 1;
	else if ((Point[a].y - Point[b].y) / (Point[a].x - Point[b].x) >= 0)
	{
		if (Point[a].x == Point[c].x)
		{
			if (Point[c].y>Point[a].y)
				t = 1;
		}
		else if ((Point[a].y - Point[c].y) / (Point[a].x - Point[c].x) >= (Point[a].y - Point[b].y) / (Point[a].x - Point[b].x))
			t = 1;
	}
	else if ((Point[a].y - Point[b].y) / (Point[a].x - Point[b].x)<0)
	{
		if (Point[a].x == Point[c].x)
		{
			if (Point[c].y>Point[a].y)
				t = 1;
		}
		else if ((Point[a].y - Point[c].y) / (Point[a].x - Point[c].x) >= (Point[a].y - Point[b].y) / (Point[a].x - Point[b].x))
			t = 1;
	}
	return t;
}

int CTINConstruction2View::HaveSameEdgeScan(int P1, int P2, int P3, int PA, int PB, int PC, int *Q1, int *Q2)
{
	int Q = 0;
	if (SameEdge(P1, P2, PA, PB) == 1) { *Q2 = P1; *Q1 = P2; Q = 1; }
	else if (SameEdge(P1, P2, PB, PC) == 1) { *Q2 = P1; *Q1 = P2; Q = 1; }
	else if (SameEdge(P1, P2, PC, PA) == 1) { *Q2 = P1; *Q1 = P2; Q = 1; }
	else if (SameEdge(P2, P3, PA, PB) == 1) { *Q2 = P2; *Q1 = P3; Q = 1; }
	else if (SameEdge(P2, P3, PB, PC) == 1) { *Q2 = P2; *Q1 = P3; Q = 1; }
	else if (SameEdge(P2, P3, PC, PA) == 1) { *Q2 = P2; *Q1 = P3; Q = 1; }
	else if (SameEdge(P1, P3, PA, PB) == 1) { *Q2 = P1; *Q1 = P3; Q = 1; }
	else if (SameEdge(P1, P3, PB, PC) == 1) { *Q2 = P1; *Q1 = P3; Q = 1; }
	else if (SameEdge(P1, P3, PC, PA) == 1) { *Q2 = P1; *Q1 = P3; Q = 1; }
	return Q;
}
//判断m,n
//ThuNumber函数首先将a,b,c三个参数从小到大排序
//然后将b,c与排序好的三个有序数相比
//与三个有序数前两个相等，则返回1，即b,c的值为a,b,c中两个较小的值
//与后两个相等，则返回2，即b,c为a,b,c中较大的两个值
//与前后两个相等,则返回3,即b,c为a,b,c中一大一下两个值
//程序首先要快速排序（安装y值从小到大排序），所以点的序号与大小直接相关
int CTINConstruction2View::TheNumber(int m, int n, int a, int b, int c)
{
	int d[3] = { a, b, c };
	int t;
	if (d[1]<d[0]) { t = d[0]; d[0] = d[1]; d[1] = t; }
	if (d[2]<d[0]) { t = d[0]; d[0] = d[2]; d[2] = t; }
	if (d[2]<d[1]) { t = d[1]; d[1] = d[2]; d[2] = t; }   //从小到大排序
	if ((m == d[0] && n == d[1]) || (n == d[0] && m == d[1]))  //m，n等于前两个值
		return 1;
	else if ((m == d[1] && n == d[2]) || (n == d[1] && m == d[2]))  //m，n等于后两个值
		return 2;
	else if ((m == d[0] && n == d[2]) || (n == d[0] && m == d[2]))  //m，n等于前后两个值
		return 3;
	else
	{
		AfxMessageBox("TheNumber函数出错!");
		return -1;
	}
}
//情况2——新加入点的投影未落到前壳上
//参数：i——节点编号
//		Data——点数据集；pNum——数据集大小
//		tinfirst——三角网的头指针；tinlast——三角网的尾指针
//		root，frontf，frontl——二叉排序树，存储前壳
//		basef，basel——双向链表，存储底壳
void CTINConstruction2View::Circs2(int i, MyPoint *Point, TRIANGLE **tinlast, TREENODE **Root, TREENODE **Frontf, TREENODE **Frontl, FB **Basef, FB **Basel)
{

	TREENODE *frontf = *Frontf, *frontl = *Frontl;
	FB *basef = *Basef, *basel = *Basel;

	TREENODE *treeNew = NULL;
	FB *fbNew = NULL, *tfNode1 = NULL, *tfNode2 = NULL;
	int a = 0, b = 0, c = 0;
	int tusanjiao = 0;    //表示该三角是突三角型,到第二点时就不用再构造下去了
	float a1 = 0, b1 = 0, c1 = 0;
	TRIANGLE *tinNode = NULL, *tinSave = NULL, *t = NULL;
	//最左侧情况
	if (Point[i].x <= Point[frontf->no].x)   //i的x坐标小于前壳的first节点的x坐标
	{
		if (((Point[i].y - Point[frontf->no].y)*(Point[frontf->next->no].x - Point[frontf->no].x) <=                         //         /
			(Point[i].x - Point[frontf->no].x)*(Point[frontf->next->no].y - Point[frontf->no].y)) &&						   //	  \  /
			((Point[frontf->no].y - Point[frontf->next->no].y) / (Point[frontf->no].x - Point[frontf->next->no].x) <= 0))       //      \/
		{
			treeNew = new TREENODE;
			treeNew->no = i;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->back = treeNew->next = NULL;
			treeNew->parent = frontf;  frontf->lchild = treeNew;
			treeNew->next = frontf;  frontf->back = treeNew;
			AddNewTin(Point[i].ID, Point[basef->no].ID, Point[basef->next->no].ID, basef->tri, tinlast);
			//			AddNewTin(Point[i].ID, Point[basef->no].ID, Point[basef->next->no].ID, basef->tri, tinlast,Point);
			treeNew->tri = *tinlast;
			frontf = treeNew;

			fbNew = new FB; fbNew->no = i; //壳底的初步处理
			fbNew->tri = *tinlast;
			fbNew->back = NULL;		fbNew->next = NULL;
			fbNew->next = basef->next;
			basef->next->back = fbNew;
			basef = fbNew;           //注意：断开的指针应该删除，释放内存
			tfNode1 = basef->next;
			tfNode2 = tfNode1->next; //壳底三角形的处理
			if (tfNode2 != NULL)
			{
				a = i;  b = tfNode1->no;   c = tfNode2->no;
				tinNode = tfNode1->tri;
				//如果点c在直线ab的右侧,循环继续
				while ((Point[b].y - Point[a].y)*(Point[c].x - Point[a].x)>(Point[c].y - Point[a].y)*(Point[b].x - Point[a].x))
				{
					AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, basef->tri, tinlast);
					//AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, basef->tri, tinlast,Point);
					AddNewPointer(tinNode, *tinlast);
					basef->next = tfNode2;   basef->tri = *tinlast;
					tfNode2->back = basef;
					if (tfNode2->next == NULL)
					{
						*Frontf = frontf; *Frontl = frontl;
						*Basef = basef;    *Basel = basel;
						return;
					}
					else
					{
						tfNode1 = tfNode2;  tfNode2 = tfNode2->next;
						b = tfNode1->no;  c = tfNode2->no;
						tinNode = tfNode1->tri;
					}
				}
			}
		}
		else
		{
			tinNode = frontf->tri;
			AddNewTin(Point[i].ID, Point[frontf->no].ID, Point[frontf->next->no].ID, tinNode, tinlast);
			//AddNewTin(Point[i].ID, Point[frontf->no].ID, Point[frontf->next->no].ID, tinNode, tinlast,Point);
			tinSave = *tinlast;
			treeNew = new TREENODE;
			treeNew->no = i;  treeNew->tri = *tinlast;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->back = treeNew->next = NULL;
			treeNew->next = frontf->next; frontf->next->back = treeNew;
			frontf->lchild = treeNew;  treeNew->parent = frontf;
			TreeDelete(frontf, frontf->no, Root);
			frontf = treeNew;

			for (TREENODE *r = frontf->next; ((r->next != NULL) && (tusanjiao == 0)); r = r->next)
			{
				a1 = (Point[i].x - Point[r->no].x)*(Point[i].x - Point[r->no].x) + (Point[i].y - Point[r->no].y)*(Point[i].y - Point[r->no].y);
				b1 = (Point[r->next->no].x - Point[r->no].x)*(Point[r->next->no].x - Point[r->no].x) + (Point[r->next->no].y - Point[r->no].y)*(Point[r->next->no].y - Point[r->no].y);
				c1 = (Point[i].x - Point[r->next->no].x)*(Point[i].x - Point[r->next->no].x) + (Point[i].y - Point[r->next->no].y)*(Point[i].y - Point[r->next->no].y);
				if ((a1 + b1 - c1) / (2.0*(sqrt(a1*b1)))>0)
				{
					tinNode = *tinlast;
					t = r->tri;
					if (r->next->next != NULL)
						if ((Point[r->next->next->no].ID == t->ID1) || (Point[r->next->next->no].ID == t->ID2) || (Point[r->next->next->no].ID == t->ID3))
							tusanjiao = 1;
					AddNewTin(Point[i].ID, Point[r->no].ID, Point[r->next->no].ID, t, tinlast);
					//AddNewTin(Point[i].ID, Point[r->no].ID,Point[r->next->no].ID, t, tinlast,Point);
					AddNewPointer(tinNode, *tinlast);
					frontf->tri = *tinlast;
					frontf->next = r->next;
					r->next->back = frontf;
					TreeDelete(r, r->no, Root);
				}
				else
					break;
			}
			//壳底的初步处理
			//*****************************************************************
			if ((Point[basef->next->no].x - Point[i].x)*(Point[basef->no].y - Point[i].y)<(Point[basef->no].x - Point[i].x)*(Point[basef->next->no].y - Point[i].y))
			{
				fbNew = new FB; fbNew->no = i;
				fbNew->tri = tinSave;
				fbNew->back = NULL; fbNew->next = NULL;
				fbNew->next = basef;
				basef->back = fbNew;
				basef = fbNew;
				*Frontf = frontf, *Frontl = frontl;
				*Basef = basef, *Basel = basel;
				return;
			}
			AddNewTin(Point[i].ID, Point[basef->no].ID, Point[basef->next->no].ID, tinSave, tinlast);
			//AddNewTin(Point[i].ID, Point[basef->no].ID, Point[basef->next->no].ID, tinSave, tinlast,Point);
			AddNewPointer(basef->tri, *tinlast);

			fbNew = new FB;  fbNew->no = i;
			fbNew->tri = *tinlast;
			fbNew->back = NULL; fbNew->next = NULL;
			fbNew->next = basef->next;
			basef->next->back = fbNew;
			basef = fbNew;           //注意：断开的指针应该删除，释放内存
			tfNode1 = basef->next; tfNode2 = tfNode1->next; //壳底三角形的处理
			if (tfNode2 != NULL)
			{
				a = i;  b = tfNode1->no;  c = tfNode2->no;  t = *tinlast;
				tinNode = tfNode1->tri;
				while ((Point[b].y - Point[a].y)*(Point[c].x - Point[a].x)>(Point[c].y - Point[a].y)*(Point[b].x - Point[a].x))
				{
					AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, t, tinlast);
					//AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, t, tinlast,Point);
					AddNewPointer(tinNode, *tinlast);

					t = *tinlast;
					basef->next = tfNode2; basef->tri = *tinlast;
					tfNode2->back = basef;
					if (tfNode2->next == NULL)
					{
						*Frontf = frontf, *Frontl = frontl;
						*Basef = basef, *Basel = basel;
						return;
					}
					else
					{
						tfNode1 = tfNode2;   tfNode2 = tfNode2->next;
						b = tfNode1->no;  c = tfNode2->no;
						tinNode = tfNode1->tri;
					}
				}
			}
		}
	}
	//最右侧,情况
	else if (Point[i].x >= Point[frontl->no].x)
	{
		if (((Point[i].y - Point[frontl->no].y)*(Point[frontl->back->no].x - Point[frontl->no].x) >=
			(Point[i].x - Point[frontl->no].x)*(Point[frontl->back->no].y - Point[frontl->no].y)) &&
			((Point[frontl->no].y - Point[frontl->back->no].y) / (Point[frontl->no].x - Point[frontl->back->no].x) >= 0))
		{
			treeNew = new TREENODE;
			treeNew->no = i;  treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->back = treeNew->next = NULL;
			treeNew->parent = frontl;   frontl->rchild = treeNew;
			treeNew->back = frontl;   frontl->next = treeNew;
			treeNew->tri = NULL;
			AddNewTin(Point[i].ID, Point[basel->no].ID, Point[basel->back->no].ID, basel->back->tri, tinlast);
			//AddNewTin(Point[i].ID, Point[basel->no].ID, Point[basel->back->no].ID, basel->back->tri, tinlast,Point);
			frontl->tri = *tinlast;
			frontl = treeNew;

			fbNew = new FB; fbNew->no = i; //壳底的初步处理
			fbNew->tri = NULL;
			fbNew->back = NULL; fbNew->next = NULL;
			fbNew->back = basel->back;
			basel->back->next = fbNew;
			basel->back->tri = *tinlast;
			basel = fbNew;
			tfNode1 = basel->back; tfNode2 = tfNode1->back; //壳底三角形的处理
			if (tfNode2 != NULL)
			{
				a = i;   b = tfNode1->no;   c = tfNode2->no;
				t = *tinlast;
				tinNode = tfNode2->tri;
				//点c在直线ab左边，循环继续		
				while ((Point[b].y - Point[a].y)*(Point[c].x - Point[a].x)<(Point[c].y - Point[a].y)*(Point[b].x - Point[a].x))
				{
					AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, t, tinlast);
					//AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, t, tinlast,Point);
					AddNewPointer(*tinlast, tinNode);
					t = *tinlast;
					basel->back = tfNode2; tfNode2->tri = *tinlast;
					tfNode2->next = basel;
					if (tfNode2->back == NULL)
					{
						*Frontf = frontf, *Frontl = frontl;
						*Basef = basef, *Basel = basel;
						return;
					}
					else
					{
						tfNode1 = tfNode2;  tfNode2 = tfNode2->back;  b = tfNode1->no;
						c = tfNode2->no; tinNode = tfNode2->tri;
					}
				}
			}
		}
		else
		{
			tinNode = frontl->back->tri;
			AddNewTin(Point[i].ID, Point[frontl->no].ID, Point[frontl->back->no].ID, tinNode, tinlast);
			//AddNewTin(Point[i].ID, Point[frontl->no].ID, Point[frontl->back->no].ID, tinNode,tinlast,Point);
			//AddTinScan(i,frontl->no,frontl->back->no,&t);
			tinSave = *tinlast;
			treeNew = new TREENODE;
			treeNew->no = i; treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->back = NULL; treeNew->next = NULL;
			frontl->back->tri = *tinlast;
			treeNew->back = frontl->back; frontl->back->next = treeNew;
			frontl->rchild = treeNew; treeNew->parent = frontl;
			TreeDelete(frontl, frontl->no, Root);
			frontl = treeNew;
			for (TREENODE *r = frontl->back; ((r->back != NULL) && (tusanjiao == 0)); r = r->back)
			{
				a1 = (Point[i].x - Point[r->no].x)*(Point[i].x - Point[r->no].x) + (Point[i].y - Point[r->no].y)*(Point[i].y - Point[r->no].y);
				b1 = (Point[r->back->no].x - Point[r->no].x)*(Point[r->back->no].x - Point[r->no].x) + (Point[r->back->no].y - Point[r->no].y)*(Point[r->back->no].y - Point[r->no].y);
				c1 = (Point[i].x - Point[r->back->no].x)*(Point[i].x - Point[r->back->no].x) + (Point[i].y - Point[r->back->no].y)*(Point[i].y - Point[r->back->no].y);
				if ((a1 + b1 - c1) / (2.0*(sqrt(a1*b1)))>0)
				{
					tinNode = r->tri;  t = r->back->tri;
					if (r->back->back != NULL)
						if ((Point[r->back->back->no].ID == t->ID1) || (Point[r->back->back->no].ID == t->ID2) || (Point[r->back->back->no].ID == t->ID3))
							tusanjiao = 1;
					AddNewTin(Point[i].ID, Point[r->no].ID, Point[r->back->no].ID, t, tinlast);
					//AddNewTin(Point[i].ID, Point[r->no].ID, Point[r->back->no].ID, t,tinlast,Point);
					t = *tinlast;
					AddNewPointer(tinNode, *tinlast);
					frontl->back = r->back;
					r->back->next = frontl;
					TreeDelete(r, r->no, Root);
					frontl->back->tri = *tinlast;
				}
				else
					break;
			}
			//以下是计算壳底的
			if ((Point[basel->back->no].x - Point[i].x)*(Point[basel->no].y - Point[i].y)>(Point[basel->no].x - Point[i].x)*(Point[basel->back->no].y - Point[i].y))
			{
				fbNew = new FB; fbNew->no = i;
				fbNew->tri = NULL;
				fbNew->back = NULL; fbNew->next = NULL;
				basel->tri = tinSave;
				basel->next = fbNew; fbNew->back = basel;
				basel = fbNew;
				*Frontf = frontf, *Frontl = frontl;
				*Basef = basef, *Basel = basel;
				return;
			}
			if (basel->back == NULL)
			{
				*Frontf = frontf, *Frontl = frontl;
				*Basef = basef, *Basel = basel;
				return;
			}
			AddNewTin(Point[i].ID, Point[basel->no].ID, Point[basel->back->no].ID, tinSave, tinlast);
			//AddNewTin(Point[i].ID, Point[basel->no].ID, Point[basel->back->no].ID, tinSave, tinlast,Point);
			AddNewPointer(*tinlast, basel->back->tri);

			fbNew = new FB;   fbNew->no = i;
			fbNew->tri = NULL;
			fbNew->back = NULL; fbNew->next = NULL;
			fbNew->back = basel->back;
			basel->back->next = fbNew;
			basel->back->tri = *tinlast;
			basel = fbNew;
			tfNode1 = basel->back; tfNode2 = tfNode1->back;
			if (tfNode2 != NULL)
			{
				a = i; b = tfNode1->no; c = tfNode2->no;
				tinNode = *tinlast;
				t = tfNode2->tri;
				while ((Point[b].y - Point[a].y)*(Point[c].x - Point[a].x)<(Point[c].y - Point[a].y)*(Point[b].x - Point[a].x))
				{
					AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, tinNode, tinlast);
					//AddNewTin(Point[a].ID, Point[b].ID, Point[c].ID, tinNode, tinlast,Point);
					AddNewPointer(*tinlast, t);
					tinNode = *tinlast;
					basel->back = tfNode2;
					tfNode2->tri = *tinlast;
					tfNode2->next = basel;
					if (tfNode2->back == NULL)
					{
						*Frontf = frontf, *Frontl = frontl;
						*Basef = basef, *Basel = basel;
						return;
					}
					else
					{
						tfNode1 = tfNode2;  tfNode2 = tfNode2->back;  b = tfNode1->no; c = tfNode2->no;
						t = tfNode2->tri;
					}
				}
			}
		}
	}
	*Frontf = frontf, *Frontl = frontl;
	*Basef = basef, *Basel = basel;
}

//情况1——新加入点的投影正好落到前壳上
//参数：i——节点编号
//		Point——点数据集；
//		p——二叉排序树的搜索到的节点
//		tinlast——三角网的尾指针
//		Root——二叉排序树的根节点
void CTINConstruction2View::Circs1(int i, MyPoint *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root)
{
	int flag = 0;       //flag=1表示参数p在i点后
	TRIANGLE *pp = NULL;
	TREENODE *lp = NULL, *rp = NULL, *t = NULL, *tfb = NULL, *r1 = NULL, *np = NULL;
	float a = 0, b = 0, c = 0;
	int tl = 0, tr = 0;
	if (Point[p->no].x>Point[i].x)//p在i点后
	{
		np = p; p = np->back; flag = 1;
	}
	else //p在i点前
	{
		np = p->next; flag = 0;
	}
	pp = p->tri;

	AddNewTin(Point[i].ID, Point[p->no].ID, Point[np->no].ID, pp, tinlast);
	//AddNewTin(Point[i].ID, Point[p->no].ID, Point[np->no].ID, pp,tinlast,Point);
	pp = *tinlast;
	lp = p;		rp = np;
	//
	if (flag == 0)
		TreeInsert(Point, p, pp, i, flag);
	else
		TreeInsert(Point, np, pp, i, flag);
	p->tri = pp;

	tfb = p->next;
	//-------------------更新前壳-------------------------------//
	//向前壳的后端更新
	for (t = lp; lp->back != NULL; lp = r1)
	{
		a = (Point[i].x - Point[lp->no].x)*(Point[i].x - Point[lp->no].x) + (Point[i].y - Point[lp->no].y)*(Point[i].y - Point[lp->no].y);
		b = (Point[lp->back->no].x - Point[lp->no].x)*(Point[lp->back->no].x - Point[lp->no].x) + (Point[lp->back->no].y - Point[lp->no].y)*(Point[lp->back->no].y - Point[lp->no].y);
		c = (Point[i].x - Point[lp->back->no].x)*(Point[i].x - Point[lp->back->no].x) + (Point[i].y - Point[lp->back->no].y)*(Point[i].y - Point[lp->back->no].y);
		if ((a + b - c) / (2.0*(sqrt(a*b)))>0)
		{
			if (PointUpLine(Point, lp->back->no, i, lp->no) == 1)
				break;
			t = t->back;
			AddNewTin(Point[i].ID, Point[lp->no].ID, Point[lp->back->no].ID, (t->tri), tinlast);
			//AddNewTin(Point[i].ID, Point[lp->no].ID, Point[lp->back->no].ID, (t->tri),tinlast,Point);
			lp->back->next = tfb;
			tfb->back = lp->back;
			r1 = lp->back;
			r1->tri = *tinlast;
			TreeDelete(lp, lp->no, Root);
			AddNewPointer(r1->tri, lp->tri);
		}
		else
			break;
	}
	tfb = p->next;
	//向前壳的前端更新
	for (t = rp; rp->next != NULL; t = rp = r1)
	{
		a = (Point[i].x - Point[rp->no].x)*(Point[i].x - Point[rp->no].x) + (Point[i].y - Point[rp->no].y)*(Point[i].y - Point[rp->no].y);
		b = (Point[rp->next->no].x - Point[rp->no].x)*(Point[rp->next->no].x - Point[rp->no].x) + (Point[rp->next->no].y - Point[rp->no].y)*(Point[rp->next->no].y - Point[rp->no].y);
		c = (Point[i].x - Point[rp->next->no].x)*(Point[i].x - Point[rp->next->no].x) + (Point[i].y - Point[rp->next->no].y)*(Point[i].y - Point[rp->next->no].y);
		if ((a + b - c) / (2.0*(sqrt(a*b)))>0)
		{
			if (PointUpLine(Point, i, rp->next->no, rp->no) == 1)
				break;
			AddNewTin(Point[i].ID, Point[rp->no].ID, Point[rp->next->no].ID, t->tri, tinlast);
			//AddNewTin(Point[i].ID, Point[rp->no].ID, Point[rp->next->no].ID, t->tri,tinlast,Point);
			rp->next->back = tfb;
			tfb->next = rp->next;
			r1 = rp->next;
			TreeDelete(rp, rp->no, Root);
			AddNewPointer(tfb->tri, *tinlast);
			tfb->tri = *tinlast;
		}
		else
			break;
	}
}
void CTINConstruction2View::Circs1(int i, MyPoint *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root, FB **basel, FB **baser)
{
	int flag = 0;       //flag=1表示参数p在i点后
	TRIANGLE *pp = NULL;
	TREENODE *lp = NULL, *rp = NULL, *t = NULL, *tfb = NULL, *r1 = NULL, *np = NULL;
	float a = 0, b = 0, c = 0;
	int tl = 0, tr = 0;
	if (Point[p->no].x>Point[i].x)//p在i点后
	{
		np = p; p = np->back; flag = 1;
	}
	else //p在i点前
	{
		np = p->next; flag = 0;
	}
	pp = p->tri;

	if (PointInCircle(PointData, Point[i].ID, pp->ID1, pp->ID2, pp->ID3)) //i点在pp的外接圆内，交换边
	{
		//创建一个新的三角形并加入到tin中，边的邻接关系未确定
		TRIANGLENODE *newTri = new TRIANGLENODE;
		newTri->back = NULL;  newTri->next = NULL;
		newTri->p1tin = NULL; newTri->p2tin = NULL; newTri->p3tin = NULL;
		newTri->ID1 = Point[i].ID;
		newTri->ID2 = Point[np->no].ID;
		if (Point[p->no].ID != pp->ID1&&Point[np->no].ID != pp->ID1)
		{
			newTri->ID3 = pp->ID1;
		}
		if (Point[p->no].ID != pp->ID2&&Point[np->no].ID != pp->ID2)
		{
			newTri->ID3 = pp->ID2;
		}
		if (Point[p->no].ID != pp->ID3&&Point[np->no].ID != pp->ID3)
		{
			newTri->ID3 = pp->ID3;
		}
		int SeqNum = (*tinlast)->g_SeqNum;
		newTri->g_SeqNum = ++SeqNum;
		(*tinlast)->next = newTri;
		newTri->back = (*tinlast);
		*tinlast = newTri;
		//修改pp的顶点
		pp->ID1 = Point[p->no].ID;
		pp->ID2 = Point[i].ID;
		pp->ID3 = newTri->ID3;
		//更新pp原有的邻接关系
		TRIANGLENODE *p1tin = pp->p1tin, *p2tin = pp->p2tin, *p3tin = pp->p3tin;
		if (p1tin != NULL)
		{
			int q1, q2, no;
			if (HaveSameEdgeScan(pp->ID1, pp->ID2, pp->ID3, p1tin->ID1, p1tin->ID2, p1tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p1tin->ID1, p1tin->ID2, p1tin->ID3);
				if (no == 1)  p1tin->p1tin = pp;
				else if (no == 2) p1tin->p2tin = pp;
				else if (no == 3) p1tin->p3tin = pp;
				no = TheNumber(q1, q2, pp->ID1, pp->ID2, pp->ID3);
				if (no == 1)  pp->p1tin = p1tin;
				else if (no == 2) pp->p2tin = p1tin;
				else if (no == 3) pp->p3tin = p1tin;
			}
			if (HaveSameEdgeScan(newTri->ID1, newTri->ID2, newTri->ID3, p1tin->ID1, p1tin->ID2, p1tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p1tin->ID1, p1tin->ID2, p1tin->ID3);
				if (no == 1)  p1tin->p1tin = newTri;
				else if (no == 2) p1tin->p2tin = newTri;
				else if (no == 3) p1tin->p3tin = newTri;
				no = TheNumber(q1, q2, newTri->ID1, newTri->ID2, newTri->ID3);
				if (no == 1)  newTri->p1tin = p1tin;
				else if (no == 2) newTri->p2tin = p1tin;
				else if (no == 3) newTri->p3tin = p1tin;
			}
		}
		if (p2tin != NULL)
		{
			int q1, q2, no;
			if (HaveSameEdgeScan(pp->ID1, pp->ID2, pp->ID3, p2tin->ID1, p2tin->ID2, p2tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p2tin->ID1, p2tin->ID2, p2tin->ID3);
				if (no == 1)  p2tin->p1tin = pp;
				else if (no == 2) p2tin->p2tin = pp;
				else if (no == 3) p2tin->p3tin = pp;
				no = TheNumber(q1, q2, pp->ID1, pp->ID2, pp->ID3);
				if (no == 1)  pp->p1tin = p2tin;
				else if (no == 2) pp->p2tin = p2tin;
				else if (no == 3) pp->p3tin = p2tin;
			}
			if (HaveSameEdgeScan(newTri->ID1, newTri->ID2, newTri->ID3, p2tin->ID1, p2tin->ID2, p2tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p2tin->ID1, p2tin->ID2, p2tin->ID3);
				if (no == 1)  p2tin->p1tin = newTri;
				else if (no == 2) p2tin->p2tin = newTri;
				else if (no == 3) p2tin->p3tin = newTri;
				no = TheNumber(q1, q2, newTri->ID1, newTri->ID2, newTri->ID3);
				if (no == 1)  newTri->p1tin = p2tin;
				else if (no == 2) newTri->p2tin = p2tin;
				else if (no == 3) newTri->p3tin = p2tin;
			}
		}
		if (p3tin != NULL)
		{
			int q1, q2, no;
			if (HaveSameEdgeScan(pp->ID1, pp->ID2, pp->ID3, p3tin->ID1, p3tin->ID2, p3tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p3tin->ID1, p3tin->ID2, p3tin->ID3);
				if (no == 1)  p3tin->p1tin = pp;
				else if (no == 2) p3tin->p2tin = pp;
				else if (no == 3) p3tin->p3tin = pp;
				no = TheNumber(q1, q2, pp->ID1, pp->ID2, pp->ID3);
				if (no == 1)  pp->p1tin = p3tin;
				else if (no == 2) pp->p2tin = p3tin;
				else if (no == 3) pp->p3tin = p3tin;
			}
			if (HaveSameEdgeScan(newTri->ID1, newTri->ID2, newTri->ID3, p3tin->ID1, p3tin->ID2, p3tin->ID3, &q1, &q2))
			{
				no = TheNumber(q1, q2, p3tin->ID1, p3tin->ID2, p3tin->ID3);
				if (no == 1)  p3tin->p1tin = newTri;
				else if (no == 2) p3tin->p2tin = newTri;
				else if (no == 3) p3tin->p3tin = newTri;
				no = TheNumber(q1, q2, newTri->ID1, newTri->ID2, newTri->ID3);
				if (no == 1)  newTri->p1tin = p3tin;
				else if (no == 2) newTri->p2tin = p3tin;
				else if (no == 3) newTri->p3tin = p3tin;
			}
		}
		//更新pp和newtri的邻接关系
		int q1, q2, no;
		if (HaveSameEdgeScan(pp->ID1, pp->ID2, pp->ID3, newTri->ID1, newTri->ID2, newTri->ID3, &q1, &q2))
		{
			no = TheNumber(q1, q2, newTri->ID1, newTri->ID2, newTri->ID3);
			if (no == 1)  newTri->p1tin = pp;
			else if (no == 2) newTri->p2tin = pp;
			else if (no == 3) newTri->p3tin = pp;
			no = TheNumber(q1, q2, pp->ID1, pp->ID2, pp->ID3);
			if (no == 1)  pp->p1tin = newTri;
			else if (no == 2) pp->p2tin = newTri;
			else if (no == 3) pp->p3tin = newTri;
		}
		if (flag == 0)
			TreeInsert(Point, p, newTri, i, flag);
		else
			TreeInsert(Point, np, newTri, i, flag);
		//处理底壳
		FB* fbl = *basel, *fbr = *baser;
		// 		while (fb!=NULL)
		// 		{
		// 			if (fb->no==p->no&&fb->next->no==p->)
		// 			{
		// 				fb->tri=pp;
		// 				fb->next->tri=newTri;
		// 			}
		// 			fb=fb->next;
		// 		}
		if (fbl->no == p->no)
		{
			fbl->tri = pp;
		}
		if (fbr->no == np->no)
		{
			fbr->back->tri = newTri;
		}
		lp = p;		rp = np;
		//		return;
	}
	else     //直接将i点和i点落到的边的两个顶点加入三角网
	{
		Circs1(i, Point, p, tinlast, Root);
	}

	tfb = p->next;
	//-------------------更新前壳-------------------------------//
	//向前壳的后端更新
	for (t = lp; lp->back != NULL; lp = r1)
	{
		a = (Point[i].x - Point[lp->no].x)*(Point[i].x - Point[lp->no].x) + (Point[i].y - Point[lp->no].y)*(Point[i].y - Point[lp->no].y);
		b = (Point[lp->back->no].x - Point[lp->no].x)*(Point[lp->back->no].x - Point[lp->no].x) + (Point[lp->back->no].y - Point[lp->no].y)*(Point[lp->back->no].y - Point[lp->no].y);
		c = (Point[i].x - Point[lp->back->no].x)*(Point[i].x - Point[lp->back->no].x) + (Point[i].y - Point[lp->back->no].y)*(Point[i].y - Point[lp->back->no].y);
		if ((a + b - c) / (2.0*(sqrt(a*b)))>0)
		{
			if (PointUpLine(Point, lp->back->no, i, lp->no) == 1)
				break;
			t = t->back;
			AddNewTin(Point[i].ID, Point[lp->no].ID, Point[lp->back->no].ID, (t->tri), tinlast);
			//AddNewTin(Point[i].ID, Point[lp->no].ID, Point[lp->back->no].ID, (t->tri),tinlast,Point);
			lp->back->next = tfb;
			tfb->back = lp->back;
			r1 = lp->back;
			r1->tri = *tinlast;
			TreeDelete(lp, lp->no, Root);
			AddNewPointer(r1->tri, lp->tri);
		}
		else
			break;
	}
	tfb = p->next;
	//向前壳的前端更新
	for (t = rp; rp->next != NULL; t = rp = r1)
	{
		a = (Point[i].x - Point[rp->no].x)*(Point[i].x - Point[rp->no].x) + (Point[i].y - Point[rp->no].y)*(Point[i].y - Point[rp->no].y);
		b = (Point[rp->next->no].x - Point[rp->no].x)*(Point[rp->next->no].x - Point[rp->no].x) + (Point[rp->next->no].y - Point[rp->no].y)*(Point[rp->next->no].y - Point[rp->no].y);
		c = (Point[i].x - Point[rp->next->no].x)*(Point[i].x - Point[rp->next->no].x) + (Point[i].y - Point[rp->next->no].y)*(Point[i].y - Point[rp->next->no].y);
		if ((a + b - c) / (2.0*(sqrt(a*b)))>0)
		{
			if (PointUpLine(Point, i, rp->next->no, rp->no) == 1)
				break;
			AddNewTin(Point[i].ID, Point[rp->no].ID, Point[rp->next->no].ID, t->tri, tinlast);
			//AddNewTin(Point[i].ID, Point[rp->no].ID, Point[rp->next->no].ID, t->tri,tinlast,Point);
			rp->next->back = tfb;
			tfb->next = rp->next;
			r1 = rp->next;
			TreeDelete(rp, rp->no, Root);
			AddNewPointer(tfb->tri, *tinlast);
			tfb->tri = *tinlast;
		}
		else
			break;
	}
}

//处理前壳，完成构建
void CTINConstruction2View::EndFrontLine(MyPoint *Point, TRIANGLE **tinlast, TREENODE **Frontf, TREENODE **Root)
{
	int finish = 0;		TREENODE *p = NULL;	TRIANGLE *t = NULL;
	TREENODE *frontf = *Frontf;
	do {
		finish = 1;
		for (p = frontf; p->next->next != NULL;)
		{
			//当中间点位于两端点的下方时，生成新三角形，更新前壳
			//修改p点后继点指针
			//继续检测，直到前壳的尾端
			if (PointUpLine(Point, p->no, p->next->next->no, p->next->no) == 0)
			{
				t = p->tri;
				AddNewTin(Point[p->next->next->no].ID, Point[p->next->no].ID, Point[p->no].ID, t, tinlast);
				//AddNewTin(Point[p->next->next->no].ID, Point[p->next->no].ID, Point[p->no].ID, t,tinlast,Point);
				AddNewPointer(*tinlast, p->next->tri);
				p->tri = *tinlast;
				finish = 0;
				TreeDelete(p->next, p->next->no, Root);
				p->next = p->next->next;
			}
			else
				p = p->next;
		}
	} while (finish == 0); //当检测到前壳有更新时，继续从头再次检测，直到检测不到有中间点位于两端点下方的情况
}
//LOP优化，交换边等操作
//t原始三角形，rear1邻接三角形链表，rear2为原始三角形链表节点，OriginalData为原始数据点集
void CTINConstruction2View::Lop_all(TRIANGLE **t, LIST **rear1, LIST *rear2, MyPoint *OriginalData)
{
	TRIANGLE *t4 = NULL;	int d;
	if (CorrespondingTriangle((*t)->ID1, (*t)->ID2, (*t)->ID3, &d, (*t), &t4))//取出相关三角形给t4保存
	{
		if (PointInCircle(OriginalData, d, (*t)->ID1, (*t)->ID2, (*t)->ID3)>0) //判断公共边所对的顶点是否在外接圆内
		{
			SetTinScan_all((*t)->ID1, d, (*t)->ID2, (*t)->ID3, &(*t), &t4, rear1, rear2); //交换边
			Lop_all(&(*t), rear1, rear2, OriginalData);	//递归调用，其中*t，rear1，rear2以得到更新
			return;
		}
	}
	if (CorrespondingTriangle((*t)->ID2, (*t)->ID3, (*t)->ID1, &d, (*t), &t4))
	{
		if (PointInCircle(OriginalData, d, (*t)->ID1, (*t)->ID2, (*t)->ID3)>0)
		{
			SetTinScan_all((*t)->ID2, d, (*t)->ID3, (*t)->ID1, &(*t), &t4, rear1, rear2);
			Lop_all(&(*t), rear1, rear2, OriginalData);
			return;
		}
	}
	if (CorrespondingTriangle((*t)->ID3, (*t)->ID1, (*t)->ID2, &d, (*t), &t4))
	{
		if (PointInCircle(OriginalData, d, (*t)->ID1, (*t)->ID2, (*t)->ID3)>0)
		{
			SetTinScan_all((*t)->ID3, d, (*t)->ID1, (*t)->ID2, &(*t), &t4, rear1, rear2);
			Lop_all(&(*t), rear1, rear2, OriginalData);
			return;
		}
	}
}
//相关三角形判断
//a,b,c分别为某三角形的三个顶点的编号
//t为原三角形
//t2为找到的相关三角形
int CTINConstruction2View::CorrespondingTriangle(int a, int b, int c, int *d, TRIANGLE *t, TRIANGLE **t2)
{
	TRIANGLE *tp = NULL;
	int no = TheNumber(b, c, a, b, c);   //ThuNumber函数首先将a,b,c三个参数从小到大排序
										 //然后将b,c与排序好的三个有序数相比
										 //与三个有序数前两个相等，则返回1，即b,c的值为a,b,c中两个较小的值
										 //与后两个相等，则返回2，即b,c为a,b,c中较大的两个值
										 //与前后两个相等,则返回3,即b,c为a,b,c中一大一下两个值
										 //程序首先要快速排序（安装y值从小到大排序），所以点的序号与大小直接相关

	if (no == 1)     //此时a的y值最大，即a在最上方
	{
		tp = t->p1tin;  //取出该三角形的第一个邻接三角形
		if (tp == NULL)
			return 0;
		*t2 = tp;
		//取出第一个邻接三角形与公共边相对的顶点，赋值给d
		if (tp->ID1 != b&&tp->ID1 != c)
		{
			*d = tp->ID1;
		}
		else if (tp->ID2 != b&&tp->ID2 != c)
		{
			*d = tp->ID2;
		}
		else if (tp->ID3 != b&&tp->ID3 != c)
		{
			*d = tp->ID3;
		}
		else
			AfxMessageBox("CorrespondingTriangle出错！");
		return 1;
	}
	else if (no == 2)
	{
		tp = t->p2tin;
		if (tp == NULL)
			return 0;
		*t2 = tp;
		if (tp->ID1 != b&&tp->ID1 != c)
		{
			*d = tp->ID1;
		}
		else if (tp->ID2 != b&&tp->ID2 != c)
		{
			*d = tp->ID2;
		}
		else if (tp->ID3 != b&&tp->ID3 != c)
		{
			*d = tp->ID3;
		}
		else
			AfxMessageBox("CorrespondingTriangle出错！");
		return 1;
	}
	else if (no == 3)
	{
		tp = t->p3tin;
		if (tp == NULL)
			return 0;
		*t2 = tp;
		if (tp->ID1 != b&&tp->ID1 != c)
		{
			*d = tp->ID1;
		}
		else if (tp->ID2 != b&&tp->ID2 != c)
		{
			*d = tp->ID2;
		}
		else if (tp->ID3 != b&&tp->ID3 != c)
		{
			*d = tp->ID3;
		}
		else
			AfxMessageBox("CorrespondingTriangle出错！");
		return 1;
	}
	else
	{
		AfxMessageBox("CorrespondingTriangle出错！");
		return 0;
	}
}
//空外接圆判断
//P0、P1、P2、P3为原始点集PointData中的序号
//P0为判断点
//P1、P2、P3为三角形的三个顶点
int CTINConstruction2View::PointInCircle(MyPoint *OriginalData, int P0, int P1, int P2, int P3)
{
	int isIn = -1;
	double x0, y0, R;   //x0，y0为圆心坐标，R为半径
	CircleBy3Points(OriginalData[P1], OriginalData[P2], OriginalData[P3], &x0, &y0, &R); //三点确定一个圆
	if (R - ((x0 - OriginalData[P0].x)*(x0 - OriginalData[P0].x) + (y0 - OriginalData[P0].y)*(y0 - OriginalData[P0].y))>0.0001)
		isIn = 1;
	return isIn;
}
//交换边等操作
//a,b,c为三角形的顶点
//d为需要邻接三角形的顶点
//t为三角形
//t1为邻接三角形
//rear1，rear2
void CTINConstruction2View::SetTinScan_all(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1, LIST **rear1, LIST *rear2)
{
	int no;
	TRIANGLE *abTin = NULL, *acTin = NULL, *dbTin = NULL, *dcTin = NULL;//相当于保存两个三角形所构成的四边形的边
																		//边交换之前，首先保留四条边所对应的邻接三角形信息
	no = TheNumber(a, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  abTin = (*t)->p1tin;
	else if (no == 2) abTin = (*t)->p2tin;
	else if (no == 3)  abTin = (*t)->p3tin;

	no = TheNumber(a, c, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  acTin = (*t)->p1tin;
	else if (no == 2) acTin = (*t)->p2tin;
	else if (no == 3)  acTin = (*t)->p3tin;

	no = TheNumber(d, b, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  dbTin = (*t1)->p1tin;
	else if (no == 2) dbTin = (*t1)->p2tin;
	else if (no == 3)  dbTin = (*t1)->p3tin;

	no = TheNumber(d, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  dcTin = (*t1)->p1tin;
	else if (no == 2) dcTin = (*t1)->p2tin;
	else if (no == 3)  dcTin = (*t1)->p3tin;

	//交换边
	(*t)->ID1 = a, (*t)->ID2 = b, (*t)->ID3 = d;
	(*t1)->ID1 = a, (*t1)->ID2 = c, (*t1)->ID3 = d;

	//边交换后维护邻接三角形的信息
	no = TheNumber(a, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = abTin;
	else if (no == 2) (*t)->p2tin = abTin;
	else if (no == 3)  (*t)->p3tin = abTin;
	if (abTin != NULL)
	{
		no = TheNumber(a, b, abTin->ID1, abTin->ID2, abTin->ID3);
		if (no == 1)  abTin->p1tin = *t;
		else if (no == 2) abTin->p2tin = *t;
		else if (no == 3)  abTin->p3tin = *t;
	}

	no = TheNumber(d, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = dbTin;
	else if (no == 2) (*t)->p2tin = dbTin;
	else if (no == 3)  (*t)->p3tin = dbTin;
	if (dbTin != NULL)
	{
		no = TheNumber(d, b, dbTin->ID1, dbTin->ID2, dbTin->ID3);
		if (no == 1)  dbTin->p1tin = *t;
		else if (no == 2) dbTin->p2tin = *t;
		else if (no == 3)  dbTin->p3tin = *t;
	}

	no = TheNumber(a, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = acTin;
	else if (no == 2) (*t1)->p2tin = acTin;
	else if (no == 3)  (*t1)->p3tin = acTin;
	if (acTin != NULL)
	{
		no = TheNumber(a, c, acTin->ID1, acTin->ID2, acTin->ID3);
		if (no == 1)  acTin->p1tin = *t1;
		else if (no == 2) acTin->p2tin = *t1;
		else if (no == 3)  acTin->p3tin = *t1;
	}

	no = TheNumber(d, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = dcTin;
	else if (no == 2) (*t1)->p2tin = dcTin;
	else if (no == 3)  (*t1)->p3tin = dcTin;
	if (dcTin != NULL)
	{
		no = TheNumber(d, c, dcTin->ID1, dcTin->ID2, dcTin->ID3);
		if (no == 1)  dcTin->p1tin = *t1;
		else if (no == 2) dcTin->p2tin = *t1;
		else if (no == 3)  dcTin->p3tin = *t1;
	}

	no = TheNumber(a, d, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = *t1;
	else if (no == 2) (*t)->p2tin = *t1;
	else if (no == 3)  (*t)->p3tin = *t1;
	no = TheNumber(a, d, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = *t;
	else if (no == 2) (*t1)->p2tin = *t;
	else if (no == 3)  (*t1)->p3tin = *t;

	if ((*rear1) != NULL)   //用于非递归的扫描法
	{
		LIST *tp = new LIST;       //将交换过的三角形加入到邻接三角形链表中
		tp->tri = *t1;
		tp->tri->visited = -1;
		(*rear1)->next = tp;
		tp->next = rear2;
		*rear1 = (*rear1)->next;
	}
}
void CTINConstruction2View::EdgeChange(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1)
{
	int no;
	TRIANGLE *abTin = NULL, *acTin = NULL, *dbTin = NULL, *dcTin = NULL;//相当于保存两个三角形所构成的四边形的边
																		//边交换之前，首先保留四条边所对应的邻接三角形信息
	no = TheNumber(a, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  abTin = (*t)->p1tin;
	else if (no == 2) abTin = (*t)->p2tin;
	else if (no == 3)  abTin = (*t)->p3tin;

	no = TheNumber(a, c, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  acTin = (*t)->p1tin;
	else if (no == 2) acTin = (*t)->p2tin;
	else if (no == 3)  acTin = (*t)->p3tin;

	no = TheNumber(d, b, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  dbTin = (*t1)->p1tin;
	else if (no == 2) dbTin = (*t1)->p2tin;
	else if (no == 3)  dbTin = (*t1)->p3tin;

	no = TheNumber(d, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  dcTin = (*t1)->p1tin;
	else if (no == 2) dcTin = (*t1)->p2tin;
	else if (no == 3)  dcTin = (*t1)->p3tin;

	//交换边
	(*t)->ID1 = a, (*t)->ID2 = b, (*t)->ID3 = d;
	(*t1)->ID1 = a, (*t1)->ID2 = c, (*t1)->ID3 = d;

	//边交换后维护邻接三角形的信息
	no = TheNumber(a, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = abTin;
	else if (no == 2) (*t)->p2tin = abTin;
	else if (no == 3)  (*t)->p3tin = abTin;
	if (abTin != NULL)
	{
		no = TheNumber(a, b, abTin->ID1, abTin->ID2, abTin->ID3);
		if (no == 1)  abTin->p1tin = *t;
		else if (no == 2) abTin->p2tin = *t;
		else if (no == 3)  abTin->p3tin = *t;
	}

	no = TheNumber(d, b, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = dbTin;
	else if (no == 2) (*t)->p2tin = dbTin;
	else if (no == 3)  (*t)->p3tin = dbTin;
	if (dbTin != NULL)
	{
		no = TheNumber(d, b, dbTin->ID1, dbTin->ID2, dbTin->ID3);
		if (no == 1)  dbTin->p1tin = *t;
		else if (no == 2) dbTin->p2tin = *t;
		else if (no == 3)  dbTin->p3tin = *t;
	}

	no = TheNumber(a, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = acTin;
	else if (no == 2) (*t1)->p2tin = acTin;
	else if (no == 3)  (*t1)->p3tin = acTin;
	if (acTin != NULL)
	{
		no = TheNumber(a, c, acTin->ID1, acTin->ID2, acTin->ID3);
		if (no == 1)  acTin->p1tin = *t1;
		else if (no == 2) acTin->p2tin = *t1;
		else if (no == 3)  acTin->p3tin = *t1;
	}

	no = TheNumber(d, c, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = dcTin;
	else if (no == 2) (*t1)->p2tin = dcTin;
	else if (no == 3)  (*t1)->p3tin = dcTin;
	if (dcTin != NULL)
	{
		no = TheNumber(d, c, dcTin->ID1, dcTin->ID2, dcTin->ID3);
		if (no == 1)  dcTin->p1tin = *t1;
		else if (no == 2) dcTin->p2tin = *t1;
		else if (no == 3)  dcTin->p3tin = *t1;
	}

	no = TheNumber(a, d, (*t)->ID1, (*t)->ID2, (*t)->ID3);
	if (no == 1)  (*t)->p1tin = *t1;
	else if (no == 2) (*t)->p2tin = *t1;
	else if (no == 3)  (*t)->p3tin = *t1;
	no = TheNumber(a, d, (*t1)->ID1, (*t1)->ID2, (*t1)->ID3);
	if (no == 1)  (*t1)->p1tin = *t;
	else if (no == 2) (*t1)->p2tin = *t;
	else if (no == 3)  (*t1)->p3tin = *t;
}
//三点确定一个圆
void CTINConstruction2View::CircleBy3Points(MyPoint P1, MyPoint P2, MyPoint P3, double * ox, double * oy, double * RR)
{
	double a1, b1, c1, a2, b2, c2;//xc,yc;
								  //先处理三点共线
	a1 = (P1.y - P2.y); a2 = (P1.x - P2.x);
	b1 = (P3.y - P2.y); b2 = (P3.x - P2.x);
	if (a2*b1 == a1*b2)
	{
		*oy = 0.0; *ox = 0.0; *RR = 0.0;
	}
	else
	{
		a1 = 2 * (P1.x - P2.x); b1 = 2 * (P1.y - P2.y); c1 = (P1.x*P1.x + P1.y*P1.y) - (P2.x*P2.x + P2.y*P2.y);
		a2 = 2 * (P2.x - P3.x); b2 = 2 * (P2.y - P3.y); c2 = (P2.x*P2.x + P2.y*P2.y) - (P3.x*P3.x + P3.y*P3.y);
		*oy = (a2*c1 - a1*c2) / (a2*b1 - a1*b2 + 1.E-8);
		*ox = (b1*c2 - b2*c1) / (a2*b1 - a1*b2 + 1.E-8);
		*RR = (P1.x - *ox)*(P1.x - *ox) + (P1.y - *oy)*(P1.y - *oy);
	}
}

///////扫描线法生成TIN///////////////////////////
//参数：Data——点数据集；pNum——数据集大小
//		tinfirst——三角网的头指针；tinlast——三角网的尾指针
//		root，frontf，frontl——二叉排序树，存储前壳
//		basef，basel——双向链表，存储底壳
//		g_pointmark——前壳最高点的标记
void CTINConstruction2View::CreateTinBy_Scan(MyPoint *Point, int pNum, TRIANGLE **tinlast, int &g_pointmark, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel)
{
	TREENODE *r1 = NULL, *r2 = NULL;  //r2就是r1在X方向的后一个节点
	for (int i = g_pointmark + 1; i<pNum; i++)
	{
		if (((Point[i].x == Point[(*frontf)->no].x) && (Point[i].y == Point[(*frontf)->no].y)) ||
			((Point[i].x == Point[(*frontl)->no].x) && (Point[i].y == Point[(*frontl)->no].y)))
			;                        //点和前壳头点或尾点重合，则什么也不用做
		else if ((Point[i].x <= Point[(*frontf)->no].x) || (Point[i].x >= Point[(*frontl)->no].x))  //不与前线相交
																									//情况2——投影未落到前壳上
		{
			Circs2(i, Point, tinlast, root, frontf, frontl, basef, basel);
			//AfxMessageBox("circs2"); 
		}
		else
		{
			TreeSearch(Point, root, &r1, i);
			if (Point[r1->no].x>Point[i].x)
				r2 = r1->back;
			else
				r2 = r1->next;
			if (((Point[i].x == Point[r1->no].x) && (Point[i].y == Point[r1->no].y)) ||
				(Point[i].x == Point[r2->no].x) && (Point[i].y == Point[r2->no].y))
				;                //与端点重合，不做处理
			else if (((Point[i].x >= Point[r1->no].x) && (Point[i].x<Point[r2->no].x)) ||
				((Point[i].x >= Point[r2->no].x) && (Point[i].x<Point[r1->no].x)))
			{
				//情况1
				Circs1(i, Point, r1, tinlast, root);
				//Circs1(i, Point, r1, tinlast,root,basef,basel);//与前线相交
				//AfxMessageBox("circs1");
			}
		}
	}
}

// 按照y坐标排序
int cmp(const void *a, const void *b)
{
	MyPoint *c = (MyPoint *)a;
	MyPoint *d = (MyPoint *)b;
	if (c->y != d->y)
	{
		if (c->y - d->y>0)
			return 1;
		else
			return -1;
	}
	else
	{
		if (c->x - d->x>0)
			return 1;
		else if (c->x - d->x<0)
			return -1;
		else
			return 0;
	}
}
//LOP优化算法
void CTINConstruction2View::LOPScan_Nonrecursion(TRIANGLE *p, TRIANGLE **tinfirst, MyPoint *OriginalData)
{
	LIST  *tp1 = NULL, *tp2 = NULL;	//定义记录三角形的链表节点
									//tp1保存链表头
									//tp2保存动态加入的邻接三角形

	LIST *front = NULL, *rear1 = NULL, *rear2 = NULL;	//front保存的是原三角形链表
														//rear1保存的是邻接三角形的链表
														//rear2保存的实际是一个节点
														//该节点保存了第一个三角形（tinfirst），next指向null
														//邻接三角形链表的末尾都指向该节点，作用大概是，递归的出口，即next指向null

	rear2 = new LIST;	rear2->tri = *tinfirst;  rear2->next = NULL;
	front = new LIST;	front->tri = p; p->visited = 1;	front->next = rear2;
	rear1 = front;
	while (front->next != NULL)
	{
		tp1 = front;    //tp1为链表头
		if (tp1->tri->p1tin != NULL)  //邻接三角形存在
		{
			if (tp1->tri->p1tin->visited != 1) //且未被检测过
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p1tin;
				tp2->tri->visited = 1;
				rear1->next = tp2;      //将邻接三角形加入到rear1中
				tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		if (tp1->tri->p2tin != NULL)
		{
			if (tp1->tri->p2tin->visited != 1)
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p2tin;
				tp2->tri->visited = 1;
				rear1->next = tp2; tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		if (tp1->tri->p3tin != NULL)
		{
			if (tp1->tri->p3tin->visited != 1)
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p3tin;
				tp2->tri->visited = 1;
				rear1->next = tp2; tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		front = front->next;
		//将第一个三角形的邻接三角形全部找到，并加入到链表rear1中
		//然后在从第一个三角形开始，从邻接三角形表的最后一个三角形开始进行优化
		Lop_all(&(tp1->tri), &rear1, rear2, OriginalData);
		delete tp1;
	}
	delete rear2;
	//	delete front;
}

//初始化扫描线
//参数：Data——点数据集；pNum——数据集大小
//		tinfirst——三角网的头指针；tinlast——三角网的尾指针
//		root，frontf，frontl——二叉排序树，存储前壳
//		basef，basel——双向链表，存储底壳
//		g_pointmark——记录第一个与前两个点不共线的点
void CTINConstruction2View::SetInitialScan(MyPoint *Point, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, int &g_pointmark, TREENODE **Root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel)
{
	int i;
	TRIANGLE *tinNode;
	TREENODE *treeNew, *root;
	FB *fbNew;    //底壳
				  //构造第一个三角形，即找出与前两个点不共线的第一个点
	for (i = 2; i<pNum; i++)
		if (((Point[i].y - Point[i - 1].y)*(Point[i].x - Point[0].x)) != ((Point[i].x - Point[i - 1].x)*(Point[i].y - Point[0].y)))
			break;                   //遇到与前两个点不共线的点，停止
	g_pointmark = i;
	//Point数组按y值从小到大排序，初始共线的几点按x值从大到小排序 
	if (i>2)
		qs_x(Point, 0, i - 1); //以x坐标排序
	//---------------构建前壳和底壳，下面分情况讨论--------------
	root = new TREENODE;
	//7-d
	if ((Point[i - 1].x<Point[0].x) && (Point[i].x <= Point[i - 1].x))
	{
		if ((Point[i - 1].x - Point[0].x)*(Point[i].y - Point[0].y)<(Point[i].x - Point[0].x)*(Point[i - 1].y - Point[0].y))
			//点i-1在直线0、i的左侧
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID); //产生第一个三角形
			tinNode = *tinfirst;
			for (int t = 2; t<i; t++)
				FirAddTin(Point[i].ID, Point[t - 1].ID, Point[t].ID, &tinNode, tinlast); //加入共线的点

			//-------------------------初始化前壳------------------------------//
			//二叉排序树的根节点初始化
			root->no = i;             //记录前壳最高点的序号 
			root->tri = *tinfirst;    //与前壳共边的三角形指向三角网第一个三角形
			root->lchild = root->rchild = root->parent = NULL;  //左右孩子及父节点指向NULL

			treeNew = new TREENODE;
			treeNew->no = 0;    treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

			root->rchild = treeNew;  treeNew->parent = root;

			*frontf = root;  *frontl = treeNew;
			root->next = treeNew;  treeNew->back = root;
			root->back = NULL; treeNew->next = NULL;
			//-------------------------初始化底壳------------------------------//
			fbNew = new FB;  fbNew->no = i;  fbNew->next = NULL;  fbNew->back = NULL;
			fbNew->tri = *tinlast;

			*basef = *basel = fbNew;
			tinNode = *tinlast;  //记录与底壳共边的三角形
			for (int m = i - 1; m>0; m--)
			{
				fbNew = new FB;  fbNew->no = m;  fbNew->next = NULL;  fbNew->back = NULL;
				fbNew->tri = tinNode;
				(*basel)->next = fbNew; fbNew->back = *basel;  *basel = fbNew;
				tinNode = tinNode->back;
			}
			fbNew = new FB;  fbNew->no = 0;  fbNew->next = NULL;  fbNew->back = NULL;
			fbNew->tri = NULL;
			(*basel)->next = fbNew;  fbNew->back = *basel;
			*basel = fbNew;
		}
		else //点i-1在直线0、i的右侧
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
			tinNode = *tinfirst;
			for (int t = 2; t<i; t++)
				FirAddTin(Point[i].ID, Point[t - 1].ID, Point[t].ID, &tinNode, tinlast);

			root->no = i; root->tri = *tinlast;
			root->lchild = root->rchild = root->parent = NULL;
			root->back = NULL;

			*frontf = *frontl = root;  tinNode = *tinlast;
			for (int m = i - 1; m>0; m--)
			{
				treeNew = new TREENODE;
				treeNew->no = m;  treeNew->tri = tinNode;
				treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

				(*frontl)->rchild = treeNew; treeNew->parent = *frontl;
				(*frontl)->next = treeNew; treeNew->back = *frontl;

				*frontl = treeNew;   tinNode = tinNode->back;
			}
			treeNew = new TREENODE;
			treeNew->no = 0;  treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			(*frontl)->rchild = treeNew;  treeNew->parent = *frontl;
			(*frontl)->next = treeNew;  treeNew->back = *frontl;  treeNew->next = NULL;
			*frontl = treeNew;

			fbNew = new FB;  fbNew->no = i; fbNew->next = NULL;  fbNew->back = NULL;
			fbNew->tri = *tinfirst;
			*basef = *basel = fbNew;
			fbNew = new FB;  fbNew->no = 0;	fbNew->next = NULL; fbNew->back = NULL;
			fbNew->tri = NULL;
			(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;
		}
	}
	//7-c
	else if ((Point[i - 1].x<Point[0].x) && (Point[i].x>Point[i - 1].x) && (Point[i].x<Point[0].x))
	{
		AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
		tinNode = *tinfirst;
		for (int t = 2; t<i; t++)
			FirAddTin(Point[i].ID, Point[t - 1].ID, Point[t].ID, &tinNode, tinlast);

		root->no = i; root->tri = *tinfirst;
		root->lchild = root->rchild = root->parent = NULL;

		treeNew = new TREENODE;
		treeNew->no = 0;  treeNew->tri = NULL;
		treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

		root->rchild = treeNew;  treeNew->parent = root;
		treeNew->next = NULL;  treeNew->back = root;  root->next = treeNew;

		*frontl = treeNew;
		treeNew = new TREENODE;
		treeNew->no = i - 1; treeNew->tri = *tinlast;
		treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
		root->lchild = treeNew;  treeNew->parent = root;
		treeNew->back = NULL; treeNew->next = root; root->back = treeNew;
		*frontf = treeNew;

		//壳底处理
		fbNew = new FB; fbNew->no = i - 1; fbNew->next = NULL; fbNew->back = NULL;
		fbNew->tri = *tinlast;
		*basef = *basel = fbNew;   tinNode = (*tinlast)->back;
		for (int m = i - 2; m >= 0; m--)
		{
			fbNew = new FB;  fbNew->no = m;  fbNew->next = NULL; fbNew->back = NULL;
			fbNew->tri = tinNode;

			(*basel)->next = fbNew;  fbNew->back = *basel;  *basel = fbNew;
			if (tinNode == NULL) break;
			tinNode = tinNode->back;
		}
	}
	//7-a
	else if ((Point[i - 1].x <= Point[0].x) && (Point[i].x >= Point[0].x))
	{
		if ((Point[0].x - Point[i - 1].x)*(Point[i].y - Point[i - 1].y)>(Point[i].x - Point[i - 1].x)*(Point[0].y - Point[i - 1].y))
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
			tinNode = *tinfirst;
			for (int t = 2; t<i; t++)
				FirAddTin(Point[i].ID, Point[t - 1].ID, Point[t].ID, &tinNode, tinlast);

			root->no = i - 1; root->tri = *tinlast;
			root->lchild = root->rchild = root->parent = NULL;

			treeNew = new TREENODE;
			treeNew->no = i; treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

			root->rchild = treeNew;  treeNew->parent = root;
			root->back = NULL; root->next = treeNew;
			treeNew->back = root; treeNew->next = NULL;
			*frontf = root; *frontl = treeNew;

			fbNew = new FB; fbNew->no = i - 1; fbNew->next = NULL; fbNew->back = NULL;
			fbNew->tri = *tinlast;
			*basef = *basel = fbNew; tinNode = (*tinlast)->back;
			for (int m = i - 2; m>0; m--)
			{
				fbNew = new FB;  fbNew->no = m;  fbNew->next = NULL; fbNew->back = NULL;
				fbNew->tri = tinNode;
				tinNode = tinNode->back;
				(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;
			}
			fbNew = new FB;  fbNew->no = 0;  fbNew->next = NULL; fbNew->back = NULL;
			fbNew->tri = *tinfirst;
			(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;

			fbNew = new FB; fbNew->no = i; fbNew->next = NULL; fbNew->back = NULL;
			fbNew->tri = NULL;
			(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;
		}
		else if ((g_pointmark>2) && ((Point[0].x - Point[i - 1].x)*(Point[i].y - Point[i - 1].y)<(Point[i].x - Point[i - 1].x)*(Point[0].y - Point[i - 1].y)))
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
			tinNode = *tinfirst;
			for (int t = 2; t<i; t++)
				FirAddTin(Point[i].ID, Point[t - 1].ID, Point[t].ID, &tinNode, tinlast);

			root->no = 0; root->tri = *tinfirst;
			root->lchild = root->rchild = root->parent = NULL;
			root->back = root->next = NULL;

			treeNew = new TREENODE;
			treeNew->no = i;  treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->back = treeNew->next = NULL;

			root->rchild = treeNew;  treeNew->parent = root;
			root->next = treeNew;  treeNew->back = root;
			*frontl = treeNew;
			*frontf = root; tinNode = *tinfirst;
			for (int m = 1; m <= i - 1; m++)
			{
				treeNew = new TREENODE;
				treeNew->no = m; treeNew->tri = tinNode;
				treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
				treeNew->back = treeNew->next = NULL;

				(*frontf)->lchild = treeNew; treeNew->parent = *frontf;
				treeNew->next = *frontf; treeNew->back = treeNew;
				*frontf = treeNew; tinNode = tinNode->next;
			}
			//处理壳底
			fbNew = new FB;
			fbNew->back = fbNew->next = NULL;
			fbNew->no = i - 1;  fbNew->tri = *tinlast;
			*basef = *basel = fbNew;
			fbNew = new FB;
			fbNew->no = i; fbNew->back = fbNew->next = NULL;
			fbNew->tri = NULL;
			(*basef)->next = fbNew; fbNew->back = *basel;
			*basel = fbNew;
		}
	}
	//7-e
	else if ((Point[i - 1].x>Point[0].x) && (Point[i].x >= Point[i - 1].x))
	{
		//点1在直线0、2的右侧
		if ((Point[i].y - Point[0].y)*(Point[i - 1].x - Point[0].x)>(Point[i].x - Point[0].x)*(Point[i - 1].y - Point[0].y))
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
			root->no = 0; root->tri = *tinfirst;
			root->lchild = root->rchild = root->parent = NULL;

			treeNew = new TREENODE;
			treeNew->no = i;  treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

			root->rchild = treeNew;  treeNew->parent = root;
			root->back = NULL;  root->next = treeNew;
			treeNew->back = root;  treeNew->next = NULL;

			*frontf = root;  *frontl = treeNew;

			fbNew = new FB;  fbNew->no = 0; fbNew->next = NULL; fbNew->back = NULL; 	fbNew->tri = *tinfirst;
			*basef = *basel = fbNew;
			fbNew = new FB;  fbNew->no = 1; fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = *tinfirst;
			(*basel)->next = fbNew;  fbNew->back = *basel;
			*basel = fbNew;
			fbNew = new FB;  fbNew->no = 2; fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = NULL;
			(*basel)->next = fbNew;  fbNew->back = *basel;
			*basel = fbNew;
		}
		else //点1在直线0、2的左侧
		{
			AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
			root->no = 0;  root->tri = *tinfirst;
			root->lchild = root->rchild = root->parent = NULL;
			root->back = NULL;
			*frontf = *frontl = root;

			treeNew = new TREENODE;
			treeNew->no = 1;   treeNew->tri = *tinfirst;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->next = treeNew->back = NULL;
			root->rchild = treeNew; treeNew->parent = root;
			root->next = treeNew; treeNew->back = root;
			*frontl = treeNew;

			treeNew = new TREENODE;
			treeNew->no = 2; treeNew->tri = NULL;
			treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
			treeNew->next = treeNew->back = NULL;

			(*frontl)->rchild = treeNew; treeNew->parent = *frontl;
			(*frontl)->next = treeNew; treeNew->back = *frontl;
			*frontl = treeNew;

			fbNew = new FB;  fbNew->no = 0;   fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = *tinfirst;
			*basef = *basel = fbNew;
			fbNew = new FB; fbNew->no = 2;  fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = NULL;
			(*basel)->next = fbNew; fbNew->back = *basel;  *basel = fbNew;
		}
	}
	//7-f
	else if ((Point[i - 1].x>Point[0].x) && (Point[i].x<Point[i - 1].x) && (Point[i].x>Point[0].x))
	{
		AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
		root->no = i; root->tri = *tinfirst;
		root->lchild = root->rchild = root->parent = NULL;

		treeNew = new TREENODE;
		treeNew->no = i - 1;  treeNew->tri = NULL;
		treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
		root->rchild = treeNew;  treeNew->parent = root;
		*frontl = treeNew;
		root->next = treeNew;  treeNew->back = root;  treeNew->next = NULL;

		treeNew = new TREENODE;  treeNew->no = 0;  treeNew->tri = *tinfirst;
		treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;
		root->lchild = treeNew;  treeNew->parent = root;
		root->back = treeNew;  treeNew->next = root;  treeNew->back = NULL;
		*frontf = treeNew;

		fbNew = new FB; fbNew->no = 0; fbNew->next = NULL; fbNew->back = NULL;	fbNew->tri = *tinfirst;
		*basef = *basel = fbNew;
		fbNew = new FB; fbNew->no = 1; fbNew->next = NULL; fbNew->back = NULL;	 fbNew->tri = NULL;
		(*basel)->next = fbNew;  fbNew->back = *basel;  *basel = fbNew;
	}
	//6-b
	else if ((Point[i - 1].x >= Point[0].x) && (Point[i].x <= Point[0].x))
	{
		AddFirstTin(tinfirst, tinlast, Point[0].ID, Point[i].ID, Point[1].ID);
		root->no = i; root->tri = *tinfirst;
		root->lchild = root->rchild = root->parent = NULL;

		treeNew = new TREENODE;
		treeNew->no = i - 1; treeNew->tri = NULL;
		treeNew->lchild = treeNew->rchild = treeNew->parent = NULL;

		root->rchild = treeNew;  treeNew->parent = root;
		root->back = NULL;  root->next = treeNew;
		treeNew->back = root;  treeNew->next = NULL;
		*frontf = root;  *frontl = treeNew;

		fbNew = new FB; fbNew->no = 2; fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = *tinfirst;
		*basef = *basel = fbNew;

		fbNew = new FB; fbNew->no = 0; fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = *tinfirst;
		(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;

		fbNew = new FB; fbNew->no = 1; fbNew->next = NULL; fbNew->back = NULL; fbNew->tri = NULL;
		(*basel)->next = fbNew; fbNew->back = *basel; *basel = fbNew;
	}
	*Root = root;
}
//判断点P 在线段P1-P2的哪一侧? (-1:左侧; 1: 右侧)
int CTINConstruction2View::OnLeft(MyPoint P, MyPoint P1, MyPoint P2)
{
	if ((P2.x - P1.x)*(P.y - P1.y) - (P2.y - P1.y)*(P.x - P1.x)>0)
		return -1;
	else if ((P2.x - P1.x)*(P.y - P1.y) - (P2.y - P1.y)*(P.x - P1.x)<0)
		return 1;
	else
		return 0;
}
void CTINConstruction2View::SetTriInfor(TRIANGLE* tri, TRIANGLE *blockTri, int e1, int e2)
{
	if (tri != NULL)
	{
		int no = TheNumber(e1, e2, tri->ID1, tri->ID2, tri->ID3);
		if (no == -1)
		{
			int no1;
			if (tri->p1tin != NULL)
			{
				no1 = TheNumber(e1, e2, tri->p1tin->ID1, tri->p1tin->ID2, tri->p1tin->ID3);
				if (no1 != -1)
				{
					blockTri = tri->p1tin;
					return;
				}
			}
			if (tri->p2tin != NULL)
			{
				no1 = TheNumber(e1, e2, tri->p2tin->ID1, tri->p2tin->ID2, tri->p2tin->ID3);
				if (no1 != -1)
				{
					blockTri = tri->p2tin;
					return;
				}
			}
			if (tri->p3tin != NULL)
			{
				no1 = TheNumber(e1, e2, tri->p3tin->ID1, tri->p3tin->ID2, tri->p3tin->ID3);
				if (no1 != -1)
				{
					blockTri = tri->p3tin;
					return;
				}
			}
			return;
		}
		else
		{
			blockTri = tri;
			return;
		}
	}
}
//BlockTIN生成后维护BlockTIN的数据信息
void CTINConstruction2View::GetCurrentBlockInfor(BlockTin &blocktin, TRIANGLE *TinSave, MyPoint *Point, TREENODE *frontf, FB *basef)
{
	int no;
	//探测前壳对应的三角形的边
	for (TREENODE *f = frontf; f->next != NULL&& f != NULL; f = f->next)
	{
		no = TheNumber(Point[f->no].ID, Point[f->next->no].ID, TinSave->ID1, TinSave->ID2, TinSave->ID3);
		if (no == 1)
		{
			blocktin.Tin[no] = f->tri;
			//SetTriInfor(f->tri,blocktin.Tin[no],Point[f->no].ID, Point[f->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[f->no].ID; blocktin.Edge[no].ID2 = Point[f->next->no].ID;
		}
		else if (no == 2)
		{
			blocktin.Tin[no] = f->tri;
			//SetTriInfor(f->tri,blocktin.Tin[no],Point[f->no].ID, Point[f->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[f->no].ID; blocktin.Edge[no].ID2 = Point[f->next->no].ID;
		}
		else if (no == 3)
		{
			blocktin.Tin[no] = f->tri;
			//SetTriInfor(f->tri,blocktin.Tin[no],Point[f->no].ID, Point[f->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[f->no].ID; blocktin.Edge[no].ID2 = Point[f->next->no].ID;
		}
	}
	//探测底壳对应的三角形的边
	for (FB *b = basef; b->next != NULL&& b != NULL; b = b->next)
	{
		no = TheNumber(Point[b->no].ID, Point[b->next->no].ID, TinSave->ID1, TinSave->ID2, TinSave->ID3);
		if (no == 1)
		{
			blocktin.Tin[no] = b->tri;
			//SetTriInfor(b->tri,blocktin.Tin[no],Point[b->no].ID, Point[b->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[b->no].ID; blocktin.Edge[no].ID2 = Point[b->next->no].ID;
		}
		else if (no == 2)
		{
			blocktin.Tin[no] = b->tri;
			//SetTriInfor(b->tri,blocktin.Tin[no],Point[b->no].ID, Point[b->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[b->no].ID; blocktin.Edge[no].ID2 = Point[b->next->no].ID;
		}
		else if (no == 3)
		{
			blocktin.Tin[no] = b->tri;
			//SetTriInfor(b->tri,blocktin.Tin[no],Point[b->no].ID, Point[b->next->no].ID);
			blocktin.IsVisited[no] = false;
			blocktin.Edge[no].ID1 = Point[b->no].ID; blocktin.Edge[no].ID2 = Point[b->next->no].ID;
		}
	}
}
void CTINConstruction2View::SetBlockPointer(TRIANGLE *Tbig1, BlockTin *block)
{
	TRIANGLE *Tbig2; //用来标记块之间的邻接三角形
	TRIANGLE *Tsmall1, *Tsmall2; //用来标记块内部的小三角形
	Tbig2 = Tsmall1 = Tsmall2 = NULL;
	int no, e1, e2;
	if (Tbig1 == NULL) return;
	//以下为递归的出口：
	//某一条边为空，其他边都被访问过
	//或者三条边都不为空，但都被访问过
	else if (Tbig1->p1tin == NULL && block[Tbig1->g_SeqNum].IsVisited[2] == true && block[Tbig1->g_SeqNum].IsVisited[3] == true)
		return;
	else if (Tbig1->p2tin == NULL && block[Tbig1->g_SeqNum].IsVisited[1] == true && block[Tbig1->g_SeqNum].IsVisited[3] == true)
		return;
	else if (Tbig1->p3tin == NULL && block[Tbig1->g_SeqNum].IsVisited[1] == true && block[Tbig1->g_SeqNum].IsVisited[2] == true)
		return;
	else if (Tbig1->p1tin != NULL && Tbig1->p2tin != NULL && Tbig1->p3tin != NULL && block[Tbig1->g_SeqNum].IsVisited[1] == true && block[Tbig1->g_SeqNum].IsVisited[2] == true && block[Tbig1->g_SeqNum].IsVisited[3] == true)
		return;
	//第一条边
	if (Tbig1->p1tin != NULL && block[Tbig1->g_SeqNum].IsVisited[1] != true)
	{
		block[Tbig1->g_SeqNum].IsVisited[1] = true; //标记该三角形块中的外围三角形的第一条边被访问过
		e1 = block[Tbig1->g_SeqNum].Edge[1].ID1;
		e2 = block[Tbig1->g_SeqNum].Edge[1].ID2;    //取出该条边的两个顶点
		Tsmall1 = block[Tbig1->g_SeqNum].Tin[1];    //取出该三角形块中与第一条边相对应的三角形

		Tbig2 = Tbig1->p1tin;     //取出该三角形块的邻接三角形（与第一条边相对应）
		no = TheNumber(e1, e2, Tbig2->ID1, Tbig2->ID2, Tbig2->ID3);//计算邻接关系

		if (no == 1)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[1];
			block[Tbig2->g_SeqNum].IsVisited[1] = true;
		}
		else if (no == 2)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[2];
			block[Tbig2->g_SeqNum].IsVisited[2] = true;
		}
		else if (no == 3)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[3];
			block[Tbig2->g_SeqNum].IsVisited[3] = true;
		}
		//建立Tsmall1、Tsmall2之间的邻接指针
		no = TheNumber(e1, e2, Tsmall1->ID1, Tsmall1->ID2, Tsmall1->ID3);
		if (no == 1) Tsmall1->p1tin = Tsmall2;
		else if (no == 2) Tsmall1->p2tin = Tsmall2;
		else if (no == 3) Tsmall1->p3tin = Tsmall2;
		no = TheNumber(e1, e2, Tsmall2->ID1, Tsmall2->ID2, Tsmall2->ID3);
		if (no == 1) Tsmall2->p1tin = Tsmall1;
		else if (no == 2) Tsmall2->p2tin = Tsmall1;
		else if (no == 3) Tsmall2->p3tin = Tsmall1;
	}

	//第二条边
	else if (Tbig1->p2tin != NULL && block[Tbig1->g_SeqNum].IsVisited[2] != true)
	{
		block[Tbig1->g_SeqNum].IsVisited[2] = true;
		e1 = block[Tbig1->g_SeqNum].Edge[2].ID1;
		e2 = block[Tbig1->g_SeqNum].Edge[2].ID2;
		Tsmall1 = block[Tbig1->g_SeqNum].Tin[2];

		Tbig2 = Tbig1->p2tin;
		no = TheNumber(e1, e2, Tbig2->ID1, Tbig2->ID2, Tbig2->ID3);
		if (no == 1)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[1];
			block[Tbig2->g_SeqNum].IsVisited[1] = true;
		}
		else if (no == 2)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[2];
			block[Tbig2->g_SeqNum].IsVisited[2] = true;
		}
		else if (no == 3)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[3];
			block[Tbig2->g_SeqNum].IsVisited[3] = true;
		}
		//建立Tsmall1、Tsmall2之间的邻接指针
		no = TheNumber(e1, e2, Tsmall1->ID1, Tsmall1->ID2, Tsmall1->ID3);
		if (no == 1) Tsmall1->p1tin = Tsmall2;
		else if (no == 2) Tsmall1->p2tin = Tsmall2;
		else if (no == 3) Tsmall1->p3tin = Tsmall2;
		no = TheNumber(e1, e2, Tsmall2->ID1, Tsmall2->ID2, Tsmall2->ID3);
		if (no == 1) Tsmall2->p1tin = Tsmall1;
		else if (no == 2) Tsmall2->p2tin = Tsmall1;
		else if (no == 3) Tsmall2->p3tin = Tsmall1;
	}
	//第三条边
	else if (Tbig1->p3tin != NULL && block[Tbig1->g_SeqNum].IsVisited[3] != true)
	{
		block[Tbig1->g_SeqNum].IsVisited[3] = true;
		e1 = block[Tbig1->g_SeqNum].Edge[3].ID1;
		e2 = block[Tbig1->g_SeqNum].Edge[3].ID2;
		Tsmall1 = block[Tbig1->g_SeqNum].Tin[3];

		Tbig2 = Tbig1->p3tin;
		no = TheNumber(e1, e2, Tbig2->ID1, Tbig2->ID2, Tbig2->ID3);
		if (no == 1)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[1];
			block[Tbig2->g_SeqNum].IsVisited[1] = true;
		}
		else if (no == 2)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[2];
			block[Tbig2->g_SeqNum].IsVisited[2] = true;
		}
		else if (no == 3)
		{
			Tsmall2 = block[Tbig2->g_SeqNum].Tin[3];
			block[Tbig2->g_SeqNum].IsVisited[3] = true;
		}
		//建立Tsmall1、Tsmall2之间的邻接指针
		no = TheNumber(e1, e2, Tsmall1->ID1, Tsmall1->ID2, Tsmall1->ID3);
		if (no == 1) Tsmall1->p1tin = Tsmall2;
		else if (no == 2) Tsmall1->p2tin = Tsmall2;
		else if (no == 3) Tsmall1->p3tin = Tsmall2;
		no = TheNumber(e1, e2, Tsmall2->ID1, Tsmall2->ID2, Tsmall2->ID3);
		if (no == 1) Tsmall2->p1tin = Tsmall1;
		else if (no == 2) Tsmall2->p2tin = Tsmall1;
		else if (no == 3) Tsmall2->p3tin = Tsmall1;
	}
	//依次对邻接的三个三角形进行递归调用，完成对整个区域的三角形连接
	SetBlockPointer(Tbig1->p1tin, block);
	SetBlockPointer(Tbig1->p2tin, block);
	SetBlockPointer(Tbig1->p3tin, block);
}
//生成三角网
//参数：Data——点数据集；pNum——数据集大小
//		tinfirst——三角网的头指针；tinlast——三角网的尾指针
//		root，frontf，frontl——二叉排序树，存储前壳
//		basef，basel——双向链表，存储底壳
void CTINConstruction2View::GeneDelaunay(MyPoint *Data, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel)
{
	qsort(Data, pNum, sizeof(Data[0]), cmp);   //快速排序
	int g_pointmark = 0;
	//初始化扫描线
	SetInitialScan(Data, pNum, tinfirst, tinlast, g_pointmark, root, frontf, frontl, basef, basel);
	//生成TIN
	CreateTinBy_Scan(Data, pNum, tinlast, g_pointmark, root, frontf, frontl, basef, basel);
	//处理前壳
	EndFrontLine(Data, tinlast, frontf, root);
}
void CTINConstruction2View::DrawBlockTin(TRIANGLE *tin, MyPoint *OriginalData)
{
	CDC *pDC = GetDC();
	static int flag = 0;
	CPen  NewPen;
	if (flag != 0)
	{
		srand((unsigned)time(NULL));
		int i = rand() % 14;  //[0, 13]
		NewPen.CreatePen(PS_SOLID, 1, colors[i]);
	}
	else
		NewPen.CreatePen(PS_SOLID, 2, colors[0]);
	CPen *OldPen = pDC->SelectObject(&NewPen);
	for (TRIANGLE *T = tin; T != NULL; T = T->next)
	{
		PNT P1 = { OriginalData[T->ID1].x,OriginalData[T->ID1].y };
		PNT P2 = { OriginalData[T->ID2].x,OriginalData[T->ID2].y };
		PNT P3 = { OriginalData[T->ID3].x,OriginalData[T->ID3].y };
		GetScreenPoint(&P1); GetScreenPoint(&P2); GetScreenPoint(&P3);
		pDC->MoveTo(P1.x, P1.y);
		pDC->LineTo(P2.x, P2.y);

		pDC->MoveTo(P1.x, P1.y);
		pDC->LineTo(P3.x, P3.y);

		pDC->MoveTo(P3.x, P3.y);
		pDC->LineTo(P2.x, P2.y);
	}

	pDC->SelectObject(OldPen);
	flag++;
	ReleaseDC(pDC);
}
void CTINConstruction2View::DrawTin(CDC *pDC, MyPoint *OriginalData, COLOR PRGB)
{
	if (OriginalData == NULL) {
		return;
	}
	CPen  NewPen;
	NewPen.CreatePen(PS_SOLID, 1, colors[PRGB]);
	CPen *OldPen = pDC->SelectObject(&NewPen);
	CBrush br;
	br.CreateSolidBrush(colors[YELLO]);
	CBrush *pOldBrush = pDC->SelectObject(&br);
	// 绘制最短路径三角形
	for (TRIANGLE *tri = pEndTri; tri != NULL; tri = tri->parentTri) {
		DrawTriangle(pDC, tri);
	}

	//int pNum = 0;
	for (TRIANGLE *T = tinHead; T != NULL; T = T->next)
	{
		PNT P1 = { OriginalData[T->ID1].x,OriginalData[T->ID1].y };
		PNT P2 = { OriginalData[T->ID2].x,OriginalData[T->ID2].y };
		PNT P3 = { OriginalData[T->ID3].x,OriginalData[T->ID3].y };
		GetScreenPoint(&P1); GetScreenPoint(&P2); GetScreenPoint(&P3);
		//pDC->BeginPath();
		pDC->MoveTo(P1.x, P1.y);
		pDC->LineTo(P2.x, P2.y);
		// 		CString ss1;
		// 		ss1.Format("%d",T->ID1);
		// 		pDC->TextOut(P1.x,P1.y,ss1);
		pDC->MoveTo(P2.x, P2.y);
		pDC->LineTo(P3.x, P3.y);
		// 		CString ss2;
		// 		ss2.Format("%d",T->ID2);
		// 		pDC->TextOut(P2.x,P2.y,ss2);
		pDC->MoveTo(P3.x, P3.y);
		pDC->LineTo(P1.x, P1.y);
		// 		CString ss3;
		// 		ss3.Format("%d",T->ID3);
		// 		pDC->TextOut(P3.x,P3.y,ss3);
		//pDC->EndPath();
		//pDC->FillPath();
		CString str;
		str.Format("%d", T->g_SeqNum);
		pDC->TextOut((P1.x + P2.x + P3.x) / 3, (P1.y + P2.y + P3.y) / 3, str);
	}
	
	pDC->SelectObject(OldPen);
	pDC->SelectObject(&pOldBrush);
}

void CTINConstruction2View::DrawTriangle(CDC *pDC, TRIANGLE *T) {
	PNT P1 = { PointData[T->ID1].x,PointData[T->ID1].y };
	PNT P2 = { PointData[T->ID2].x,PointData[T->ID2].y };
	PNT P3 = { PointData[T->ID3].x,PointData[T->ID3].y };
	GetScreenPoint(&P1); GetScreenPoint(&P2); GetScreenPoint(&P3);
	pDC->BeginPath();
	pDC->MoveTo(P1.x, P1.y);
	pDC->LineTo(P2.x, P2.y);
	pDC->LineTo(P3.x, P3.y);
	pDC->LineTo(P1.x, P1.y);
	pDC->EndPath();
	pDC->FillPath();
}

///////初始化自适应分组后格网中心点集///////////////
void CTINConstruction2View::InitialCenterMyPoint(MyPoint *Data, int &DataSize, MyPoint *OriginalData, long &OriginalSize)
{
	//原始点集初始化，将外围四个顶点加入点集
	//外围四个顶点向外扩展了20倍长宽，目的是为了去除顶点时，不影响内部三角网
	OriginalData[OriginalSize].x = dXmin - 20 * dXdelt; OriginalData[OriginalSize].y = dYmin - 20 * dYdelt;
	OriginalData[OriginalSize].ID = OriginalSize;  OriginalData[OriginalSize].label = true;
	OriginalSize++;
	OriginalData[OriginalSize].x = dXmax + 20 * dXdelt; OriginalData[OriginalSize].y = dYmin - 20 * dYdelt;
	OriginalData[OriginalSize].ID = OriginalSize;  OriginalData[OriginalSize].label = true;
	OriginalSize++;
	OriginalData[OriginalSize].x = dXmax + 20 * dXdelt; OriginalData[OriginalSize].y = dYmax + 20 * dYdelt;
	OriginalData[OriginalSize].ID = OriginalSize;  OriginalData[OriginalSize].label = true;
	OriginalSize++;
	OriginalData[OriginalSize].x = dXmin - 20 * dXdelt; OriginalData[OriginalSize].y = dYmax + 20 * dYdelt;
	OriginalData[OriginalSize].ID = OriginalSize;  OriginalData[OriginalSize].label = true;
	OriginalSize++;
	//将外围四个顶点加入中心点集	
	int Index = OriginalSize - 4;
	Data[DataSize].x = dXmin - dXdelt; Data[DataSize].y = dYmin - dYdelt;  Data[DataSize].ID = Index++;
	DataSize++;
	Data[DataSize].x = dXmax + dXdelt; Data[DataSize].y = dYmin - dYdelt;  Data[DataSize].ID = Index++;
	DataSize++;
	Data[DataSize].x = dXmax + dXdelt; Data[DataSize].y = dYmax + dYdelt;  Data[DataSize].ID = Index++;
	DataSize++;
	Data[DataSize].x = dXmin - dXdelt; Data[DataSize].y = dYmax + dYdelt;  Data[DataSize].ID = Index++;
	DataSize++;
}

//////////////////////////////////////////////////////////////////////////
int CTINConstruction2View::OnLeft(double x, double y, MyPoint P1, MyPoint P2)
{
	if ((P2.x - P1.x)*(y - P1.y) - (P2.y - P1.y)*(x - P1.x)>0)
		return -1;
	else if ((P2.x - P1.x)*(y - P1.y) - (P2.y - P1.y)*(x - P1.x)<0)
		return 1;
	else
		return 0;
}
//得到各个三角形block中所含的blockgrid信息
void CTINConstruction2View::GetBlockGridInfo()
{
	//起点坐标
	double xStart = xmin - BlockGridOffset, yStart = ymin - BlockGridOffset;
	//终点坐标
	double xEnd = xmax + BlockGridOffset, yEnd = ymax + BlockGridOffset;
	//每一小格网的长、宽
	double xcol_wide = (xEnd - xStart) / (BlockGridSize);
	double yrow_wide = (yEnd - yStart) / (BlockGridSize);
	double x1, y1, x2, y2, x3, y3, x4, y4;


	for (int i = 0; i<BlockGridSize; i++)
	{
		for (int j = 0; j<BlockGridSize; j++)
		{
			x1 = xStart + j*xcol_wide; y1 = yStart + i*yrow_wide;
			x2 = xStart + (j + 1)*xcol_wide; y2 = yStart + i*yrow_wide;
			x3 = xStart + j*xcol_wide; y3 = yStart + (i + 1)*yrow_wide;
			x4 = xStart + (j + 1)*xcol_wide; y4 = yStart + (i + 1)*yrow_wide;
			blockGrid[i][j].triangleNum = 0;
			blockGrid[i][j].triFirst = NULL;
			for (TRIANGLE *t = tinHead; t != NULL; t = t->next)
			{
				//在同一个三角形的点的个数
				int sameTriNum = 0;
				if (OnLeft(x1, y1, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x1, y1, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x1, y1, PointData[t->ID3], PointData[t->ID1]) == 3
					|| OnLeft(x1, y1, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x1, y1, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x1, y1, PointData[t->ID3], PointData[t->ID1]) == -3
					)	//判断点是否在三角形内
					sameTriNum++;
				if (OnLeft(x2, y2, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x2, y2, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x2, y2, PointData[t->ID3], PointData[t->ID1]) == 3
					|| OnLeft(x2, y2, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x2, y2, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x2, y2, PointData[t->ID3], PointData[t->ID1]) == -3
					)	//判断点是否在三角形内
					sameTriNum++;
				if (OnLeft(x3, y3, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x3, y3, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x3, y3, PointData[t->ID3], PointData[t->ID1]) == 3
					|| OnLeft(x3, y3, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x3, y3, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x3, y3, PointData[t->ID3], PointData[t->ID1]) == -3
					)	//判断点是否在三角形内
					sameTriNum++;
				if (OnLeft(x4, y4, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x4, y4, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x4, y4, PointData[t->ID3], PointData[t->ID1]) == 3
					|| OnLeft(x4, y4, PointData[t->ID1], PointData[t->ID2]) + OnLeft(x4, y4, PointData[t->ID2], PointData[t->ID3]) + OnLeft(x4, y4, PointData[t->ID3], PointData[t->ID1]) == -3
					)	//判断点是否在三角形内
					sameTriNum++;
				if (sameTriNum == 0)
				{
					continue;
				}
				else if (sameTriNum == 4)
				{
					blockGrid[i][j].triangleNum++;
					TRIANGLENODE *tri = new TRIANGLENODE;
					tri->next = NULL; tri->g_SeqNum = t->g_SeqNum; tri->ID1 = t->ID1; tri->ID2 = t->ID2; tri->ID3 = t->ID3;
					blockGrid[i][j].triFirst = tri;
					break;
				}
				else
				{
					blockGrid[i][j].triangleNum++;
					TRIANGLENODE *tri = new TRIANGLENODE;
					switch (blockGrid[i][j].triangleNum)
					{
					case 1:

						tri->next = NULL; tri->g_SeqNum = t->g_SeqNum; tri->ID1 = t->ID1; tri->ID2 = t->ID2; tri->ID3 = t->ID3;
						blockGrid[i][j].triFirst = tri;
						break;
					case 2:

						tri->next = NULL; tri->g_SeqNum = t->g_SeqNum; tri->ID1 = t->ID1; tri->ID2 = t->ID2; tri->ID3 = t->ID3;
						blockGrid[i][j].triFirst->next = tri;
						break;
					case 3:

						tri->next = NULL; tri->g_SeqNum = t->g_SeqNum; tri->ID1 = t->ID1; tri->ID2 = t->ID2; tri->ID3 = t->ID3;
						blockGrid[i][j].triFirst->next->next = tri;
						break;
					case 4:

						tri->next = NULL; tri->g_SeqNum = t->g_SeqNum; tri->ID1 = t->ID1; tri->ID2 = t->ID2; tri->ID3 = t->ID3;
						blockGrid[i][j].triFirst->next->next->next = tri;
						break;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////


///////////////生成TIN的主函数////////////////////
/*************************************************/
//void CTINConstruction2View::OnGenerateTin()
//{
//	clock_t start, finish, startTotal, finishTotal;
//
//	if (PointCenter == NULL)
//	{
//		AfxMessageBox(_T("请先进行自适应分组！"));
//		return;
//	}
//	startTotal = start = clock();
//	FBNODE *basef = NULL, *basel = NULL; //双向链表，存储底壳
//	TREENODE *root = NULL, *frontf = NULL, *frontl = NULL; //二叉排序树，存储前壳
//
//	int i = 0;
//	//初始化中心点数据，将外围四个顶点数据加入到数据集中
//	//参数PointCenter，中心点数组，CenterSize ，数组大小
//	//    PointData  ，原始点数据，pointNumber，原始点数据大小
//	InitialCenterMyPoint(PointCenter, CenterSize, PointData, pointNumber);
//	//根据中心点生成三角网
//	GeneDelaunay(PointCenter, CenterSize, &tinHead, &tinEnd, &root, &frontf, &frontl, &basef, &basel);
//	//清除内存
//	ClearPartionMemory(frontf, basef);
//	//LOP优化
//	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
//
//	BlockTotal = tinEnd->g_SeqNum + 1;
//	int *gpNum = new int[BlockTotal];
//	for (i = 0; i<BlockTotal; i++)
//		gpNum[i] = 0;  //保存每一块内的点数
//	GetBlockGridInfo();
//	//////////////////////////////////////////////////////////////////////////
//	//起点坐标
//	double xStart = xmin - BlockGridOffset, yStart = ymin - BlockGridOffset;
//	//终点坐标
//	double xEnd = xmax + BlockGridOffset, yEnd = ymax + BlockGridOffset;
//	//每一小格网的长、宽
//	double xcol_wide = (xEnd - xStart) / (BlockGridSize);
//	double yrow_wide = (yEnd - yStart) / (BlockGridSize);
//	for (i = 0; i<pointNumber - 4; i++)      //遍历每个点，判断其所在的三角形
//	{
//		if (PointData[i].label == true)
//		{
//			PointData[i].gNo = -1;
//			continue;
//		}
//		int col = (int)((PointData[i].x - xStart) / xcol_wide);
//		int row = (int)((PointData[i].y - yStart) / yrow_wide);
//		if (blockGrid[row][col].triangleNum == 1)
//		{
//			PointData[i].gNo = blockGrid[row][col].triFirst->g_SeqNum;
//			gpNum[PointData[i].gNo]++;
//		}
//		else if (blockGrid[row][col].triangleNum == 0)
//		{
//			PointData[i].gNo = -1;
//		}
//		else
//		{
//			bool find = false;
//			//优先在block相关的三角形中查找
//			for (TRIANGLE *t = blockGrid[row][col].triFirst; t != NULL; t = t->next)
//			{
//				if (OnLeft(PointData[i], PointData[t->ID1], PointData[t->ID2]) + OnLeft(PointData[i], PointData[t->ID2], PointData[t->ID3]) + OnLeft(PointData[i], PointData[t->ID3], PointData[t->ID1]) == 3
//					|| OnLeft(PointData[i], PointData[t->ID1], PointData[t->ID2]) + OnLeft(PointData[i], PointData[t->ID2], PointData[t->ID3]) + OnLeft(PointData[i], PointData[t->ID3], PointData[t->ID1]) == -3
//					)	//判断点是否在三角形内
//				{
//					PointData[i].gNo = t->g_SeqNum;
//					gpNum[PointData[i].gNo]++;
//					find = true;
//					break;
//				}
//			}
//			if (find == false)
//			{
//				TRIANGLE *t = tinHead;
//				for (; t != NULL; t = t->next)
//				{
//					if (OnLeft(PointData[i], PointData[t->ID1], PointData[t->ID2]) + OnLeft(PointData[i], PointData[t->ID2], PointData[t->ID3]) + OnLeft(PointData[i], PointData[t->ID3], PointData[t->ID1]) == 3
//						|| OnLeft(PointData[i], PointData[t->ID1], PointData[t->ID2]) + OnLeft(PointData[i], PointData[t->ID2], PointData[t->ID3]) + OnLeft(PointData[i], PointData[t->ID3], PointData[t->ID1]) == -3
//						)	//判断点是否在三角形内
//					{
//						PointData[i].gNo = t->g_SeqNum;
//						gpNum[PointData[i].gNo]++;
//						break;
//					}
//				}
//				if (t == NULL)
//				{
//					PointData[i].gNo = -1;
//				}
//			}
//		}
//	}
//	blockTin = new BlockTin[BlockTotal];
//	//用指针数组来存储链表中的对应三角形，便于检索
//
//	TinSave = (TRIANGLE**)malloc(sizeof(*TinSave) * BlockTotal);
//	i = 0;
//	for (TRIANGLE *t = tinHead; t != NULL; t = t->next, i++)
//		TinSave[i] = t;
//	for (i = 0; i<BlockTotal; i++)
//	{
//		blockTin[i].pNum = gpNum[i] + 3;   //三角形内点数+三角形三个顶点
//		blockTin[i].pts = new MyPoint[blockTin[i].pNum];  //存储点集的数组
//		int pT = blockTin[i].pNum;
//		////////以下将三角形的三个顶点放到数组的最后////////////
//		blockTin[i].pts[pT - 3].x = PointData[TinSave[i]->ID1].x; blockTin[i].pts[pT - 3].y = PointData[TinSave[i]->ID1].y; blockTin[i].pts[pT - 3].ID = PointData[TinSave[i]->ID1].ID;
//		blockTin[i].pts[pT - 2].x = PointData[TinSave[i]->ID2].x; blockTin[i].pts[pT - 2].y = PointData[TinSave[i]->ID2].y; blockTin[i].pts[pT - 2].ID = PointData[TinSave[i]->ID2].ID;
//		blockTin[i].pts[pT - 1].x = PointData[TinSave[i]->ID3].x; blockTin[i].pts[pT - 1].y = PointData[TinSave[i]->ID3].y; blockTin[i].pts[pT - 1].ID = PointData[TinSave[i]->ID3].ID;
//	}
//	for (i = 0; i<BlockTotal; i++)
//		gpNum[i] = 0;  //标记每一块内的点数的序号
//	for (i = 0; i<pointNumber - 4; i++)
//	{
//		int gNo = PointData[i].gNo;    //点所在的块号
//		if (gNo >= 0 && gNo<BlockTotal)
//		{
//			blockTin[gNo].pts[gpNum[gNo]].x = PointData[i].x;
//			blockTin[gNo].pts[gpNum[gNo]].y = PointData[i].y;
//			blockTin[gNo].pts[gpNum[gNo]].ID = PointData[i].ID;
//			gpNum[gNo]++;  //块内点数加一
//		}
//	}
//	delete[]gpNum;
//	finish = clock();
//	runTime[0] = (double)(finish - start) / CLOCKS_PER_SEC;   // 自适应分组的时间   
//	start = clock();
//	for (i = 0; i<BlockTotal; i++)
//	{
//		int	pTotal = blockTin[i].pNum;     //块内的点数
//
//		root = NULL;	frontf = NULL;  frontl = NULL;	basef = NULL;	 basel = NULL;
//		GeneDelaunay(blockTin[i].pts, pTotal, &(blockTin[i].first), &(blockTin[i].last), &root, &frontf, &frontl, &basef, &basel);
//
//		GetCurrentBlockInfor(blockTin[i], TinSave[i], blockTin[i].pts, frontf, basef);
//		LOPScan_Nonrecursion(blockTin[i].first, &(blockTin[i].first), PointData);
//		RefreshBlockInfor(&blockTin[i], *TinSave[i]);
//		ClearPartionMemory(frontf, basef);
//		//		DrawBlockTin(blockTin[i].first,PointData);
//	}
//	SetBlockPointer(tinHead, blockTin);
//	ClearTinMemory(&tinHead);
//	for (i = 0; i<BlockTotal; i++)
//	{
//		if (i == 0)
//		{
//			tinHead = blockTin[i].first;
//			tinEnd = blockTin[i].last;
//		}
//		else
//		{
//			tinEnd->next = blockTin[i].first;
//			tinEnd = blockTin[i].last;
//		}
//	}
//	delete[]blockTin;
//	finish = clock();
//	runTime[1] = (double)(finish - start) / CLOCKS_PER_SEC;      //分块计算TIN的总时间
//	start = clock();
//	TRIANGLENODE *pTin = tinHead;
//	while (pTin != NULL)
//	{
//		pTin->visited = 0;
//		pTin = pTin->next;
//	}
//	LOPScan_Nonrecursion(tinHead, &tinHead, PointData);
//	//	LOPScan_Nonrecursion(tinHead, &tinHead,PointData);
//	finish = clock();
//	runTime[2] = (double)(finish - start) / CLOCKS_PER_SEC;      //LOP的总时间
//	start = clock();
//	DeleteVertex();
//	finish = clock();
//	runTime[3] = (double)(finish - start) / CLOCKS_PER_SEC;      //删除多余矩形顶点的时间
//	RefreshScreen();
//	finishTotal = clock();
//	runTime[4] = (double)(finishTotal - startTotal) / CLOCKS_PER_SEC;
//}

void CTINConstruction2View::SetBlockTri(BlockTin* block, TRIANGLE* blockTri, TRIANGLE tri, int no, int* found)
{

	TRIANGLE* tp = NULL;
	TRIANGLE* tp1 = NULL;
	int p = 0, q = 0;

	tp = tp1 = blockTri;
	while (tp != NULL&&tp->next != NULL)
	{
		tp = tp->next;
		if (HaveSameEdgeScan(tp->ID1, tp->ID2, tp->ID3, tri.ID1, tri.ID2, tri.ID3, &p, &q) == 1 && SameEdge(block->Edge[no].ID1, block->Edge[no].ID2, p, q) == 1)
		{
			block->Tin[no] = tp;
			*found = 1;
			return;
		}
	}
	while (tp1 != NULL&&tp1->back != NULL)
	{
		tp1 = tp1->back;
		if (HaveSameEdgeScan(tp1->ID1, tp1->ID2, tp1->ID3, tri.ID1, tri.ID2, tri.ID3, &p, &q) == 1 && SameEdge(block->Edge[no].ID1, block->Edge[no].ID2, p, q) == 1)
		{
			block->Tin[no] = tp1;
			*found = 1;
			return;
		}
	}
	if (*found != 1)
	{
		AfxMessageBox("SetBlockTri错误");
	}
}
void CTINConstruction2View::RefreshBlockInfor(BlockTin* block, TRIANGLE tri)
{
	int p = 0, q = 0, find = 0;
	if (!HaveSameEdgeScan(block->Tin[1]->ID1, block->Tin[1]->ID2, block->Tin[1]->ID3, tri.ID1, tri.ID2, tri.ID3, &p, &q))
	{

		find = 0;
		SetBlockTri(block, block->Tin[1], tri, 1, &find);
	}
	if (!HaveSameEdgeScan(block->Tin[2]->ID1, block->Tin[2]->ID2, block->Tin[2]->ID3, tri.ID1, tri.ID2, tri.ID3, &p, &q))
	{

		find = 0;
		SetBlockTri(block, block->Tin[2], tri, 2, &find);
	}
	if (!HaveSameEdgeScan(block->Tin[3]->ID1, block->Tin[3]->ID2, block->Tin[3]->ID3, tri.ID1, tri.ID2, tri.ID3, &p, &q))
	{

		find = 0;
		SetBlockTri(block, block->Tin[3], tri, 3, &find);
	}
}
//删除外包矩形多余的四个顶点
void CTINConstruction2View::DeleteVertex()
{
	TRIANGLENODE *p = tinHead;
	for (TRIANGLENODE *t = p->next; t != NULL;)
	{
		//pointNumber记录了点集中点的个数，最后四个为外包矩形的四个顶点
		if (pointNumber - 5<t->ID1 || pointNumber - 5<t->ID2 || pointNumber - 5<t->ID3)
		{
			p->next = t->next;
			TRIANGLENODE *temp = t;
			t = t->next;
			delete temp;
			continue;
		}
		t = t->next;
		p = p->next;
	}
	if (tinHead->ID1>pointNumber - 5 || tinHead->ID2>pointNumber - 5 || tinHead->ID3>pointNumber - 5)
		tinHead = tinHead->next;
}

void CTINConstruction2View::LOPScan_BlockTIN(TRIANGLE *p, TRIANGLE **tinfirst, MyPoint *OriginalData)
{
	LIST  *tp1 = NULL, *tp2 = NULL;	//定义记录三角形的链表节点
									//tp1保存链表头
									//tp2保存动态加入的邻接三角形

	LIST *front = NULL, *rear1 = NULL, *rear2 = NULL;	//front保存的是原三角形链表
														//rear1保存的是邻接三角形的链表
														//rear2保存的实际是一个节点，
														//该节点保存了第一个三角形（tinfirst），next指向null
														//邻接三角形链表的末尾都指向该节点，作用大概是，递归的出口，即next指向null

	rear2 = new LIST;	rear2->tri = *tinfirst;  rear2->next = NULL;
	front = new LIST;	front->tri = p; p->visited = 1;	front->next = rear2;
	rear1 = front;
	while (front->next != NULL)
	{
		tp1 = front;    //tp1为链表头
		if (tp1->tri->p1tin != NULL)  //邻接三角形存在
		{
			if (tp1->tri->p1tin->visited != 1) //且未被检测过
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p1tin;
				tp2->tri->visited = 1;
				rear1->next = tp2;      //将邻接三角形加入到rear1中
				tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		if (tp1->tri->p2tin != NULL)
		{
			if (tp1->tri->p2tin->visited != 1)
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p2tin;
				tp2->tri->visited = 1;
				rear1->next = tp2; tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		if (tp1->tri->p3tin != NULL)
		{
			if (tp1->tri->p3tin->visited != 1)
			{
				tp2 = new LIST;
				tp2->tri = tp1->tri->p3tin;
				tp2->tri->visited = 1;
				rear1->next = tp2; tp2->next = rear2;
				rear1 = rear1->next;
			}
		}
		front = front->next;
		//将第一个三角形的邻接三角形全部找到，并加入到链表rear1中
		//然后在从第一个三角形开始，从邻接三角形表的最后一个三角形开始进行优化
		Lop_all(&(tp1->tri), &rear1, rear2, OriginalData);
		delete tp1;
	}
	delete rear2;
	TRIANGLENODE *pTin = p;
	while (pTin != NULL)
	{
		pTin->visited = 0;
		pTin = pTin->next;
	}
	//	delete front;
}

DWORD WINAPI CreateTin(LPVOID pThreadParam)
{
	CThreadParam * p = (CThreadParam*)pThreadParam;

	int i = p->iBlock;
	FBNODE *basef = NULL, *basel = NULL; //底壳
	TREENODE *root = NULL, *frontf = NULL, *frontl = NULL; //前壳
	int	pTotal = p->pTin->blockTin[i].pNum;     //块内的点数
	root = NULL;	frontf = NULL;  frontl = NULL;	basef = NULL;	 basel = NULL;
	p->pTin->GeneDelaunay(p->pTin->blockTin[i].pts, pTotal, &(p->pTin->blockTin[i].first), &(p->pTin->blockTin[i].last), &root, &frontf, &frontl, &basef, &basel);
	p->pTin->GetCurrentBlockInfor(p->pTin->blockTin[i], p->pTin->TinSave[i], p->pTin->blockTin[i].pts, frontf, basef);
	p->pTin->ClearPartionMemory(frontf, basef);
	p->pTin->LOPScan_BlockTIN(p->pTin->blockTin[i].first, &(p->pTin->blockTin[i].first), p->pTin->PointData);
	p->pTin->RefreshBlockInfor(&p->pTin->blockTin[i], *p->pTin->TinSave[i]);

	return 0;
}


void CTINConstruction2View::OnGlobe()
{
	// TODO: 在此添加命令处理程序代码
	ZoomFull();
	RefreshScreen();
}


void CTINConstruction2View::OnSelect()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = SELECT;
}


void CTINConstruction2View::OnZoomIn()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = ZOOMIN;
}

void CTINConstruction2View::OnZoomOut()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = ZOOMOUT;
}


void CTINConstruction2View::OnPan()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = PAN;
}

void CTINConstruction2View::OnStartPNT()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = STARTPNT;
}

void CTINConstruction2View::OnEndPNT()
{
	// TODO: 在此添加命令处理程序代码
	OperateID = ENDPNT;
}

void CTINConstruction2View::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ptMouse.x = point.x;  ptMouse.y = point.y;  ptMouseTmp = ptMouse;
	PNT mpt = ptMouse;  GetMapPoint(&mpt);
	CRect rect;
	GetClientRect(&rect);
	//--------设置鼠标光标
	switch (OperateID)
	{
	case ZOOMOUT:  ::SetCursor(m_hZoomOut);
		xcenter = mpt.x; ycenter = mpt.y; zoomratio *= ZoomFactor;
		InvalidateRect(&rect);  break;
	case ZOOMIN:   ::SetCursor(m_hZoomIn);
		Captured = true;  break;
	case SELECT:   ::SetCursor(m_hSelect); break;

	case PAN:      ::SetCursor(m_hPan);
	default: break;
	}
	CView::OnLButtonDown(nFlags, point);
}


void CTINConstruction2View::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int DX, DY;  PNT mpt1 = ptMouse, mpt2 = { point.x,point.y };
	DX = (int)(mpt2.x - mpt1.x); DY = (int)(mpt2.y - mpt1.y);
	GetMapPoint(&mpt1); GetMapPoint(&mpt2);

	CRect   Rect;
	GetClientRect(&Rect);
	//-----设置鼠标光标
	switch (OperateID)
	{
	case ZOOMOUT:  ::SetCursor(m_hZoomOut); break;
	case ZOOMIN:   ::SetCursor(m_hZoomIn);
		Captured = false;
		if (labs(DX) >= 2 && labs(DY) >= 2)
		{
			double xxmin, yymin, xxmax, yymax, dxx, dyy, fx, fy;
			if (mpt1.x > mpt2.x) { xxmin = mpt2.x; xxmax = mpt1.x; }
			else { xxmin = mpt1.x; xxmax = mpt2.x; }
			if (mpt1.y > mpt2.y) { yymin = mpt2.y; yymax = mpt1.y; }
			else { yymin = mpt1.y; yymax = mpt2.y; }
			dxx = xxmax - xxmin; dyy = yymax - yymin;
			xcenter = xxmin + dxx / 2.;  ycenter = yymin + dyy / 2.;
			fx = dxx / ClientWidth;   fy = dyy / ClientHeight;
			zoomratio = (fx > fy) ? fx : fy;
		}
		else
		{
			xcenter = mpt2.x; ycenter = mpt2.y; zoomratio /= ZoomFactor;
		}
		InvalidateRect(&Rect); break;
	case PAN:      ::SetCursor(m_hPan);
		xcenter -= mpt2.x - mpt1.x; ycenter -= mpt2.y - mpt1.y;
		InvalidateRect(&Rect); break;
	case SELECT:   ::SetCursor(m_hSelect); break;
	case STARTPNT: 
		for (TRIANGLE *T = tinHead; T != NULL; T = T->next) {
			if (IsPointInTriangle(T, point.x, point.y)) {
				nStartTri = T->g_SeqNum;

				CString cstr;
				cstr.Format("%d", nStartTri);
				AfxMessageBox(cstr);
			}
		}
		if (!pStartPoint) {
			pStartPoint = new PNT;
		}
		pStartPoint->x = point.x;
		pStartPoint->y = point.y;
		GetMapPoint(pStartPoint);
		InvalidateRect(&Rect); break;
	case ENDPNT:
		for (TRIANGLE *T = tinHead; T != NULL; T = T->next) {
			if (IsPointInTriangle(T, point.x, point.y)) {
				nEndTri = T->g_SeqNum;

				CString cstr;
				cstr.Format("%d", nEndTri);
				AfxMessageBox(cstr);

				
			}
		}
		if (!pEndPoint) {
			pEndPoint = new PNT;
		}
		pEndPoint->x = point.x;
		pEndPoint->y = point.y;
		GetMapPoint(pEndPoint);
		InvalidateRect(&Rect); break;
	default: break;
	}

	CView::OnLButtonUp(nFlags, point);
}


bool CTINConstruction2View::SameSide(PNT & A, PNT & B, PNT & C, PNT & P)
{

	double ABx = B.x - A.x;
	double ABy = B.y - A.y;
	double ACx = C.x - A.x;
	double ACy = C.y - A.y;
	double APx = P.x - A.x;
	double APy = P.y - A.y;

	// AP * AB
	double APAB = ABy * APx - ABx * APy;
	// AC * AP
	double ACAP = APy * ACx - APx * ACy;

	// v1 and v2 should point to the same direction
	return APAB * ACAP >= 0;
}

// Same side method
// Determine whether point P in triangle ABC
bool CTINConstruction2View::IsPointInTriangle(TRIANGLE *T, PNT& P)
{
	PNT A, B, C;
	A.x = PointData[T->ID1].x;
	A.y = PointData[T->ID1].y;
	B.x = PointData[T->ID2].x;
	B.y = PointData[T->ID2].y;
	C.x = PointData[T->ID3].x;
	C.y = PointData[T->ID3].y;
	GetScreenPoint(&A);
	GetScreenPoint(&B);
	GetScreenPoint(&C);
	GetScreenPoint(&P);
	return SameSide(A, B, C, P) && SameSide(B, C, A, P) && SameSide(C, A, B, P);
}
bool CTINConstruction2View::IsPointInTriangle(TRIANGLE *T, double xp, double yp)
{
	PNT A, B, C, P;
	A.x = PointData[T->ID1].x;
	A.y = PointData[T->ID1].y; 
	B.x = PointData[T->ID2].x;
	B.y = PointData[T->ID2].y;
	C.x = PointData[T->ID3].x; 
	C.y = PointData[T->ID3].y;
	P.x = xp;
	P.y = yp;
	GetScreenPoint(&A);
	GetScreenPoint(&B);
	GetScreenPoint(&C);
	return SameSide(A, B, C, P) && SameSide(B, C, A, P) && SameSide(C, A, B, P);
}
void CTINConstruction2View::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	PNT mpt = { point.x,point.y }, pt12 = ptMouseTmp;

	GetMapPoint(&mpt);

	//------设置鼠标光标
	switch (OperateID)
	{
	case PAN:                     ::SetCursor(m_hPan); break;
	case ZOOMOUT:                 ::SetCursor(m_hZoomOut); break;
	case ZOOMIN:                  ::SetCursor(m_hZoomIn); break;
	case SELECT:                  ::SetCursor(m_hSelect); break;
	default:                      ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW)); break;
	}

	if (Captured&&OperateID == ZOOMIN)
	{
		CClientDC dc(this);

		CPen  NewPen1(PS_DOT, 1, RGB(255, 255, 255));
		CPen *OldPen = dc.SelectObject(&NewPen1);
		dc.SetROP2(R2_NOT);
		//-----消除原矩形
		dc.MoveTo((int)ptMouse.x, (int)ptMouse.y);
		dc.LineTo((int)ptMouse.x, (int)pt12.y);
		dc.LineTo((int)pt12.x, (int)pt12.y);
		dc.LineTo((int)pt12.x, (int)ptMouse.y);
		dc.LineTo((int)ptMouse.x, (int)ptMouse.y);
		CPen NewPen2(PS_DOT, 1, RGB(0, 0, 0));
		dc.SelectObject(&NewPen2)->DeleteObject();
		//画新矩形
		dc.MoveTo((int)ptMouse.x, (int)ptMouse.y);
		dc.LineTo((int)ptMouse.x, (int)point.y);
		dc.LineTo((int)point.x, (int)point.y);
		dc.LineTo((int)point.x, (int)ptMouse.y);
		dc.LineTo((int)ptMouse.x, (int)ptMouse.y);
		dc.SelectObject(OldPen)->DeleteObject();

		ptMouseTmp.x = point.x;   ptMouseTmp.y = point.y;

	}
	CView::OnMouseMove(nFlags, point);
}

//NET net;
//void CTINConstruction2View::OnNetConstruction()
//{
//	// TODO: 在此添加命令处理程序代码
//	tinHead;
//}

bool CTINConstruction2View::IsLineExist(int PID1, int PID2)
{
	Line* pLine = m_LineSet.pLine;
	while (pLine != NULL) {
		if (pLine->ID1 == PID1 && pLine->ID2 == PID2) {
			return true;
		}
		pLine = pLine->next;
	}
	return false;
}

void CTINConstruction2View::TopologyConstruct() {
	//TriangleSet m_TriSet;
	//Line* pLineColl = new Line[_MAX_ARCNUM_aMap];
	long count = 0;
	Line *LineHead, *LineRear;
	LineHead = LineRear = NULL;
	for (TRIANGLE *t = tinHead; t != NULL; t = t->next) {
		// 先判断ID1、ID2组成的边是否已经存在(已存在的边则不用再次建立拓扑关系)
		if (!IsLineExist(t->ID1, t->ID2)) { 
			Line* pLine = new Line;
			pLine->LID = count++;
			pLine->ID1 = t->ID1;
			pLine->ID2 = t->ID2;
			//判断顶点ID3是否在顶点ID1、ID2组成的直线的左边，-1为左，1为右
			if (OnLeft(PointData[t->ID3], PointData[t->ID1], PointData[t->ID2]) == -1) {
				pLine->LeftTri = t->g_SeqNum;
				pLine->RightTri = t->p3tin ? t->p3tin->g_SeqNum : -1;
			}
			else
			{
				pLine->RightTri = t->g_SeqNum;
				pLine->LeftTri = t->p3tin ? t->p3tin->g_SeqNum : -1;
			}
			if (count == 1) {
				LineHead = pLine;
				LineRear = LineHead;
			}
			else
			{
				LineRear->next = pLine;
				LineRear = LineRear->next;
			}
			
			
		}
		//判断顶点ID2是否在顶点ID1、ID3组成的直线的左边，-1为左，1为右
		if (!IsLineExist(t->ID1, t->ID3)) {
			Line* pLine = new Line;
			pLine->LID = count++;
			pLine->ID1 = t->ID1;
			pLine->ID2 = t->ID3;
			if (OnLeft(PointData[t->ID2], PointData[t->ID1], PointData[t->ID3]) == -1) {
				pLine->LeftTri = t->g_SeqNum;
				pLine->RightTri = t->p2tin ? t->p2tin->g_SeqNum : -1;
			}
			else
			{
				pLine->RightTri = t->g_SeqNum;
				pLine->LeftTri = t->p2tin ? t->p2tin->g_SeqNum : -1;
			}
			LineRear->next = pLine;
			LineRear = LineRear->next;
		}
		//判断顶点ID1是否在顶点ID2、ID3组成的直线的左边，-1为左，1为右
		if (!IsLineExist(t->ID2, t->ID3)) {
			Line* pLine = new Line;
			pLine->LID = count++;
			pLine->ID1 = t->ID2;
			pLine->ID2 = t->ID3;
			if (OnLeft(PointData[t->ID1], PointData[t->ID2], PointData[t->ID3]) == -1) {
				pLine->LeftTri = t->g_SeqNum;
				pLine->RightTri = t->p1tin ? t->p1tin->g_SeqNum : -1;
			}
			else
			{
				pLine->RightTri = t->g_SeqNum;
				pLine->LeftTri = t->p1tin ? t->p1tin->g_SeqNum : -1;
			}
			LineRear->next = pLine;
			LineRear = LineRear->next;
		}
		//memcpy(pLineColl + count - 3, pLine, 3 * sizeof(Line));
	}
	m_LineSet.nLineNum = count;
	m_LineSet.pLine = LineHead;
}


void CTINConstruction2View::CreateTriPath()
{
	if (nStartTri == -1 || nEndTri == -1) {
		AfxMessageBox("未设置起点或终点！");
		return;
	}

	std::queue<TRIANGLE*> queTri;

	for (TRIANGLE *tri = tinHead; tri != NULL; tri = tri->next) {
		tri->visited = 0;
		if (tri->g_SeqNum == nStartTri) {
			pStartTri = tri;
		}
		if (tri->g_SeqNum == nEndTri) {
			pEndTri = tri;
		}
	}
	// 最短路径计算
	queTri.push(pStartTri);
	while (queTri.size() != 0) {
		TRIANGLE *T = queTri.front();
		T->visited = 1;
		if (T->p1tin && !T->p1tin->visited) {
			double accu = T->accu + (T->weight + T->p1tin->weight) / 2;
			if (!T->p1tin->parentTri || accu < T->p1tin->accu) {
				T->p1tin->accu = accu;
				T->p1tin->parentTri = T;
			}
			queTri.push(T->p1tin);
		}
		if (T->p2tin && !T->p2tin->visited) {
			double accu = T->accu + (T->weight + T->p2tin->weight) / 2;
			if (!T->p2tin->parentTri || accu < T->p2tin->accu) {
				T->p2tin->accu = accu;
				T->p2tin->parentTri = T;
			}
			queTri.push(T->p2tin);
		}
		if (T->p3tin && !T->p3tin->visited) {
			double accu = T->accu + (T->weight + T->p3tin->weight) / 2;
			if (!T->p3tin->parentTri || accu < T->p3tin->accu) {
				T->p3tin->accu = accu;
				T->p3tin->parentTri = T;
			}
			queTri.push(T->p3tin);
		}
		queTri.pop();
	}

	CRect Rect;
	GetClientRect(&Rect);
	InvalidateRect(&Rect);
}

void CTINConstruction2View::CreateLinePath() {
	MyPoint* PointData2;
	try {
		PointData2 = new MyPoint[pointNumber + 2];
	}
	catch (exception ex) {
		AfxMessageBox("alloc error!");
		return;
	}
	memcpy(PointData2, PointData, pointNumber * sizeof(MyPoint));
	delete[]PointData;
	PointData = PointData2;

	PointData[pointNumber].x = pStartPoint->x;
	PointData[pointNumber].y = pStartPoint->y;
	PointData[pointNumber].ID = pointNumber++;

	PointData[pointNumber].x = pEndPoint->x;
	PointData[pointNumber].y = pEndPoint->y;
	PointData[pointNumber].ID = pointNumber++;

	OnTinGenerate();
	TopologyConstruct();

	std::queue<Line*> queLine;
}

void CTINConstruction2View::OnPathConstruction()
{
	// TODO: 在此添加命令处理程序代码
	CreateTriPath();
	CreateLinePath();
}


void CTINConstruction2View::OnTinDensify()
{
	// TODO: 在此添加命令处理程序代码
	
}
