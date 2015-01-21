#include "StdAfx.h"
#include "ldrawable.h"
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
#include "lmanipulator.h"
#include "linput.h"
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

LODDrawable::LODDrawable()

{
	setSupportsDisplayList(false);
	glewInit();


}

LODDrawable::~LODDrawable()
{
}


void LODDrawable::initRaw(const char* heightFiledMap)
{
	std::unique_ptr<dataImp> rawImp(dataImpFactory::instance()->create(dataImpFactory::_RAW, heightFiledMap));
	std::unique_ptr<heightField> input(new heightField(rawImp.release()));
	m_rglobalPara._width = input->getWidth();
	m_rglobalPara._height = input->getHeight();
	m_rglobalPara._centerX = input->getCenterX();
	m_rglobalPara._centerY = input->getCenterY();
	BYTE* tempHeightMap = new BYTE[m_rglobalPara._width*m_rglobalPara._height]; 
	input->generateTile(0, 0, 1, tempHeightMap, m_rglobalPara);
	int index;
	int N =1;
	for (int j = 0; j < N; j++)
	{
		for (int i = 0; i < N; i++)
		{
			index = j * N + i;
			PTileThread	pTile = new TileThread();
			Range tileRange;
			tileRange._width = tileRange._height = input->getTileWidth(i, j, N);
			tileRange._centerX = input->getTileCenterX(i, j, N);
			tileRange._centerY = input->getTileCenterY(i, j, N);
			tileRange._index_i = i;
			tileRange._index_j= j;
			tileRange._N = N;
			pTile->init(tempHeightMap, m_rglobalPara, tileRange);
			m_vecTile.push_back(std::auto_ptr<TileThread>(pTile));
			m_vecRange.push_back(tileRange);
		}
	}
	delete[] tempHeightMap;
	input.release();

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

	//TODO: fix me. We should handle the case that the terrain differs in height and width.@yulw
	m_rglobalPara._height = m_rglobalPara._width;

	m_rglobalPara._centerX =floor(float (m_rglobalPara._width) /2);
	m_rglobalPara._centerY =floor (float(m_rglobalPara._height )/2);

	BYTE* tempHeightMap = new BYTE[m_rglobalPara._width*m_rglobalPara._height]; 
	fread(tempHeightMap, 1, m_rglobalPara._width*m_rglobalPara._height, fp);

	//std::auto_ptr<TileThread> pTile[16];
	int index = 0; 
	int N = 1;
	int tileSize = (m_rglobalPara._width - 1) / N +1;
	for (int j = 0; j < N; j++)
	{

		for (int i = 0; i < N; i++)
		{

			index = j * N + i;
			PTileThread	pTile = new TileThread();
			Range tileRange;
			tileRange._width = tileRange._height = tileSize;

			tileRange._centerX = i*tileSize + (tileSize - 1) / 2;

			tileRange._centerY = j*tileSize + (tileSize - 1) / 2;
			
			tileRange._index_i = i;
			tileRange._index_j= j;
			tileRange._N = N;
			pTile->init(tempHeightMap, m_rglobalPara, tileRange);

			m_vecTile.push_back(std::auto_ptr<TileThread>(pTile));

			m_vecRange.push_back(tileRange);
		}
	}

	
	

	delete[] tempHeightMap;
	fclose(fp);

	return;

}


void LODDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	osg::GLBeginEndAdapter& gl = renderInfo.getState()->getGLBeginEndAdapter();

	osg::ref_ptr<osg::Camera> camera = renderInfo.getCurrentCamera();

	osg::Vec3d eye, at, up;
	camera->getViewMatrixAsLookAt(eye, at, up,2.0);
	_LOG_MATRIX(camera->getViewMatrix(), "drawImplementation");
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	gluLookAt(eye.x(), eye.y(), eye.z(), at.x(), at.y(), at.z(), up.x(), up.y(), up.z());
	
	int sz = m_vecTile.size();
	osg::ref_ptr<osg::DisplaySettings> ds = camera->getDisplaySettings();
	for (int i = 0; i < sz; i++)
	{
		m_vecTile[i]->updateCameraInfo(eye,gl,renderInfo.getState());
#ifdef _GL_MT
		m_vecTile[i]->run();
#else
		m_vecTile[i]->BFSRender();
#endif
		//m_vecTile[i]->start();
	}
#ifdef _GL_MT
	for (int i = 0; i < sz; i++)
	{
		m_vecTile[i]->DrawIndexedPrimitive();
	}
	int isStopped = false;
	while (1)
	{
		isStopped = true;
		for (int i = 0; i < sz; i++)
		{
			if (m_vecTile[i]->isRunning())
			{
				isStopped = false;
				break;
			}
		}
		if (isStopped)
			break;
					
	}
#endif
}


