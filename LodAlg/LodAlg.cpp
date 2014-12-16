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
#include <osgGA/StandardManipulator>
#include <osgGA/GUIEventHandler>
#include "LODManipulator.h"
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
class camDrawcallback :public osg::Drawable::DrawCallback
{

public:
	camDrawcallback()
	{	
	}

	camDrawcallback(const osg::Drawable::DrawCallback & cb, const osg::CopyOp& co)
	:osg::Drawable::DrawCallback(cb,co)
	{
	}


	virtual void drawImplementation(osg::RenderInfo& /*renderInfo*/ renderinfo, const osg::Drawable* /*drawable*/) const 
	{
		osg::ref_ptr<osg::Camera> camera = renderinfo.getCurrentCamera();
		osg::Matrixd mat= camera->getViewMatrix();
		osg::Matrixd cameraRotation;
		cameraRotation.makeRotate(
			osg::DegreesToRadians(0.0), osg::Vec3(0, 1, 0),
			osg::DegreesToRadians(0.0), osg::Vec3(1, 0, 0),
			osg::DegreesToRadians(180.0), osg::Vec3(0, 0, 1));

		camera->setViewMatrix(cameraRotation*mat);

		renderinfo.getView()->setCamera(camera);

	
	}

};

int _tmain(int argc, _TCHAR* argv[])
{

	osgViewer::Viewer viewer;


	viewer.setUpViewInWindow(50, 50, 1280, 720);

	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	
	viewer.realize();

	osg::Matrixd cameraRotation;
	cameraRotation.makeRotate(osg::DegreesToRadians(180.0), osg::Z_AXIS);



	
	viewer.getCamera()->getGraphicsContext()->makeCurrent();
	
	viewer.setDataVariance(osg::Object::DYNAMIC);

	
	LODDrawable* lod = new LODDrawable();
	lod->init(_HEIGHT_FIELD_FILE_RAW);
	lod->setName("LOD");

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(lod);
	
	geode->setCullingActive(false);
	geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc);
	geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	//osgGA::FirstPersonManipulator* manipulator = new osgGA::FirstPersonManipulator();


	LODManipulator* manipulator = new LODManipulator(lod);

	osg::Matrixd vm;
	int initZ = lod->getFieldHeight(0, lod->getLODRange()._centerX, lod->getLODRange()._centerY)-100;
	vm.makeLookAt(osg::Vec3d(0, 3, initZ), osg::Vec3d(0, 2, initZ), osg::Vec3d(0, 0, 1));
	manipulator->setByMatrix(vm);
	//manipulator->setUserData(lod);
	viewer.setCameraManipulator(manipulator);
	//viewer.setCameraManipulator(new osgGA::TerrainManipulator());

	viewer.setSceneData(geode);
	viewer.setRunMaxFrameRate(90);
	viewer.addEventHandler(new  osgViewer::StatsHandler);
	viewer.addEventHandler(new  osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new  osgViewer::HelpHandler);

	return viewer.run();
}

