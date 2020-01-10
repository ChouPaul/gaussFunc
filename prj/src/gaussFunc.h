#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <Eigen/Eigen>
#include <iomanip>

using namespace std;
using namespace Eigen;

#define SQR2(x) ((x)*(x))
#define SQR3(x) (SQR2(x)*(x))
#define SQR4(x) (SQR3(x)*(x))
#define SQR6(x) (SQR4(x)*SQR2(x))
#define SQR8(x) (SQR4(x)*SQR4(x))
#define SQR10(x) (SQR8(x)*SQR2(x))

#define PI (3.1415926535898)
#define D2R (PI/180.)
#define R2D (180./PI)

// ��ѡ����������
enum  enEarType
{
	SYSBJ54 = 0,
	SYSGDZ80,
	SYSWGS84,
	SYSCGCS2000,
};

// �������������ͨ�����������������ᡢ���ʣ��������򣬼��Զ���Ĵ�2�������Ĺ��캯��
struct stEarPara
{
	double a;
	double f;
	double e;
	double b;
	double e2;
	stEarPara(){}
	stEarPara(double da, double df)
	{
		a = da, f = df;
		e = sqrt(2 * f - f * f);
		b = a - a * f;
		e2 = sqrt(a*a - b* b) / b;
	}
};

// ����ת��������������ƽ���Ĳ���&�ռ�ֱ������7����
struct stCoorTransPara
{
	double dX;
	double dY;
	double dZ;
	double rX;
	double rY;
	double rZ;
	double m;
};

// ���ֳ�������ļ��ϲ�����ͨ�������ᡢ���ʼ��ɻ�ȡ�������β���
// ref--����Ԫ��P98�� �����磬P8�� ICD BDS��2013
static const stEarPara gBJ54Ear(6378245., 1. / 298.3);

static const stEarPara gGDZ80Ear(6378140., 1. / 298.257);

static const stEarPara gWGS84Ear(6378137., 1. / 298.257223563);

static const stEarPara gCGCS2000Ear(6378137., 1. / 298.257222101);


typedef struct stRawData
{
	string m_sN;
	double m_dX1;
	double m_dY1;
	double m_dZ1;
	double m_dX2;
	double m_dY2;
	double m_dZ2;
};

void printMat(const string &str, const int &iWid, const int &iFixed, ofstream &os, const Eigen::MatrixXd &mat);

void parseStr(const string &sSrc, const string &sParser, vector<string> &vsParsed);

string fTrimAll(string &sSrc);



// ��˹ͶӰ�������㲻�ִ�: ָ������������,�����dX dY����500km�����Ӵ���
// dB -- [-90, 90], deg; dL -- [-180, 180], deg; dL0 -- [0., 360.], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussFw(enEarType earType, double dB, double dL, double dL0, double *dX, double *dY);

// ��˹ͶӰ�������㲻�ִ�: ָ������������,�����dX dY��500km�����Ӵ���
// dB -- [-90, 90], deg; dL -- [-180, 180], deg; dL0 -- [0., 360.], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussFwWithAdd(enEarType earType, double dB, double dL, double dL0, double *dX, double *dY);

// ��˹ͶӰ��������6�ȴ�: ָ������������,�����dX dY��500km���Ӵ���
// dB -- [-90, 90], deg; dL -- [-180, 180], deg; dL0 -- [0., 360.], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussFwWithBelt(enEarType earType, double dB, double dL, double dL0, double *dX, double *dY);

// ��˹ͶӰ���귴��: ָ������������,�����dx dy����500km����������
// dL0 -- [0., 360.], deg; dB -- [-90, 90], deg; dL -- [0, 360], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussBw(enEarType earType, double dx, double dy, double dL0, double *dB, double *dL);

// ��˹ͶӰ���귴��: ָ������������,�����dx dy��500km����������
// dL0 -- [0., 360.], deg; dB -- [-90, 90], deg; dL -- [0, 360], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussBwWithAdd(enEarType earType, double dx, double dy, double dL0, double *dB, double *dL);

// ��׵�γ��
double calcuBf(enEarType earType, double dx);

// ��˹ͶӰ���귴��: *��һ���㷨*ָ������������,�����dx dy��500km���������ţ���gaussBw1�����൱
// dL0 -- [0., 360.], deg; dB -- [-90, 90], deg; dL -- [0, 360], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int gaussBw1(enEarType earType, double dx, double dy, double dL0, double *dB, double *dL);


// ������˹ͶӰ���������㲻�ִ�: ָ������������,�����dX dY��500km�����Ӵ���
// imode -- 0���㣬 1����
// dB -- [-90, 90], deg; dL -- [-180, 180], deg; dL0 -- [0., 360.], deg;
// return: -1 -- �����������, 1 -- ok
// note: �����BL�������xy������ͬһ����
int _declspec(dllexport) gaussBatch1(enEarType earType, int imode, const char *sFnSrc, const char *sFnDst);

