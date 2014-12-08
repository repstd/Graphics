// LodAlg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LODDrawable.h"
#include "LODTiles.h"
#include <osgViewer/Viewer>
#include <osgViewer/View>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/CameraManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/FirstPersonManipulator>
#include <osgGA/TerrainManipulator>

#include <osgGA/GUIEventHandler>
#include <osg/Node>
#include <osg/BlendFunc>
#include <OpenThreads\Thread>
#define _HEIGHT_FIELD_FILE_RAW "./data/terrain.raw"
#define _HEIGHT_FIELD_FILE_SRTM "./data/srtm_ramp2_world_5400x2700_2.jpg"
class thread:
	public OpenThreads::Thread,LODTile
{
public:
	thread() :OpenThreads::Thread(), LODTile()
	{

	}
	virtual void run()
	{
		this->BFSRender();
		
	}
private:
	int x;
	mutable int y;
	int f() const
	{
		return 0;
	}

};

void test()
{


}


int _tmain(int argc, _TCHAR* argv[])
{



	//thread* th = new thread();
	//th->Init();
	//th->start();
	//return 0;
	osgViewer::Viewer viewer;


	viewer.setUpViewInWindow(50, 50, 1280, 720);

	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	
	viewer.realize();
	viewer.getCamera()->getGraphicsContext()->makeCurrent();

	osg::Matrixd mat = viewer.getCamera()->getViewMatrix();


	LODDrawable* lod = new LODDrawable();
	lod->init(_HEIGHT_FIELD_FILE_RAW);
	lod->setName("LOD");
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(lod);
	geode->setCullingActive(false);
	geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc);
	geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	viewer.setCameraManipulator(new osgGA::TerrainManipulator());
	viewer.setSceneData(geode);
	viewer.setRunMaxFrameRate(90);
	viewer.addEventHandler(new  osgViewer::StatsHandler);
	viewer.addEventHandler(new  osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new  osgViewer::HelpHandler);

	return viewer.run();
}

