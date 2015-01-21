#include "stdafx.h"
#include "lthreads.h"


static int m_status = -1;

TileThreadW::TileThreadW()
{
	m_status = TTH_NREADY;
}
void TileThreadW::run()
{

	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EventLoop, (void*)(&m_pTile), 0, NULL);

	return;

}
void TileThreadW::init(BYTE* heightMat, const Range globalRange, const Range localRange)
{
	m_pTile.init(heightMat, globalRange, localRange);
}
void TileThreadW::updateCameraInfo(osg::Vec3d& eye) const
{

	m_pTile.updateCameraInfo(eye);
}

void TileThreadW::updateCameraInfo(osg::Vec3d& eye, osg::GLBeginEndAdapter& gl, osg::State* stat) const
{

	m_pTile.updateCameraInfo(eye, gl, stat);
}
void TileThreadW::EventLoop(void * para)
{

	m_status = TTH_RUNNING;
	LODTile* pTile = (LODTile*)para;
	pTile->BFSRender();
	m_status = TTH_ENDED;
}
int TileThreadW::getStatus()
{
	return m_status;
}
const bool TileThreadW::isRunning()
{
	return ([](int status){ return status == TileThreadW::TTH_RUNNING; })(getStatus());
}
void TileThreadW::BFSRender() const
{
	m_status = TTH_RUNNING;
	m_pTile.BFSRender();
	m_status = TTH_ENDED;
}

int TileThreadW::getHeight(int x, int y)
{

	return m_pTile.GetAveHeight(x, y);
}
