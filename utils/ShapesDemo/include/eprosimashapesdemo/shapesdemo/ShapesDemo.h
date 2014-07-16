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

#define MAX_DRAW_AREA_X 235
#define MAX_DRAW_AREA_Y 265
#define INITIAL_INTERVAL_MS 200


class ShapesDemoOptions
{
public:
    uint32_t m_updateIntervalMs;
    uint32_t m_movementSpeed;
    uint32_t m_domainId;
    ShapesDemoOptions()
    {
        m_updateIntervalMs = INITIAL_INTERVAL_MS;
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
    friend class DrawArea;
public:
    ShapesDemo(MainWindow* mw);
	~ShapesDemo();
	bool init();
	void stop();
	Participant* getParticipant();


	void addPublisher(ShapePublisher* SP);
    void addSubscriber(ShapeSubscriber* SS);
    void removePublisher(ShapePublisher* SP);
    void removeSubscriber(ShapeSubscriber* SS);

    uint32_t getRandomX(uint32_t size=10);
    uint32_t getRandomY(uint32_t size=10);

     bool getShapes(std::vector<Shape*>* shvec);

//     QMutex* getMutex()
//     {
//         return &m_mutex;
//     }

     void moveAllShapes();
     void writeAll();

     void setOptions(ShapesDemoOptions& opt);
     ShapesDemoOptions getOptions();

     bool isInitialized(){return this->m_isInitialized;}


private:
	std::vector<ShapePublisher*> m_publishers;
    std::vector<ShapeSubscriber*> m_subscribers;
	Participant* mp_participant;

    //std::vector<ShapeType*> m_shapes;
    bool m_isInitialized;

    uint32_t minX,minY,maxX,maxY;


    void moveShape(Shape* sh);
    void getNewDirection(Shape* sh);

    ShapeTopicDataType m_shapeTopicDataType;
    ShapesDemoOptions m_options;
    MainWindow* m_mainWindow;
    QMutex m_mutex;
};



#endif /* SHAPESDEMO_H_ */
