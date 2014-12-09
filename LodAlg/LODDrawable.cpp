#include "StdAfx.h"
#include "LODDrawable.h"
#include <vector>
#include <queue>
#include <list>
#include <osg/Matrix>
#include <osgDB/ReaderWriter>
#include <stdlib.h>
#include <stdio.h>
#include <gdal.h>
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#define MAX_DIS 700.0
void saveBMP(int width, int height, int channel, BYTE* data, char* filename, int pixelFormat = GL_BGR)
{
	osg::ref_ptr<osgDB::ReaderWriter> m_bmpWirter;

	m_bmpWirter = osgDB::Registry::instance()->getReaderWriterForExtension("mp4");


	osg::ref_ptr<osg::Image> img = new osg::Image();
	img->allocateImage(width, height, channel, pixelFormat, GL_BYTE);
	BYTE* p = img->data(0, 0);
	memcpy(p, data, width*height * channel);

	m_bmpWirter->writeImage(*img, filename);
}
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

LODDrawable::LODDrawable()

{
	setSupportsDisplayList(false);
	glViewport(0, 0, 1280, 720);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)1280 / (float)720, 0, 100);

}

LODDrawable::~LODDrawable()
{
}


void LODDrawable::init(const char* filename)
{

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		return;
	}

	long sz;
	fseek(fp, 0, std::ios::end);
	sz = ftell(fp);
	fseek(fp, 0, std::ios::beg);



	m_rglobalPara._width = sqrt(double(sz));

	//TODO: fix me. We should handle the case that the terrain differs in height and width.
	m_rglobalPara._height = m_rglobalPara._width;

	m_rglobalPara._centerX = (m_rglobalPara._width - 1) >> 1;
	m_rglobalPara._centerY = (m_rglobalPara._height - 1) >> 1;


	BYTE* tempHeightMap = new BYTE[m_rglobalPara._width*m_rglobalPara._height];
	fread(tempHeightMap, 1, m_rglobalPara._width*m_rglobalPara._height, fp);

	PTileThread pTile = new TileThread;
	
	pTile->init(tempHeightMap, m_rglobalPara, m_rglobalPara);

	
	m_vecTile.push_back(pTile);

	m_vecRange.push_back(m_rglobalPara);

	delete[] tempHeightMap;
	fclose(fp);

	return;

}
void LODDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	osg::ref_ptr<osg::Camera> camera = renderInfo.getCurrentCamera();

	osg::Vec3d eye, at, up;

	camera->getViewMatrixAsLookAt(eye, at, up);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	gluLookAt(eye.x(), eye.y(), eye.z(), at.x(), at.y(), at.z(), up.x(), up.y(), up.z());

	int sz = m_vecTile.size();
	for (int i = 0; i < sz; i++)
	{
		//m_vecTile[i].updateCameraInfo(eye);
		////m_vecTile[i].BFSRender();

		//m_vecTile[i].start();
		m_vecTile[i]->updateCameraInfo(eye);
		m_vecTile[i]->startThread();
	}
	printf("Rendring Ended.\n");
}


