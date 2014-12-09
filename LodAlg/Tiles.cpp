#include "StdAfx.h"
#include "Tiles.h"
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
#define MAX_DIS 700.0
#define _DEBUG_FILENAME_ "./pos.txt"
#define _DEBUG_ENCODE_MSG(filename,format,data) \
{\
	FILE* fp = fopen(filename, "a+"); \
	char buf[MAX_PATH]; \
	sprintf(buf, format, data); \
	fwrite(buf, strlen(buf), 1, fp); \
	fclose(fp); \
}



#define _DEBUG_LOG_INIT(filename)\
{\
	FILE* fp = fopen(filename, "w"); \
	fclose(fp); \
}

#define DEBUG_LOG_INIT _DEBUG_LOG_INIT(_DEBUG_FILENAME_)

#define DEBUG_ENCODE_MSG(format,data,...) _DEBUG_ENCODE_MSG(_DEBUG_FILENAME_,format,data)

LODTile::LODTile()
{
}


void LODTile::initParams()
{
	m_fC = 3;
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

	//	m_maxlevel = -2;
	for (int i = 0; sizeX&&sizeY; i++)
	{
		this->m_delta[i]._x = sizeX / 2;
		this->m_delta[i]._y = sizeY / 2;
		sizeX = sizeX >> 1;
		sizeY = sizeY >> 1;
		//		m_maxlevel++;
	}

}

void LODTile::init(BYTE* heightMat, const Range globalRange, const Range localRange)
{



	m_rlocalPara = localRange;
	m_rglobalPara = globalRange;
	int offsetX = localRange._centerX - ((localRange._width - 1) / 2);
	int offsetY = localRange._centerY-((localRange._height - 1) / 2);
	m_HMMatrix.Reset(m_rlocalPara._width, m_rlocalPara._height);

	m_HMMatrix.SetData(heightMat, offsetX, offsetY, globalRange._width, globalRange._height, localRange._width, localRange._height);


	initParams();


}
void LODTile::updateCameraInfo(osg::Vec3d& eye)
{
	m_ViewX = eye.x();
	m_ViewY= -1.0*eye.z();
	m_ViewZ = eye.y();

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
	int cx = (m_rlocalPara._width-1)/2+1;
	int cy = (m_rlocalPara._height - 1) / 2 + 1;

	LNODE node(cx, cy, 0);
	cur_Queue->push_back(node);
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

}


