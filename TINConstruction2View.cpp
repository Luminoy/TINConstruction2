
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

#include "TINHeader.h"

#include <vector>
#include <iostream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTINConstruction2View

IMPLEMENT_DYNCREATE(CTINConstruction2View, CView)

BEGIN_MESSAGE_MAP(CTINConstruction2View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_READSHP, &CTINConstruction2View::OnReadShapefile)
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
			if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
				OGRPoint *poPoint = (OGRPoint *)poGeometry;
				char str[100];
				PNT POINT;
				POINT.x = poPoint->getX();
				POINT.y = poPoint->getY();
				PNTSet.push_back(POINT);
				sprintf_s(str, "%d : (%f, %f)\n", idx, poPoint->getX(), poPoint->getY());
				MessageBoxA(NULL, str, "coordinate", 0);
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

// CTINConstruction2View 构造/析构

CTINConstruction2View::CTINConstruction2View()
{
	// TODO: 在此处添加构造代码
	GDALAllRegister();
	GDALDataset *pData = (GDALDataset *)GDALOpen("E:\\gdal\\20150922.png", GA_ReadOnly);
	if (pData == NULL) {
		MessageBox(_T("ERRROR!"));
		return;
	}

	int nWidth = pData->GetRasterXSize();
	int nHeight = pData->GetRasterYSize();
	GDALClose(pData);

	
}

CTINConstruction2View::~CTINConstruction2View()
{
}

BOOL CTINConstruction2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CTINConstruction2View 绘制

void CTINConstruction2View::OnDraw(CDC* /*pDC*/)
{
	CTINConstruction2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

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

// 将CString 类型转化为char *类型

char *CString2LPSTR(CString& cstr) {
	int nlen = cstr.GetLength() + 1;
	char *charstr = new char[nlen];
	wcstombs_s(0, charstr, nlen, cstr, _TRUNCATE);
	return charstr;
}

void CTINConstruction2View::OnReadShapefile()
{
	// TODO: 在此添加命令处理程序代码
	CString  TheFileName;
	CFileDialog  FileDlg(TRUE);

	if (FileDlg.DoModal() == IDOK)
		TheFileName = FileDlg.GetPathName();
	else
		return;

	char *filename = CString2LPSTR(TheFileName);
	vector<PNT> PNTSet = ReadShapefile(filename, "ESRI Shapefile");
	
	int pointNumber =  PNTSet.size();
	PointData = new PointSet[pointNumber + 4];
	for (int i = 0; i<pointNumber; i++)
	{
		fscanf(fp, "%d%lf%lf", &PointData[i].ID, &PointData[i].x, &PointData[i].y);
		PointData[i].ID = i;
		PointData[i].label = false;  //表示点未被用
	}

	int nFlagPoint = true;
	CalcBoundPoint();
	CalcBoundGraph();
	ZoomFull();
	AfxMessageBox("读取点数据完毕！");
	RefreshScreen();

}

//计算点数据的外包
void CTINConstruction2View::CalcBoundPoint()
{
	double xminPoint, yminPoint, xmaxPoint, ymaxPoint;
	xminPoint = yminPoint = 1.E+256; xmaxPoint = ymaxPoint = -1.E+256;
	for (int i = 0; i<pointNumber; i++)
	{
		if (PointData[i].x<xminPoint) xminPoint = PointData[i].x;
		if (PointData[i].x>xmaxPoint) xmaxPoint = PointData[i].x;
		if (PointData[i].y<yminPoint) yminPoint = PointData[i].y;
		if (PointData[i].y>ymaxPoint) ymaxPoint = PointData[i].y;
	}
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