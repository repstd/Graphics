#include "StdAfx.h"
#include "ltiles.h"
#include <vector>
#include <queue>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <gdal.h>
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include "linput.h"
#define MAX_DIS 700.0

#define _DEBUG_FILENAME_ "./pos.txt"
#define DEBUG_LOG_INIT _DEBUG_LOG_INIT(_DEBUG_FILENAME_)
#define DEBUG_ENCODE_MSG(format,data,...) _DEBUG_ENCODE_MSG(_DEBUG_FILENAME_,format,data)

#ifndef _GL_ELE_ARRAY
#define GL_BEGIN(status) glBegin(status)
#define GL_END() glEnd()
#else
#define GL_BEGIN(status)
#define GL_END() glEnd()
#endif
VAO::VAO()
{

	glGenBuffers(1, &m_vbo);

	glGenBuffers(1, &m_ibo);

	glGenVertexArrays(1, &m_vao);
}
VAO::~VAO()
{

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
	m_vecVertex.clear();

}

void VAO::initVertex(const float* vertexs)
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

void VAO::initVertex(const BYTE* heightMap, int offset_col, int offset_row, Range rlocal, Range rglobal, int offset_x, int offset_y, int offset_z ,bool isFlip)
{

	BYTE* pHeight = const_cast<BYTE*>(heightMap);
	for (int row = 0; row < rlocal._height; row++)
	{
		for (int col = 0; col < rlocal._width; col++)
		{
			BYTE* p;
			if (isFlip)
			{
				if (rlocal == rglobal)
					p = pHeight + (rlocal._height - 1 - row)*rglobal._width + col;
				else
					p = pHeight + (offset_row + rlocal._height - 1 - row)*rglobal._width + col + offset_col;
			}
			else
			{
				if (rlocal == rglobal)
					p = pHeight + row*rglobal._width + col;
				else
					p = pHeight + (offset_row + row)*rglobal._width + col + offset_col;
			}

			if (p == NULL)
				continue;
			m_vecVertex.push_back(col + offset_col + offset_x);
			m_vecVertex.push_back(row + offset_row + offset_y);
			m_vecVertex.push_back(*(p)+offset_z);

		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vecVertex.size()*sizeof(GLfloat), &m_vecVertex[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
void VAO::initVertex(const BYTE* heightMap, int row_offset, int col_offset, int w, int h, int offset_x, int offset_y, int offset_z,bool isFlip)
{
	BYTE* pHeight = const_cast<BYTE*>(heightMap);
	for (int row = row_offset; row < row_offset + h; row++)
	for (int col = col_offset; col < col_offset + w; col++)
	{
		BYTE* p;
		if (isFlip)
			p = pHeight + (2 * row_offset + h - 1 - row)*w + col;
		else
			p = pHeight + (2 * row_offset + row)*w + col;

		if (p == NULL)
			continue;
		m_vecVertex.push_back(col + offset_x);
		m_vecVertex.push_back(row + offset_y);
		m_vecVertex.push_back(*p + offset_z);

	}
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vecVertex.size()*sizeof(GLfloat), &m_vecVertex[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VAO::updateIndex(const UINT* indx, int size)
{

	m_index = const_cast<GLuint*>(indx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(GLuint), m_index, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void VAO::draw(int startIndex, int num)
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
		glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(3 * curIndex*sizeof(GLuint)));
	}
	//glDrawElements(GL_LINE_LOOP, num, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}
LODTile::LODTile()
{


}


void LODTile::initParams()
{
	m_fC = 1;
	m_fc = 1.5;
	m_fScale = 1.0;
	m_LodMatrix.Reset(m_rlocalPara._width, m_rlocalPara._height);
	m_DHMatrix.Reset(m_rlocalPara._width, m_rlocalPara._height);
	for (int i = 0; i < m_rlocalPara._width; i++)
	for (int j = 0; j < m_rlocalPara._height; j++)
		m_LodMatrix(i, j) = VS_UNREACH;
	CalculateDHMatrix();

	int sizeX = m_rlocalPara._width - 1;
	int sizeY = m_rlocalPara._height - 1;

	for (int i = 0; sizeX&&sizeY; i++)
	{
		this->m_delta[i]._x = sizeX / 2;
		this->m_delta[i]._y = sizeY / 2;
		sizeX = sizeX >> 1;
		sizeY = sizeY >> 1;
	}

}

void LODTile::init(BYTE* heightMat, const Range globalRange, const Range localRange)
{

	m_rlocalPara = localRange;
	m_rglobalPara = globalRange;
	int offsetCol = localRange._index_i*localRange._width;
	int offsetRow = (localRange._N - 1 - localRange._index_j)*localRange._height;

	m_HMMatrix.Reset(m_rlocalPara._width, m_rlocalPara._height);

	m_HMMatrix.SetData(heightMat, offsetCol, offsetRow, globalRange._width, globalRange._height, localRange._width, localRange._height);

	m_vertexBuf.initVertex(heightMat,
		offsetCol,
		offsetRow,
		localRange,
		globalRange,
		m_rlocalPara._centerX - m_rlocalPara._width / 2 - m_rglobalPara._width / 2,
		m_rlocalPara._centerY - m_rlocalPara._height / 2 - m_rglobalPara._height / 2,
		-100);

	initParams();

}

void LODTile::init(heightField* input, int i, int j, int N)
{
	BYTE* temp = new unsigned char[input->getTileWidth(i,j,N)* input->getTileHeight(i,j,N)];

	input->generateTile(i, j, N, temp, m_rlocalPara,m_rglobalPara);

	int offsetCol = m_rlocalPara._index_i*m_rlocalPara._width;
	int offsetRow = (m_rlocalPara._N - 1 - m_rlocalPara._index_j)*m_rlocalPara._height;

	m_HMMatrix.Reset(m_rlocalPara._width, m_rlocalPara._height);
	m_HMMatrix.SetData(temp, 0, 0, m_rlocalPara._width, m_rlocalPara._height, m_rlocalPara._width, m_rlocalPara._height,false);
	m_vertexBuf.initVertex(temp, 
		offsetCol, 
		offsetRow, 
		m_rlocalPara, 
		m_rlocalPara,
		m_rlocalPara._centerX - m_rlocalPara._width / 2 - m_rglobalPara._width / 2,
		m_rlocalPara._centerY - m_rlocalPara._height / 2 - m_rglobalPara._height / 2,
		-100,false);

	delete[] temp;

	initParams();

}
void LODTile::updateCameraInfo(osg::Vec3d& eye)
{
	m_ViewX = eye.x();
	m_ViewZ = eye.z();

	m_ViewY = eye.y();
}

void LODTile::updateCameraInfo(osg::Vec3d&	eye, osg::GLBeginEndAdapter& gl, osg::State* stat)
{
	m_ViewX = eye.x();

	m_ViewZ = eye.z();

	m_ViewY = eye.y();
}



void LODTile::BFSRender() const
{

	std::vector<LNODE>  v1;
	std::vector<LNODE>  v2;
	std::vector<LNODE>* cur_Queue;
	std::vector<LNODE>* next_Queue, *temp_v;
	cur_Queue = &v1; next_Queue = &v2;

	int level = 0, i = 0;
	int dx = m_delta[0]._x;
	int dy = m_delta[0]._y;
	//Here we should use cx(y)=(w(h)-1)/2 instead of (w(h)-1)/1+1;	
	int cx = floor(float(m_rlocalPara._width) / 2);
	int cy = floor(float(m_rlocalPara._height) / 2);
	//int cx = (m_rlocalPara._width ) / 2;
	//int cy = (m_rlocalPara._height) / 2;
	LNODE node(cx, cy, 0);
	cur_Queue->push_back(node);

	if (!m_indexBuf.empty())
		m_indexBuf.clear();
	while (dx > 0 && dy > 0)
	{
		int vlen = cur_Queue->size();
		for (i = 0; i < vlen; i++)
		{
			node = (*cur_Queue)[i];
			cx = node._x;
			cy = node._y;


			CheckNeighbor(cx, cy, dx, dy);
			if (NodeCanDivid(cx, cy, dx, dy) && CanActive(cx, cy, 2 * dx, 2 * dy))
			{
				if (dx == 1 || dy == 1)
				{
					m_LodMatrix(cx, cy) = VS_CULLED;
					DrawPrim(cx, cy);
				}
				else
				{
					DividNode(cx, cy, dx, dy);
					node.setLOD(level + 1);
					//Push the left_up sub node to the next level Queue
					node._x = cx - dx / 2;
					node._y = cy + dy / 2;
					next_Queue->push_back(node);

					//Push the left_down sub node to the next level queue
					node._x = cx - dx / 2;
					node._y = cy - dy / 2;
					next_Queue->push_back(node);

					//Push the right_up sub node to the next level queue
					node._x = cx + dx / 2;
					node._y = cy + dy / 2;
					next_Queue->push_back(node);

					//Push the right_down sub node to the next level queue
					node._x = cx + dx / 2;
					node._y = cy - dy / 2;
					next_Queue->push_back(node);
				}
			}
			else
			{
				DisableNode(cx, cy, dx, dy);
				DrawNode(cx, cy, dx, dy);
			}
		}
		cur_Queue->clear();
		//Swap the two queue
		temp_v = cur_Queue;
		cur_Queue = next_Queue;
		next_Queue = temp_v;
		level++;
		dx = m_delta[level]._x;
		dy = m_delta[level]._y;

	}
#ifdef _GL_ELE_ARRAY //use index buffer
#ifndef _GL_MT      //render outside the worker thread if multi-thread used.
	DrawIndexedPrimitive();
#endif
#endif
}
void LODTile::DrawIndexedPrimitive() const
{
	m_vertexBuf.updateIndex(&m_indexBuf[0], m_indexBuf.size());
	m_vertexBuf.draw(0, m_indexBuf.size());
}
int  LODTile::GetHeight(int x, int y) const
{


	//return (m_HMMatrix(m_rlocalPara._height - 1 - x, m_rlocalPara._height - 1-z)*m_fScale);
	if (x < 0 || x >= m_rlocalPara._width || y < 0 || y >= m_rlocalPara._height)
		return _LOD_ERROR;
	return (m_HMMatrix(x, y)*m_fScale);
	//return m_rlocalPara._centerX*0.2 + m_rlocalPara._centerY*0.2;
}


float LODTile::GetAveHeight(float x, float y) const
{
	int xl, xh, yl, yh;
	if (x<0 || x> m_rlocalPara._width || y<0 || y>m_rlocalPara._height) return 0;
	xl = x;
	xh = xl + 1;
	yl = y;
	yh = yl + 1;
	float dx = x - xl;
	float dy = y - yl;
	float hll = (dy  * dx) * m_HMMatrix(xh, yh);
	float hlh = ((1 - dy) * dx) * m_HMMatrix(xh, yl);
	float hhl = (dy  * (1 - dx)) * m_HMMatrix(xl, yh);
	float hhh = ((1 - dy) * (1 - dx)) * m_HMMatrix(xl, yl);

	return (hll + hlh + hhh + hhl) * m_fScale;
}
void LODTile::CalculateDHMatrix()
{
	int iEdgeLength = 3;
	while (iEdgeLength <= m_rlocalPara._width)
	{
		int iEdgeOffset = (iEdgeLength - 1) >> 1;
		int iChildOffset = (iEdgeLength - 1) >> 2;
		//TODO: Replace z with x. @141203
		for (int z = iEdgeOffset; z < m_rlocalPara._height; z += (iEdgeLength - 1))
		{
			for (int x = iEdgeOffset; x < m_rlocalPara._width; x += (iEdgeLength - 1))
			{
				if (iEdgeLength == 3)
				{
					int iDH[6];
					//BOTTOM
					iDH[0] = (int)ceil((float)abs(((GetHeight(x - iEdgeOffset, z + iEdgeOffset) + GetHeight(x + iEdgeOffset, z + iEdgeOffset)) >> 1) - GetHeight(x, z + iEdgeOffset)));
					//RIGHT
					iDH[1] = (int)ceil((float)abs(((GetHeight(x + iEdgeOffset, z + iEdgeOffset) + GetHeight(x + iEdgeOffset, z - iEdgeOffset)) >> 1) - GetHeight(x + iEdgeOffset, z)));
					//TOP
					iDH[2] = (int)ceil((float)abs(((GetHeight(x - iEdgeOffset, z - iEdgeOffset) + GetHeight(x + iEdgeOffset, z - iEdgeOffset)) >> 1) - GetHeight(x, z - iEdgeOffset)));
					//LEFT
					iDH[3] = (int)ceil((float)abs(((GetHeight(x - iEdgeOffset, z + iEdgeOffset) + GetHeight(x - iEdgeOffset, z - iEdgeOffset)) >> 1) - GetHeight(x - iEdgeOffset, z)));
					//LEFT-TOP TO RIGHT-BOTTOM
					iDH[4] = (int)ceil((float)abs(((GetHeight(x - iEdgeOffset, z - iEdgeOffset) + GetHeight(x + iEdgeOffset, z + iEdgeOffset)) >> 1) - GetHeight(x, z)));
					//RIGHT-TOP TO LEFT-BOTTOM
					iDH[5] = (int)ceil((float)abs(((GetHeight(x + iEdgeOffset, z - iEdgeOffset) + GetHeight(x - iEdgeOffset, z + iEdgeOffset)) >> 1) - GetHeight(x, z)));


					int iDHMAX = iDH[0];
					for (int i = 1; i < 6; i++)
					if (iDHMAX < iDH[i])
						iDHMAX = iDH[i];
					m_DHMatrix(x, z) = iDHMAX;
				}
				else
				{
					int iDH[14];
					int iNumDH = 0;
					float K = 1.0f * m_fC / (2.0f * (m_fC - 1.0f));

					//LEFT TWO CHILD
					int iNeighborX;
					int iNeighborZ;
					iNeighborX = x - (iEdgeLength - 1);
					iNeighborZ = z;
					if (iNeighborX > 0)
					{
						iDH[iNumDH] = m_DHMatrix(iNeighborX + iChildOffset, iNeighborZ - iChildOffset);
						iNumDH++;
						iDH[iNumDH] = m_DHMatrix(iNeighborX + iChildOffset, iNeighborZ + iChildOffset);
						iNumDH++;
					}
					//TOP TWO CHILD
					iNeighborX = x;
					iNeighborZ = z - (iEdgeLength - 1);
					if (iNeighborZ > 0)
					{
						iDH[iNumDH] = m_DHMatrix(iNeighborX - iChildOffset, iNeighborZ + iChildOffset);
						iNumDH++;
						iDH[iNumDH] = m_DHMatrix(iNeighborX + iChildOffset, iNeighborZ + iChildOffset);
						iNumDH++;
					}
					//RIGHT TWO CHILD
					iNeighborX = x + (iEdgeLength - 1);
					iNeighborZ = z;
					if (iNeighborX < m_rlocalPara._width)
					{
						iDH[iNumDH] = m_DHMatrix(iNeighborX - iChildOffset, iNeighborZ - iChildOffset);
						iNumDH++;
						iDH[iNumDH] = m_DHMatrix(iNeighborX - iChildOffset, iNeighborZ + iChildOffset);
						iNumDH++;
					}
					//BOTTOM TWO CHILD
					iNeighborX = x;
					iNeighborZ = z + (iEdgeLength - 1);
					if (iNeighborZ < m_rlocalPara._height)
					{
						iDH[iNumDH] = m_DHMatrix(iNeighborX - iChildOffset, iNeighborZ - iChildOffset);
						iNumDH++;
						iDH[iNumDH] = m_DHMatrix(iNeighborX + iChildOffset, iNeighborZ - iChildOffset);
						iNumDH++;
					}
					int iDHMAX = iDH[0];
					for (int i = 1; i < iNumDH; i++)
					{
						if (iDHMAX < iDH[i])
							iDHMAX = iDH[i];
					}
					m_DHMatrix(x, z) = (int)ceil(K * iDHMAX);
				}
			}
		}
		iEdgeLength = (iEdgeLength << 1) - 1;
	}
}
VECTOR LODTile::getNormal(int x, int y, int dx, int dy) const
{
	float cz = GetHeight(x, y);
	VECTOR LU(-1.0*dx, dy, GetHeight(x - dx, y + dy) - cz);
	VECTOR LD(-1.0*dx, -1.0*dy, GetHeight(x - dx, y - dy) - cz);
	VECTOR RU(dx, dy, GetHeight(x + dx, y + dy) - cz);
	VECTOR RD(dx, -1.0*dy, GetHeight(x + dx, y - dy) - cz);

	VECTOR nFaceLeft = LU.getCross(LD);
	VECTOR nFaceDown = LD.getCross(RD);
	VECTOR nFaceRight = RD.getCross(RU);
	VECTOR nFaceUp = RU.getCross(LU);

	VECTOR ave = (nFaceLeft + nFaceDown + nFaceRight + nFaceUp) / 4.0f;
	ave.normalize();
	return nFaceLeft;
}

unsigned char LODTile::CanActive(int x, int y, int patchSizeX, int patchSizeY) const
{

	if (patchSizeX == 2 || patchSizeY == 2)
		return VS_DISABLE;
	//int size = 2*d;
	int d = patchSizeX >> 1;
	int z = GetHeight(x, y);
	int lx, ly, lz;
	lx = x;
	ly = y;
	lz = z;
	local2Global(lx, ly, lz);
	//VECTOR observeVec = VECTOR(x - m_rlocalPara._width / 2 - m_ViewX, y - m_rlocalPara._height / 2 - m_ViewY, z - 100 - m_ViewZ);

	VECTOR observeVec = VECTOR(lx - m_ViewX, ly - m_ViewY, lz - m_ViewZ);

	float fViewDistance = observeVec.length();
	float f;
	float cosAngle = 0.0;

	if (x - d >= 0 && x - d < m_rlocalPara._width && y - d >= 0 && y - d < m_rlocalPara._height)
	{
		observeVec.normalize();
		VECTOR normal = getNormal(x, y, d, d);
		cosAngle = normal*observeVec;
	}

	if (fViewDistance > 1500)
		return VS_DISABLE;
	f = fViewDistance / (patchSizeX*abs(cosAngle)*m_fC*max(m_fc*m_DHMatrix(x, y), 1.0f));

	if (f < 0.3f)
		return VS_ACTIVE;
	else
		return VS_DISABLE;

}

void LODTile::DisableNode(int cx, int cy, int dx, int dy) const
{
	int d2x = dx >> 1;
	int d2y = dy >> 1;
	m_LodMatrix(cx, cy) = VS_ACTIVE;

	if (dx == 1 || dy == 1)
		return;
	m_LodMatrix(cx - d2x, cy - d2y) = VS_DISABLE;
	m_LodMatrix(cx - d2x, cy + d2y) = VS_DISABLE;
	m_LodMatrix(cx + d2x, cy - d2y) = VS_DISABLE;
	m_LodMatrix(cx + d2x, cy + d2y) = VS_DISABLE;
}

void LODTile::DividNode(int cx, int cy, int dx, int dy) const
{
	int d2x = dx >> 1;
	int d2y = dy >> 1;
	m_LodMatrix(cx, cy) = VS_ACTIVE;

	m_LodMatrix(cx - d2x, cy - d2y) = VS_ACTIVE;
	m_LodMatrix(cx - d2x, cy + d2y) = VS_ACTIVE;
	m_LodMatrix(cx + d2x, cy - d2y) = VS_ACTIVE;
	m_LodMatrix(cx + d2x, cy + d2y) = VS_ACTIVE;
}

BOOL LODTile::NodeCanDivid(int cx, int cy, int d, int dy) const
{
	if (m_neighbor[0] == VS_DISABLE)
		return FALSE;
	if (m_neighbor[1] == VS_DISABLE)
		return FALSE;
	if (m_neighbor[2] == VS_DISABLE)
		return FALSE;
	if (m_neighbor[3] == VS_DISABLE)
		return FALSE;
	return TRUE;
}

void LODTile::CheckNeighbor(int cx, int cy, int dx, int dy) const
{
	int nx, ny;
	int d2x = dx << 1;
	int d2y = dy << 1;

	m_neighbor[NV_L] = VS_ACTIVE;
	m_neighbor[NV_U] = VS_ACTIVE;
	m_neighbor[NV_D] = VS_ACTIVE;
	m_neighbor[NV_R] = VS_ACTIVE;
	nx = cx; ny = cy - d2y;
	if (ny > 0)
	{
		if (m_LodMatrix(nx, ny) == VS_DISABLE)
			m_neighbor[NV_D] = VS_DISABLE;
	}

	nx = cx; ny = cy + d2y;
	if (ny < m_rlocalPara._height)
	{
		if (m_LodMatrix(nx, ny) == VS_DISABLE)
			m_neighbor[NV_U] = VS_DISABLE;
	}

	nx = cx + d2x; ny = cy;
	if (nx <m_rlocalPara._width)
	{
		if (m_LodMatrix(nx, ny) == VS_DISABLE)
			m_neighbor[NV_R] = VS_DISABLE;
	}

	nx = cx - d2x; ny = cy;
	if (nx > 0)
	{
		if (m_LodMatrix(nx, ny) == VS_DISABLE)
			m_neighbor[NV_L] = VS_DISABLE;
	}
}

void LODTile::DrawNode(int cx, int cy, int d, int dy) const
{

	DrawNode_FRAME(cx, cy, d, d);
}
void LODTile::DrawPrim(int cx, int cy) const
{
	DrawPrim_FRAME(cx, cy);

}


//#define GLVERTEX(x, z) glVertex3f(x, GetHeight(x, z), z)
//#define GLVERTEX(x, z)\
//	glVertex3f(x, -1.0*z, 1.0*GetHeight(x, z)); \
//	DEBUG_ENCODE_MSG("Drawing Prim:\t(%d,%d,%d)\n", x, -1.0*z, 1.0*GetHeight(x, z)); 
//#define GLVERTEX(x, z)\
//{\
//	glVertex3f(x - m_nSize / 2, -1.0*z + m_nSize / 2, GetHeight(x, z) -100); \
//}
//#define GLVERTEX(x, z)\
//{\
//	glVertex3f(x + m_rlocalPara._width>>1 - m_rglobalPara._width>>1, z+m_rlocalPara._height>>1, GetHeight(x, z) - 100); \
//}

inline void LODTile::local2Global(int& x, int& y, int& z) const
{
	x += (m_rlocalPara._centerX - m_rlocalPara._width / 2 - m_rglobalPara._width / 2);
	y += (m_rlocalPara._centerY - m_rlocalPara._height / 2 - m_rglobalPara._height / 2);
	z -= 100;
}
inline void LODTile::GLVERTEX(int x, int y) const
{
#ifdef _GL_ELE_ARRAY
	m_indexBuf.push_back(y*m_rglobalPara._width+x);
#else
	int z = GetHeight(x, y);
	int lx = x;
	int ly = y;
	local2Global(lx, ly, z);
	glVertex3f(lx, ly, z);
#endif

}
void LODTile::DrawNode_FILL(int x, int z, int d, int dy) const
{
	GL_BEGIN(GL_TRIANGLE_FAN);
	glColor3f(1, 0, 0);
	GLVERTEX(x, z);

	glColor3f(1, 1, 1);
	GLVERTEX(x - d, z + d);
	if (m_neighbor[NV_L] == VS_ACTIVE)
		GLVERTEX(x - d, z);
	GLVERTEX(x - d, z - d);
	if (m_neighbor[NV_D] == VS_ACTIVE)
		GLVERTEX(x, z - d);
	GLVERTEX(x + d, z - d);
	if (m_neighbor[NV_R] == VS_ACTIVE)
		GLVERTEX(x + d, z);
	GLVERTEX(x + d, z + d);
	if (m_neighbor[NV_U] == VS_ACTIVE)
		GLVERTEX(x, z + d);
	GLVERTEX(x - d, z + d);
	GL_END();
}


#define APPLY_COLOR(index)\
{\
	switch (index)\
{		\
	case 0:\
	glColor3f(1, 1, 1); break; \
	case 1:\
	glColor3f(1, 0, 0); break; \
	case 2:\
	glColor3f(0, 1, 0); break; \
	case 3:\
	glColor3f(0, 0, 1); break; \
	default:\
	glColor3f(1, 0.5, 1); break; \
}\
}
//void LODTile::DrawNode_FRAME(int x, int z, int dx, int dy) const
//{
//
//
//	glPushAttrib(GL_COLOR);
//	APPLY_COLOR(m_rlocalPara._index_i*m_rlocalPara._N + m_rlocalPara._index_j);
//
//	GL_BEGIN(GL_LINE_STRIP);
//	GLVERTEX(x + dx, z - dy);
//	GLVERTEX(x, z);
//	GLVERTEX(x - dx, z + dy);
//	if (m_neighbor[NV_L] == VS_ACTIVE)
//		GLVERTEX(x - dx, z);
//	GLVERTEX(x - dx, z - dy);
//	if (m_neighbor[NV_D] == VS_ACTIVE)
//		GLVERTEX(x, z - dy);
//	GLVERTEX(x + dx, z - dy);
//	if (m_neighbor[NV_R] == VS_ACTIVE)
//		GLVERTEX(x + dx, z);
//	GLVERTEX(x + dx, z + dy);
//	if (m_neighbor[NV_U] == VS_ACTIVE)
//		GLVERTEX(x, z + dy);
//	GLVERTEX(x - dx, z + dy);
//	GL_END();
//	GL_BEGIN(GL_LINE_STRIP);
//	GLVERTEX(x - dx, z - dy);
//	GLVERTEX(x, z);
//	GLVERTEX(x + dx, z + dy);
//	GL_END();
//	GL_BEGIN(GL_LINE_STRIP);
//	if (m_neighbor[NV_D] == VS_ACTIVE)
//		GLVERTEX(x, z - dy);
//	GLVERTEX(x, z);
//	if (m_neighbor[NV_U] == VS_ACTIVE)
//		GLVERTEX(x, z + dy);
//	GL_END();
//	GL_BEGIN(GL_LINE_STRIP);
//	if (m_neighbor[NV_L] == VS_ACTIVE)
//		GLVERTEX(x - dx, z);
//	GLVERTEX(x, z);
//	if (m_neighbor[NV_R] == VS_ACTIVE)
//		GLVERTEX(x + dx, z);
//	GL_END();
//	glPushAttrib(GL_COLOR);
//}

//void LODTile::DrawNode_FRAME(int x, int y, int dx, int dy) const
//{
//	
//
//	glPushAttrib(GL_COLOR);
//	//left
//	if (m_neighbor[NV_L] == VS_ACTIVE)
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x - dx, y + dy);
//		GLVERTEX(x - dx, y);
//		GLVERTEX(x, y);
//		GLVERTEX(x - dx, y);
//		GLVERTEX(x - dx, y - dy);
//	}
//	else
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x - dx, y + dy);
//		GLVERTEX(x - dx, y - dy);
//	}
//	//down
//	if (m_neighbor[NV_D] == VS_ACTIVE)
//	{
//
//		GLVERTEX(x, y);
//		GLVERTEX(x - dx, y - dy);
//		GLVERTEX(x, y - dy);
//		GLVERTEX(x, y);
//		GLVERTEX(x, y - dy);
//		GLVERTEX(x + dx, y - dy);
//	}
//	else
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x - dx, y - dy);
//		GLVERTEX(x + dx, y - dy);
//	}
//	//right
//	if (m_neighbor[NV_R] == VS_ACTIVE)
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x + dx, y - dy);
//		GLVERTEX(x + dx, y);
//		GLVERTEX(x, y);
//		GLVERTEX(x + dx, y);
//		GLVERTEX(x + dx, y + dy);
//
//	}
//	else
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x + dx, y - dy);
//		GLVERTEX(x + dx, y + dy);
//	}
//	//up
//	if (m_neighbor[NV_U] == VS_ACTIVE)
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x, y + dy);
//		GLVERTEX(x - dx, y + dy);
//		GLVERTEX(x, y);
//		GLVERTEX(x + dx, y + dy);
//		GLVERTEX(x, y + dy);
//	}
//	else
//	{
//		GLVERTEX(x, y);
//		GLVERTEX(x + dx, x + dy);
//		GLVERTEX(x - dx, y + dy);
//	}
//}