int  LODTile::GetHeight(int x, int z) const
{

	return (m_HMMatrix(x, z)*m_fScale);

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
	lx = x; ly = y; lz = z;
	local2Global(lx, ly, lz);
	VECTOR observeVec = VECTOR(lx - m_ViewX, ly - m_ViewY, lz- m_ViewZ);


	float fViewDistance = observeVec.length();
	float f;
	float cosAngle = 0.0;

	if (x - d >= 0 && x - d < m_rlocalPara._width && y - d >= 0 && y - d < m_rlocalPara._height)
	{
		observeVec.normalize();
		VECTOR normal = getNormal(x, y, d, d);


		cosAngle = normal*observeVec;
		//DEBUG_ENCODE_MSG("%f\n",angle);
		//printf("(%f,%f,%f),(%f,%f,%f)\n", normal._x,normal._y,normal._z,observeVec._x,observeVec._y,observeVec._z);

	}

	//if (abs(cosAngle) < 0.7)
	//	return VS_DISABLE;
	if (fViewDistance > MAX_DIS)
		return VS_DISABLE;
	f = fViewDistance / (patchSizeX*abs(cosAngle)*m_fC*max(m_fc*m_DHMatrix(x, z), 1.0f));
	//f = fViewDistance / (patchSizeX*m_fC*max(m_fc*m_DHMatrix(x, z), 1.0f));

	if (f < 0.1f)
		return VS_ACTIVE;
	else
		return VS_DISABLE;

	//if (cosAngle > 0)
	//{
	//	f = fViewDistance / (patchSizeX*abs(cosAngle)*m_fC*max(m_fc*m_DHMatrix(x, z), 1.0f));
	//	//f = fViewDistance / (patchSizeX*m_fC*max(m_fc*m_DHMatrix(x, z), 1.0f));

	//	if (f < 0.1f)
	//		return VS_ACTIVE;
	//	else
	//		return VS_DISABLE;
	//}

	//else
	//{

	//	f = fViewDistance / (patchSizeX*m_fC*max(m_fc*m_DHMatrix(x, z), 1.0f));

	//	if (f < 1.0f)
	//		return VS_ACTIVE;
	//	else
	//		return VS_DISABLE;
	//}

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
	x += (m_rlocalPara._centerX - m_rlocalPara._width - m_rglobalPara._width >> 1);
	y += (m_rlocalPara._centerY - -m_rlocalPara._height - m_rglobalPara._height >> 1);


	//x += (m_rlocalPara._centerX - m_rlocalPara._width);
	//y += (m_rlocalPara._centerY - m_rlocalPara._height);

	z -= 100;
}
inline void LODTile::GLVERTEX(int x, int y) const
{
	int z = GetHeight(x, y);
	int lx = x;
	int ly = y;
	local2Global(lx, ly, z);

	glVertex3f(lx, ly, z);

	//glVertex3f(x + m_rlocalPara._centerX - m_rglobalPara._centerY - m_rglobalPara._width >> 1,
	//			y + m_rlocalPara._centerY-m_rglobalPara._centerY-m_rglobalPara._height >> 1,
	//			GetHeight(x, y) - 100); 

}
void LODTile::DrawNode_FILL(int x, int z, int d, int dy) const
{
	glBegin(GL_TRIANGLE_FAN);
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
	glEnd();
}

void LODTile::DrawNode_FRAME(int x, int z, int dx, int dy) const
{
	glPushAttrib(GL_COLOR);
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_STRIP);
	GLVERTEX(x + dx, z - dy);
	GLVERTEX(x, z);
	GLVERTEX(x - dx, z + dy);
	if (m_neighbor[NV_L] == VS_ACTIVE)
		GLVERTEX(x - dx, z);
	GLVERTEX(x - dx, z - dy);
	if (m_neighbor[NV_D] == VS_ACTIVE)
		GLVERTEX(x, z - dy);
	GLVERTEX(x + dx, z - dy);
	if (m_neighbor[NV_R] == VS_ACTIVE)
		GLVERTEX(x + dx, z);
	GLVERTEX(x + dx, z + dy);
	if (m_neighbor[NV_U] == VS_ACTIVE)
		GLVERTEX(x, z + dy);
	GLVERTEX(x - dx, z + dy);
	glEnd();
	glBegin(GL_LINE_STRIP);
	GLVERTEX(x - dx, z - dy);
	GLVERTEX(x, z);
	GLVERTEX(x + dx, z + dy);
	glEnd();
	glBegin(GL_LINE_STRIP);
	if (m_neighbor[NV_D] == VS_ACTIVE)
		GLVERTEX(x, z - dy);
	GLVERTEX(x, z);
	if (m_neighbor[NV_U] == VS_ACTIVE)
		GLVERTEX(x, z + dy);
	glEnd();
	glBegin(GL_LINE_STRIP);
	if (m_neighbor[NV_L] == VS_ACTIVE)
		GLVERTEX(x - dx, z);
	GLVERTEX(x, z);
	if (m_neighbor[NV_R] == VS_ACTIVE)
		GLVERTEX(x + dx, z);
	glEnd();
	glPushAttrib(GL_COLOR);
}



void LODTile::DrawPrim_FILL(int x, int z) const
{
	DrawNode_FILL(x, z, 1, 1);
}


void LODTile::DrawPrim_FRAME(int x, int z) const
{
	DrawNode_FRAME(x, z, 1, 1);
}