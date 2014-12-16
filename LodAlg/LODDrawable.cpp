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
#include "LODManipulator.h"
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

	//TODO: fix me. We should handle the case that the terrain differs in height and width.@yulw
	m_rglobalPara._height = m_rglobalPara._width;

	m_rglobalPara._centerX =floor(float (m_rglobalPara._width) /2);
	m_rglobalPara._centerY =floor (float(m_rglobalPara._height )/2);

	BYTE* tempHeightMap = new BYTE[m_rglobalPara._width*m_rglobalPara._height]; 
	fread(tempHeightMap, 1, m_rglobalPara._width*m_rglobalPara._height, fp);


	Range tileRange[16];
	TileThread pTile[16]; 
	int index = 0; 
	int N = 2;
	int tileSize = (m_rglobalPara._width - 1) / N +1;
	for (int j = 0; j < N; j++)
	{

		for (int i = 0; i < N; i++)
		{

			index = j * N + i;
			//pTile[index] = new TileThread;

			tileRange[index]._width = tileRange[index]._height = tileSize;

			tileRange[index]._centerX = i*tileSize + (tileSize - 1) / 2;

			tileRange[index]._centerY = j*tileSize + (tileSize - 1) / 2;
			
			tileRange[index]._index_i = i;
			tileRange[index]._index_j= j;
			tileRange[index]._N = N;
			pTile[index].init(tempHeightMap, m_rglobalPara, tileRange[index]);

			m_vecTile.push_back(pTile[index]);

			m_vecRange.push_back(tileRange[index]);
		}
	}


	

	delete[] tempHeightMap;
	fclose(fp);

	return;

}


void LODDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	osg::ref_ptr<osg::Camera> camera = renderInfo.getCurrentCamera();

	osg::Vec3d eye, at, up;

	camera->getViewMatrixAsLookAt(eye, at, up,2.0);
	
	_LOG_MATRIX(camera->getViewMatrix(), "drawImplementation");
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	gluLookAt(eye.x(), eye.y(), eye.z(), at.x(), at.y(), at.z(), up.x(), up.y(), up.z());
	int sz = m_vecTile.size();
	for (int i = 0; i < sz; i++)
	{
		//m_vecTile[i].start();
		m_vecTile[i].updateCameraInfo(eye);
		m_vecTile[i].BFSRender();
	}
}