void LODTile::DrawNode_FRAME(int x, int y, int dx, int dy) const
{

	GL_BEGIN(GL_LINE_LOOP);
	GLVERTEX(x, y);
	GLVERTEX(x - dx, y + dy);
	GLVERTEX(x - dx, y - dy);
	GL_END();
	if (m_neighbor[NV_L] == VS_ACTIVE)
	{
		GLVERTEX(x - dx, y);
		GLVERTEX(x - dx, y + dy);
		GLVERTEX(x - dx, y - dy);
	}
	GL_BEGIN(GL_LINE_LOOP);
	GLVERTEX(x, y);
	GLVERTEX(x - dx, y - dy);
	GLVERTEX(x + dx, y - dy);
	GL_END();

	if (m_neighbor[NV_D] == VS_ACTIVE)
	{
		GLVERTEX(x, y - dy);
		GLVERTEX(x - dx, y - dy);
		GLVERTEX(x + dx, y - dy);
	}
	GL_BEGIN(GL_LINE_LOOP);
	GLVERTEX(x, y);
	GLVERTEX(x + dx, y - dy);
	GLVERTEX(x + dx, y + dy);
	GL_END();
	if (m_neighbor[NV_R] == VS_ACTIVE)
	{
		GLVERTEX(x, y);
		GLVERTEX(x + dx, y - dy);
		GLVERTEX(x + dx, y + dy);
	}
	GL_BEGIN(GL_LINE_LOOP);
	GLVERTEX(x, y);
	GLVERTEX(x + dx, y + dy);
	GLVERTEX(x - dx, y + dy);
	{
		GLVERTEX(x, y);
		GLVERTEX(x - dx, y + dy);
		GLVERTEX(x + dx, y + dy);
	}
	GL_END();

}

void LODTile::DrawPrim_FILL(int x, int z) const
{

	DrawNode_FILL(x, z, 1, 1);
}


void LODTile::DrawPrim_FRAME(int x, int z) const
{
	DrawNode_FRAME(x, z, 1, 1);
}
Range LODTile::getLocalRange()
{
	return m_rlocalPara;
}
