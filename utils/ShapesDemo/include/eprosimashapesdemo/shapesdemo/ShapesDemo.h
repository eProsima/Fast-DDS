/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapesDemo.h
 *
 */

#ifndef SHAPESDEMO_H_
#define SHAPESDEMO_H_
#include "eprosimartps/rtps_all.h"
#include "eprosimashapesdemo/shapesdemo/ShapeTopicDataType.h"
#include <QMutex>

class ShapesDemoOptions
{
public:
    uint32_t m_updateIntervalMs;
    uint32_t m_movementSpeed;
    uint32_t m_domainId;
    ShapesDemoOptions()
    {
        m_updateIntervalMs = 200;
        m_movementSpeed = 5;
        m_domainId = 80;
    }
    ~ShapesDemoOptions()
    {

    }
};


class ShapePublisher;
class ShapeSubscriber;
class Shape;
class MainWindow;

class ShapesDemo
{
public:
    ShapesDemo(MainWindow* mw);
	~ShapesDemo();
	bool init();
	void stop();
	Participant* getParticipant();


	void addPublisher(ShapePublisher* SP);
    void addSubscriber(ShapeSubscriber* SS);

    uint32_t getRandomX(uint32_t size=10);
    uint32_t getRandomY(uint32_t size=10);

     bool getShapes(std::vector<Shape*>* shvec);

     QMutex* getMutex()
     {
         return &m_mutex;
     }

     void moveAllShapes();
     void writeAll();

     void setOptions(ShapesDemoOptions& opt);


private:
	std::vector<ShapePublisher*> m_publishers;
    std::vector<ShapeSubscriber*> m_subscribers;
	Participant* mp_participant;

    //std::vector<ShapeType*> m_shapes;
    bool m_isInitialized;

    uint32_t minX,minY,maxX,maxY;


    void moveShape(Shape* sh);
    void getNewDirection(Shape* sh);

    QMutex m_mutex;
    ShapeTopicDataType m_shapeTopicDataType;
    ShapesDemoOptions m_options;
    MainWindow* m_mainWindow;
};



#endif /* SHAPESDEMO_H_ */
