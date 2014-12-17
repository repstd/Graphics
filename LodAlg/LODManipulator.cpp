#include "StdAfx.h"
#include "LODManipulator.h"
#include "LODDrawable.h"
using osgGA::GUIEventAdapter;

LODManipulator::LODManipulator()
{
	initPara();
}
LODManipulator::LODManipulator(LODDrawable* lod)
{
	m_LOD = lod;
	initPara();
}

LODManipulator::~LODManipulator()
{
}
void LODManipulator::initPara()
{


	 _DEBUG_LOG_INIT_MANI
	
	m_viewMatrix.makeIdentity();
	m_RotMatrix.makeIdentity();
	m_TransformMatrix.makeIdentity();
	//osg::ref_ptr<LODDrawable> lodd = (LODDrawable*)getUserData();
		
	//_DEBUG_ENCODE_MSG_MANI("%s\n", m_LOD->getName());
	m_velocityX = 20;
	m_velocityY = 20;
	Range rg = m_LOD->getLODRange();
	m_posX = rg._centerX;
	m_posY = rg._centerY;
	m_posZ = m_LOD->getFieldHeight(0, m_posX, m_posY);

	//_DEBUG_ENCODE_MSG_MANI("initPara %d,%d,%d,%d,%d\n", m_posX,m_posY,m_posZ,m_velocityX,m_velocityY);

}
void LODManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	m_viewMatrix.set(matrix);
}

void LODManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{

}

osg::Matrixd LODManipulator::getMatrix() const
{

	_LOG_MATRIX(m_viewMatrix,"getMatrix");

	return m_viewMatrix;
}

osg::Matrixd LODManipulator::getInverseMatrix() const
{
	_LOG_MATRIX(getMatrix(), "getInverseMatrix");


	return osg::Matrix::inverse(m_viewMatrix);

}

bool LODManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	//us.asView()->ref();

	osg::Matrixd mat = us.asView()->getCamera()->getViewMatrix();

	_LOG_MATRIX(mat, "handle");

	switch (ea.getEventType())
	{
	case GUIEventAdapter::MOVE:
		return handleMouseMove(ea, us);

	case GUIEventAdapter::DRAG:
		return handleMouseDrag(ea, us);

	//case GUIEventAdapter::PUSH:
	//	return handleMousePush(ea, us);

	//case GUIEventAdapter::RELEASE:
	//	return handleMouseRelease(ea, us);

	case GUIEventAdapter::KEYDOWN:
		return handleKeyDown(ea, us);

	case GUIEventAdapter::KEYUP:
		return handleKeyUp(ea, us);



	default:
		return false;
	}
	return false;

}


bool LODManipulator::handleMouseMove(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	
	return true;
}
bool LODManipulator::handleMouseDrag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	osg::Matrixd rot;
	rot.makeRotate(osg::RadiansToDegrees(5.0), osg::Z_AXIS);
	m_viewMatrix.set(m_viewMatrix*rot);
	return true;

}
bool LODManipulator::handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	return true;

}
bool LODManipulator::handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	return true;

}
bool LODManipulator::handleKeyDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	_DEBUG_ENCODE_MSG_MANI("handleKeyDown %d,%d,%d\n",m_posX,m_posY,m_posZ);
	osg::Vec3d trans; 
	int x = m_posX, y = m_posY, z = m_posZ;
	switch (ea.getKey())
	{
	case osgGA::GUIEventAdapter::KEY_A:

		m_posX += m_velocityX;

		trans.set(osg::Vec3d(m_velocityX, 0, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));

		break;
	case osgGA::GUIEventAdapter::KEY_D:

		m_posX -= m_velocityX;
		trans.set(osg::Vec3d(-1.0*m_velocityX, 0, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));
		break;

	case osgGA::GUIEventAdapter::KEY_X:

		m_posY += m_velocityY;
		trans.set(osg::Vec3d(0, m_velocityY, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));
		break;
	case osgGA::GUIEventAdapter::KEY_W:

		m_posY -= m_velocityY;
		trans.set(osg::Vec3d(0, -1.0* m_velocityY, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));

		break;
	default:
		break;
	}
	if (m_posX < 0 || m_posX >= m_LOD->getLODRange()._width || m_posY < 0 || m_posY >= m_LOD->getLODRange()._height)
	{
		m_posX = x;
		m_posY = y;
		m_posZ = z;
		return true;
	}

	osg::Matrixd tl;
	tl.makeTranslate(trans);

	_LOG_MATRIX(tl, "translate\n");
	int N = m_LOD->getLODRange()._N;
	int tileX = (m_LOD->getLODRange()._width-1) / N+1;
	int tileY = (m_LOD->getLODRange()._width-1) / N+1;

	m_posZ = m_LOD->getFieldHeight(0, m_posX, m_posY);
	m_viewMatrix.set(m_viewMatrix*tl);
	return true;

}
bool LODManipulator::handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	return true;

} 