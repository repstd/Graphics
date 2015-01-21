#pragma once
#include "stdafx.h"
#include "Matrix.h"
#include <osg/Matrix>
#include <osg/GLBeginEndAdapter>
#include <OpenThreads\Thread>
#include <assert.h>

enum STATUS
{
	VS_DISABLE = 0x00,
	VS_ACTIVE = 0x01,
	VS_CULLED = 0x02,
	VS_STOPED = 0x04,
	VS_UNREACH = 0x08
};
enum NEIGHBOR
{
	NV_L = 0,
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
		float len = sqrt(double(_x*_x + _y*_y + _z*_z));

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

typedef struct _Range
{

	int _width,_height;
	int _centerX,_centerY;
	int _index_i, _index_j;
	int _N;
} Range;

class VAO
{

public:
	VAO()
	{

		glGenBuffers(1, &m_vbo);
		
		glGenBuffers(1, &m_ibo);

		glGenVertexArrays(1, &m_vao);

		
	}
	~VAO()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ibo);
		glDeleteVertexArrays(1, &m_vao);
		m_vecVertex.clear();

	}
	//[][3]
	//A array of 3-d points,of which the actual size is 3*w*h;
public:
	void initVertex(const float* vertexs)
	{
		float* p = const_cast<float*>(vertexs);
		while (p != NULL && (p + 1) != NULL && (p + 2) != NULL)
		{
			for (int i = 0; i < 3; i++)
				m_vecVertex.push_back(*(p++));

		}
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_vecVertex.size()*sizeof(GLfloat), &m_vecVertex[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void initVertex(const BYTE* heightMap, int offset_col/*offset in x brought by tile generation.*/, int offset_row, 
					Range rlocal, Range rglobal,
					int offset_x,int offset_y,int offset_z=-100)
	{

		BYTE* pHeight = const_cast<BYTE*>(heightMap);
		for (int row = 0; row < rlocal._height; row++)
		{
			for (int col = 0; col < rlocal._width; col++)
			{
				BYTE* p = pHeight + (offset_row + rlocal._height - 1 - row)*rglobal._width + col+offset_col;
				if (p == NULL)
					continue;

			m_vecVertex.push_back(col+offset_col+offset_x);
			m_vecVertex.push_back(row + offset_row + offset_y);
			m_vecVertex.push_back(*(p)+offset_z);

			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_vecVertex.size()*sizeof(GLfloat), &m_vecVertex[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);



	}
	void initVertex(const BYTE* heightMap,int row_offset,int col_offset,
					int w, int h,
					int offset_x,int offset_y,int offset_z)
	{
		BYTE* pHeight = const_cast<BYTE*>(heightMap);
		for (int row = row_offset; row<row_offset + h; row++)
			for (int col = col_offset; col < col_offset+w; col++)
		{
			BYTE* p = pHeight + (2*row_offset+h-1-row)*w + col;
			if (p == NULL)
				continue;
			m_vecVertex.push_back(col+offset_x);
			m_vecVertex.push_back(row+offset_y);
			m_vecVertex.push_back(*p+offset_z);

		}
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_vecVertex.size()*sizeof(GLfloat), &m_vecVertex[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void updateIndex(const UINT* indx, int size)
	{

		m_index = const_cast<GLuint*>(indx);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(GLuint), m_index, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	void draw(int startIndex, int num)
	{
		//We must make sure  it was complete triangles that were put into buffer.
		assert(num % 3 == 0);

		glBindVertexArray(m_vao);	
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

		glEnableClientState(GL_VERTEX_ARRAY);
		for (int curIndex = 0; curIndex < num / 3; curIndex++)
		{
			//Here every face of the mesh is Visualized as a closed line loop.
			glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(3*curIndex*sizeof(GLuint)));
		}
		//glDrawElements(GL_LINE_LOOP, num, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}
private:
	GLuint* m_index;
	GLuint m_ibo, m_vbo;
	GLuint m_vao;
	std::vector<GLfloat> m_vecVertex;
	std::vector<GLint> m_vecIndex;

};
class LODTile 
{
public:
	LODTile();


	void init(BYTE* heightMat, const Range globalRange,const Range localRange);
	void updateCameraInfo(osg::Vec3d& eye);
	void updateCameraInfo(osg::Vec3d&	eye, osg::GLBeginEndAdapter& gl,osg::State* stat);

	int   GetHeight(int x, int y) const;
	float GetAveHeight(float x, float y) const;

	void BFSRender() const;
	
	void BFSRenderPrimitive() const;
	void DrawIndexedPrimitive() const;
private:


	void initParams();

	inline void GLVERTEX(int x, int y) const;
	inline void local2Global(int& x, int& y, int& z) const;


	void DrawNode(int cx, int cy, int dx, int dy) const;
	void DrawPrim(int cx, int cy) const;

	BOOL NodeCanDivid(int cx, int cy, int dx, int dy) const;
	void DividNode(int cx, int cy, int dx, int dy) const;
	void DisableNode(int cx, int cy, int dx, int dy) const;

	void CheckNeighbor(int cx, int cy, int dx, int dy) const;
	unsigned char CanActive(int x, int y, int pitchSizeX, int pitchSizeY) const;
	VECTOR getNormal(int x, int y, int dx, int dy) const;

	void CalculateDHMatrix();
	void DrawPrim_FILL(int x, int y) const;

	//void DrawPrim_TEXTURE(int x, int y);
	void DrawPrim_FRAME(int x, int y) const;
	void DrawNode_FILL(int x, int y, int dx, int dy) const;
	//void DrawNode_TEXTURE(int x, int y, int d);
	void DrawNode_FRAME(int x, int y, int dx, int dy) const;



private:

	CMatrix<BYTE> m_HMMatrix;
	CMatrix<BYTE>    m_LodMatrix;
	CMatrix<float> m_DHMatrix;

	float m_fc;
	float m_fC;

	Range m_rlocalPara;
	Range m_rglobalPara;


	mutable float m_ViewX, m_ViewY, m_ViewZ;
	PatchSize   m_delta[30];
	mutable int   m_neighbor[4];
	float         m_fScale;
#if 0
	mutable osg::GLBeginEndAdapter m_gl;
	mutable osg::State* m_stat;
#endif

	mutable VAO m_vertexBuf;
	mutable std::vector<UINT> m_indexBuf;
};

