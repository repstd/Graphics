// LodAlg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ldrawable.h"
#include "ltiles.h"
#include <stdlib.h>
#include <iostream>
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/CameraManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/FirstPersonManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/StandardManipulator>
#include <osgGA/GUIEventHandler>
#include "lmanipulator.h"
#include "linput.h"
#include <osg/Node>
#include <osg/BlendFunc>
#include <OpenThreads\Thread>
#include "rcfactory.h"
#define _HEIGHT_FIELD_FILE_RAW "./data/terrain.raw"
#define _HEIGHT_FIELD_FILE_SRTM "./data/srtm_ramp2_world_5400x2700_2.jpg"
#define _HEIGHT_FIELD_FILE_PUGET_ASC "E://1.MyDocuments//LOD//MSR_Hoppe_PM//psdem_2005//psdem//psdem_2005.asc"
rcpipeServer* g_pipeServer;
OVERLAPPED g_oOverlap;
rcFileMapWriter*  g_writer;
rcFileMapReader*  g_reader;
	//initMemShareWriter();
	//@yulw,pipe-test,2015-4-5
	//writeToMemShare(camera->getViewMatrix(), 64);
	//readFromMemShareAndSyncMatrix(camera.get());

void initPipeServer()
{
	g_pipeServer = new rcpipeServer(_RC_PIPE_NAME);
	HANDLE hEvent = CreateEvent(
		NULL,    // default security attribute 
		TRUE,    // manual-reset event 
		TRUE,    // initial state = signaled 
		NULL);   // unnamed event object 
	if (hEvent == NULL)
	{
		printf("CreateEvent failed with %d.\n", GetLastError());
	}
	g_oOverlap.hEvent = hEvent;

}
void writeToPipe(void* data, DWORD sizeToWrite)
{
	DWORD sizeWritten=-1;
	if (g_pipeServer->getHandle() != NULL)
	{
		g_pipeServer->writeto(data, sizeToWrite, sizeWritten, &g_oOverlap);
	}
}
void initMemShareWriter()
{
	//char* name ="Master";
	char* name = "MemShare";
	char* info = "initMemShareWriter";
	printf("%s\n", info);
	g_writer = rcfactory::instance()->createFileMapWriter(name);
}
void initMemShareReader()
{
	//char* name = "Slave";
	//char* name = "Slave";
	char* name = "MemShare";
	char* info = "initMemShareReader";
	printf("%s\n", info);
	g_reader = rcfactory::instance()->createFileMapReader(name);
}

void encodeMsg(osg::ref_ptr<osgViewer::Viewer> viewer, void* msg)
{
	osg::Matrix modelview(viewer->getCamera()->getViewMatrix());
	osg::Matrix projection(viewer->getCamera()->getProjectionMatrix());
	osg::Matrix maniMatrix(viewer->getCameraManipulator()->getMatrix());
	SYNC_OSG_MSG* pMsg = reinterpret_cast<SYNC_OSG_MSG*>(msg);
	pMsg->_eventSize = 0;
	memcpy(pMsg->_matrix, maniMatrix.ptr(), 16 * sizeof(double));
	memcpy(pMsg->_modelView, modelview.ptr(), 16 * sizeof(double));
	memcpy(pMsg->_projection, projection.ptr(), 16 * sizeof(double));
}
void decodeMsg(osgViewer::Viewer* viewer, void *msg)
{
	SYNC_OSG_MSG* pMsg = reinterpret_cast<SYNC_OSG_MSG*>(msg);
	osg::Matrix modelView(pMsg->_modelView);
	osg::Matrix projection(pMsg->_projection);
	osg::Matrix mani(pMsg->_matrix);
	viewer->getCamera()->setViewMatrix(modelView);
	viewer->getCamera()->setProjectionMatrix(projection);
	viewer->getCameraManipulator()->setByMatrix(mani);
}
void writeToMemShare(osgViewer::Viewer* viewer)
{
	SYNC_OSG_MSG msg;
	encodeMsg(viewer, &msg);
	g_writer->write(&msg, _MAX_OSG_DATA_SIZE);
}
char buf[_MAX_OSG_DATA_SIZE];

void readFromMemShareAndSyncMatrix(osgViewer::Viewer* viewer)
{
	g_reader->read(buf, sizeof(SYNC_OSG_MSG));
	decodeMsg(viewer, buf);
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
class errorHandler : public osg::NotifyHandler
{
public:
	errorHandler()
	{

	}
	errorHandler(const std::string& filename)
	{
	}
	void notify(osg::NotifySeverity severity, const char* message)
	{

	}
protected:
	~errorHandler(void) {

	}
};
int _tmain(int argc, _TCHAR* argv[])
{
//#undef __MASTER
#ifdef __MASTER
	initMemShareWriter();
#else
	initMemShareReader();
#endif
	osg::ref_ptr<osgViewer::Viewer> viewer=new osgViewer::Viewer;
	viewer->setUpViewInWindow(50, 50, 1280, 720);
	viewer->realize();
	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
	osg::Matrixd cameraRotation;
	cameraRotation.makeRotate(osg::DegreesToRadians(180.0), osg::Z_AXIS);
	viewer->getCamera()->getGraphicsContext()->makeCurrent();
	viewer->setDataVariance(osg::Object::DYNAMIC);
	
	std::unique_ptr<dataImp> rawdata(dataImpFactory::instance()->createRawImp(_HEIGHT_FIELD_FILE_RAW));
	//std::unique_ptr<dataImp> gdaldata(dataImpFactory::instance()->createGDALImp(_HEIGHT_FIELD_FILE_SRTM, "GeoTiff"));
	std::unique_ptr<heightField> input(new heightField(rawdata.release()));
	LODDrawable* lod = new LODDrawable(lodImpFactory::instance()->createQuadTreeImp());
	lod->init(input.release());
	//lod->init(_HEIGHT_FIELD_FILE_RAW);
	lod->setName("LOD");

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(lod);
	
	geode->setCullingActive(false);
	geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc);
	geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	//osgGA::FirstPersonManipulator* manipulator = new osgGA::FirstPersonManipulator();


	Manipulator* manipulator = new Manipulator(lod);

	osg::Matrixd vm;
	int initZ = lod->getFieldHeight(0, lod->getLODRange()._centerX, lod->getLODRange()._centerY)-100;
	vm.makeLookAt(osg::Vec3d(0, 3, initZ), osg::Vec3d(0, 2, initZ), osg::Vec3d(0, 0, 1));
	manipulator->setByMatrix(vm);

	//manipulator->setUserData(lod);
	viewer->setCameraManipulator(manipulator);
	viewer->getCamera()->setCullingMode(osg::CullSettings::FAR_PLANE_CULLING);
	viewer->setSceneData(geode);
	viewer->setRunMaxFrameRate(90);
	viewer->addEventHandler(new  osgViewer::StatsHandler);
	viewer->addEventHandler(new  osgViewer::WindowSizeHandler);
	viewer->addEventHandler(new  osgViewer::HelpHandler);
	//osg::setNotifyHandler(new errorHandler);
	while (!viewer->done())
	{
		viewer->advance();
#ifdef __MASTER
		writeToMemShare(viewer);
#else
		readFromMemShareAndSyncMatrix(viewer);
#endif
		viewer->frame();
		viewer->eventTraversal();
		viewer->updateTraversal();
		viewer->renderingTraversals();
	}
	return 0;
}

