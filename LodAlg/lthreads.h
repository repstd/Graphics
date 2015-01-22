#include "stdafx.h"
#include <OpenThreads\Thread>
#include <osg\GraphicsContext>
#include "Matrix.h"
#include "ltiles.h"
#include "linput.h"
#ifndef TileThread
#define TileThread TileThreadW
#else
#pragma error( "TileThread Macro Not Avaliable." )

#endif

class TileThreadP : public OpenThreads::Thread
{
public:
	TileThreadP();

public:
	virtual void run() const;
	void init(BYTE* heightMat, const Range globalRange, const Range localRange);
	void init(heightField* input, int i, int j, int N);
	void updateCameraInfo(osg::Vec3d& eye) const;
	void updateCameraInfo(osg::Vec3d& eye, osg::GLBeginEndAdapter& gl, osg::State* stat) const;
	void BFSRender() const;
	int getHeight(int x, int y);
	Range getLocalRange();
private:
	mutable LODTile m_pTile;
};

class TileThreadW
{
	enum TILETHREAD_status
	{
		//Not ready.
		TTH_NREADY,
		//Running.
		TTH_RUNNING,
		//Stoped.
		TTH_ENDED

	};
public:
	TileThreadW();
public:
	virtual void run();
	void init(BYTE* heightMat, const Range globalRange, const Range localRange);
	void init(heightField* input, int i, int j, int N);
	void updateCameraInfo(osg::Vec3d& eye) const;
	void updateCameraInfo(osg::Vec3d& eye, osg::GLBeginEndAdapter& gl, osg::State* stat) const;
	static void EventLoop(void * para);

	int getStatus();
	const bool isRunning();
	void BFSRender() const;
	void DrawIndexedPrimitive() const { m_pTile.DrawIndexedPrimitive(); }
	int getHeight(int x, int y);
	Range getLocalRange();
private:
	mutable LODTile m_pTile;
	mutable HANDLE m_hThread;
};
