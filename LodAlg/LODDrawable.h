#pragma 
#include "stdafx.h"
#include "Matrix.h"
#include <osg/Drawable>
#include <osg/RenderInfo>
#include <assert.h>

enum STATUS
{
	VS_DISABLE = 0x00,
	VS_ACTIVE = 0x01,
	VS_CULLED = 0x02,
	VS_STOPED = 0x04,
	VS_UNREACH=0x08
};
enum NEIGHBOR
{
	NV_L=0,
	NV_U,
	NV_D,
	NV_R

};
typedef struct _NODE
{
	int _x, _y;
	int _lod;
	_NODE()
	{
		_x = 0;
		_y = 0;
		_lod = -1;
	}
	_NODE(int x, int y, int l)
	{
		_x = x;
		_y = y;
		_lod = l;
	}
	void setLOD(int l)
	{
		_lod = l;
	}
	int getLOD()
	{
		return _lod;
	}
} LNODE;


template <class T>
struct _VECTOR
{
	mutable T _x, _y, _z;
	_VECTOR()
	{
		_x = 0;
		_y = 0;
		_z = 0;

	
	}
	_VECTOR(T x, T y, T z)
	{

		_x = x;
		_y = y;
		_z = z;
	}
	T operator*(const _VECTOR& n) const
	{

		return _x*n._x + _y*n._y + _z*n._z;
	}
	_VECTOR operator-(const _VECTOR& n)const
	{

		T nx = _x - n._x;
		T ny = _y - n._y;
		T nz = _z - n._z;
		return _VECTOR(nx, ny, nz);
	}
	_VECTOR operator+(const _VECTOR& n)const
	{

		T nx = _x + n._x;
		T ny = _y + n._y;
		T nz = _z + n._z;
		return _VECTOR(nx, ny, nz);
	}
	_VECTOR operator/(float s)const
	{
		assert(s > 0.01);
		_x = _x / s;
		_y = _y / s;
		_z = _z / s;
		return *this;

	}
	//Cross Dot
	_VECTOR getCross(const _VECTOR& n)const
	{
		T nx = _y*n._z - _z*n._y;
		T ny = _z*n._x - _x*n._z;
		T nz = _x*n._y - _y*n._x;
		return _VECTOR(nx, ny, nz);
	}
	void normalize()
	{
		float len= sqrt(double(_x*_x + _y*_y + _z*_z));

		if (len > 0.01)
		{
			_x = _x / len;
			_y = _y / len;
			_z = _z / len;

		}
		else
		{
			_x = 0;
			_y = 1;
			_z = 0;
		}
		
	}
	T length()
	{
		float fL = sqrt(double(_x*_x + _y*_y + _z*_z));
		return *((T*)(&fL));

	}
};
//Alias for vector,specially used for normal.


typedef _VECTOR<float> VECTOR;


typedef struct _PatchSize
{
	_PatchSize()
	{
		_x = 0;
		_y = 0;
	}
	void set(int x, int y)
	{
		_x = x;
		_y = y; 
	}
	int _x, _y;

} PatchSize;
class LODDrawable :public osg::Drawable
{
public:

	LODDrawable(const LODDrawable& drawable, const osg::CopyOp    &copyop = osg::CopyOp::SHALLOW_COPY) 
		:Drawable(drawable, copyop)
	{
	}
	META_Object(osg, LODDrawable);

	LODDrawable();
	~LODDrawable();

	void init(char* heightFiledMap);
	void init(BYTE* heightMat, int width, int height);

private:
	void drawImplementation(osg::RenderInfo& renderInfo) const;
	bool loadRawData(const char* fileName);
	bool loadHeightField(const char* filename);
	int   GetHeight(int x, int y) const;
	float GetAveHeight(float x, float y) const;

	void BFSRender() const;
	void DrawNode(int cx, int cy, int dx,int dy) const;
	void DrawPrim(int cx, int cy) const;

	BOOL NodeCanDivid(int cx, int cy, int dx,int dy) const;
	void DividNode(int cx, int cy, int dx,int dy) const;
	void DisableNode(int cx, int cy, int dx,int dy) const;

	void CheckNeighbor(int cx, int cy, int dx,int dy) const;
	unsigned char CanActive(int x, int y, int pitchSizeX, int pitchSizeY) const;
	VECTOR getNormal(int x, int y, int dx,int dy) const;



	void CalculateDHMatrix();
	void DrawPrim_FILL(int x, int y) const;
	//void DrawPrim_TEXTURE(int x, int y);
	void DrawPrim_FRAME(int x, int y) const;
	void DrawNode_FILL(int x, int y, int dx,int dy) const;
	//void DrawNode_TEXTURE(int x, int y, int d);
	void DrawNode_FRAME(int x, int y, int dx,int dy) const;

private:
	



	float m_fc;     
	float m_fC;    
	int   m_nSizeX,m_nSizeY;  
	mutable float m_ViewX, m_ViewY, m_ViewZ;
	PatchSize   m_delta[30];

	mutable int   m_neighbor[4]; 
	int   m_subnode[4];


	float         m_fScale;   
	CMatrix<BYTE> m_HMMatrix;


	CMatrix<unsigned char>    m_LodMatrix;
	CMatrix<float> m_DHMatrix;

};

