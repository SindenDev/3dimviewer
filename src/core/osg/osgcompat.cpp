#include <osg/osgcompat.h>

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	UniDrawableList getGeodeDrawableList(osg::Geode *pGeode)
	{
		UniDrawableList list;
		if (NULL!=pGeode)
		{
			unsigned int nDrawables = pGeode->getNumDrawables();
			for(unsigned int i = 0; i < nDrawables; i++)
				list.push_back(pGeode->getDrawable(i));
		}
		return list;
	}
#else
	UniDrawableList getGeodeDrawableList(osg::Geode *pGeode)
	{
		if (NULL==pGeode)
		{
			UniDrawableList list;
			return list;
		}
		return pGeode->getDrawableList();
	}
#endif