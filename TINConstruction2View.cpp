
// TINConstruction2View.cpp : CTINConstruction2View ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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
	// ��׼��ӡ����
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

// CTINConstruction2View ����/����

CTINConstruction2View::CTINConstruction2View()
{
	// TODO: �ڴ˴���ӹ������
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
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CTINConstruction2View ����

void CTINConstruction2View::OnDraw(CDC* /*pDC*/)
{
	CTINConstruction2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CTINConstruction2View ��ӡ

BOOL CTINConstruction2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CTINConstruction2View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CTINConstruction2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}


// CTINConstruction2View ���

#ifdef _DEBUG
void CTINConstruction2View::AssertValid() const
{
	CView::AssertValid();
}

void CTINConstruction2View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTINConstruction2Doc* CTINConstruction2View::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTINConstruction2Doc)));
	return (CTINConstruction2Doc*)m_pDocument;
}
#endif //_DEBUG

// CTINConstruction2View ��Ϣ�������

////////////////////////////////////////////////////////////////////////////

// ��CString ����ת��Ϊchar *����

char *CString2LPSTR(CString& cstr) {
	int nlen = cstr.GetLength() + 1;
	char *charstr = new char[nlen];
	wcstombs_s(0, charstr, nlen, cstr, _TRUNCATE);
	return charstr;
}

void CTINConstruction2View::OnReadShapefile()
{
	// TODO: �ڴ���������������
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
		PointData[i].label = false;  //��ʾ��δ����
	}

	int nFlagPoint = true;
	CalcBoundPoint();
	CalcBoundGraph();
	ZoomFull();
	AfxMessageBox("��ȡ��������ϣ�");
	RefreshScreen();

}

//��������ݵ����
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