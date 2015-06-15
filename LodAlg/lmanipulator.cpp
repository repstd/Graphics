#include "StdAfx.h"
#include "lmanipulator.h"
#include <osgGA/GUIActionAdapter>

Manipulator::Manipulator(LODDrawable* lod)

{

	_DEBUG_LOG_INIT_MANI
	m_LOD = lod;

	Range rg = m_LOD->getLODRange();
	m_posX = rg._centerX;
	m_posY = rg._centerY;
	m_posZ = m_LOD->getFieldHeight(0, m_posX, m_posY);
	m_velocityX = 5;
	m_velocityY = 5;

}


Manipulator::~Manipulator()
{

}
bool Manipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{

	
	osg::Quat rot;
	osg::Vec3d trans;
	osg::Matrixd tl;
	osg::Vec3d angle=_rotation.asVec3();
	float x = m_posX*cos(angle[2]) - m_posY*sin(angle[2]);
	float y = m_posY*cos(angle[2]) + m_posX*sin(angle[2]);
	float z = m_posZ;
	float tx=0, ty=0;
	auto decompose= [](float mv, float angle,float& tx,float& ty)
	 {
		tx = mv*cos(angle);
		ty = mv*sin(angle);
	 };
	_DEBUG_ENCODE_MSG_MANI("rotate: %f,%f,%f\n", angle[0], angle[1], angle[2]);
	//auto move = []{

	//};
	switch (ea.getEventType())
	{
	
	case osgGA::GUIEventAdapter::KEYDOWN:

		switch (ea.getKey())
		{
		case osgGA::GUIEventAdapter::KEY_A:
			decompose(m_velocityX, angle[2], tx, ty);
			m_posX += tx;
			m_posY += ty;

			//trans.set(osg::Vec3d(m_velocityX, 0, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));
			trans.set(osg::Vec3d(tx, ty, m_LOD->getFieldHeight(0, m_posX, m_posY) - m_posZ));

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

		_eye += trans;
		m_posZ = m_LOD->getFieldHeight(0, m_posX, m_posY);

		break;


	default:
		return osgGA::FirstPersonManipulator::handle(ea, us);
	}
	
	return false;


}
