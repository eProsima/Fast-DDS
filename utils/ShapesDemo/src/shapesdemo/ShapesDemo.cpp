/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include <iostream>
#include <sstream>
#include <ctime>

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include "eprosimashapesdemo/qt/mainwindow.h"

#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/Domain.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/subscriber/Subscriber.h"

ShapesDemo::ShapesDemo(MainWindow *mw):
    mp_participant(nullptr),
    m_isInitialized(false),
    minX(0),minY(0),maxX(0),maxY(0),
    m_mainWindow(mw),
    m_mutex(QMutex::Recursive)
{
    srand (time(nullptr));
    minX = 0;
    minY = 0;
    maxX = MAX_DRAW_AREA_X;
    maxY = MAX_DRAW_AREA_Y;
}

ShapesDemo::~ShapesDemo()
{
    stop();
}

Participant* ShapesDemo::getParticipant()
{
    if(m_isInitialized && mp_participant !=nullptr)
        return mp_participant;
    else
    {
        if(init())
            return mp_participant;
    }
    return nullptr;
}

bool ShapesDemo::init()
{
    cout << "Initializing ShapesDemo "<< m_isInitialized <<endl;
    if(!m_isInitialized)
    {
        cout <<"Creating new Participant"<<endl;
        ParticipantAttributes pparam;
        pparam.rtps.setName("fastrtpsParticipant");
        pparam.rtps.builtin.domainId = m_options.m_domainId;
        pparam.rtps.builtin.leaseDuration.seconds = 100;
        pparam.rtps.builtin.leaseDuration_announcementperiod.seconds = 50;
        pparam.rtps.defaultSendPort = 10042;
        pparam.rtps.sendSocketBufferSize = 65536;
        pparam.rtps.listenSocketBufferSize = 2*65536;
        pparam.rtps.use_IP6_to_send = false;
        pparam.rtps.use_IP4_to_send = true;
        mp_participant = Domain::createParticipant(pparam);
        if(mp_participant!=nullptr)
        {
            // cout << "RTPSParticipant Created "<< mp_participant->getGuid() << endl;l
            m_isInitialized = true;
            Domain::registerType(mp_participant,&m_shapeTopicDataType);
            return true;
        }
        return false;
    }
    return true;
}

void ShapesDemo::stop()
{
    if(m_isInitialized)
    {
        QMutexLocker lock(&m_mutex);
        this->m_mainWindow->quitThreads();
        for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
            it!=m_publishers.end();++it)
        {
            delete(*it);
        }
        m_publishers.clear();
        for(std::vector<ShapeSubscriber*>::iterator it = m_subscribers.begin();
            it!=m_subscribers.end();++it)
        {
            delete(*it);
        }
        m_subscribers.clear();
        Domain::removeParticipant(mp_participant);
        cout << "All Stoped, removing"<<endl;
        mp_participant = nullptr;
        m_isInitialized = false;
    }
}

void ShapesDemo::addPublisher(ShapePublisher* SP)
{
    m_publishers.push_back(SP);
    this->m_mainWindow->addPublisherToTable(SP);
}

void ShapesDemo::addSubscriber(ShapeSubscriber* SSub)
{
    m_subscribers.push_back(SSub);
    this->m_mainWindow->addSubscriberToTable(SSub);
}

uint32_t ShapesDemo::getRandomX(uint32_t size)
{
    return minX+size+(uint32_t)(((maxX-size)-(minX+size))*((double) rand() / (RAND_MAX)));
}

uint32_t ShapesDemo::getRandomY(uint32_t size)
{
    return minY+size+(uint32_t)(((maxY-size)-(minY+size))*((double) rand() / (RAND_MAX)));
}

void ShapesDemo::moveAllShapes()
{
    for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        (*it)->m_mutex.lock();
        moveShape(&(*it)->m_shape);
        (*it)->m_mutex.unlock();
    }
}

void ShapesDemo::moveShape(Shape* sh)
{
    //TODO ESTAS DOS LINEAS NO SON NECESARIAS.
    if(sh->m_changeDir)
        getNewDirection(sh);
    //Apply movement
    int nx = sh->m_x + m_options.m_movementSpeed*sh->m_dirX;
    int ny = sh->m_y + m_options.m_movementSpeed*sh->m_dirY;
    //Check if the movement is correct
    bool cond1 = nx+(int)sh->m_size/2 > (int)maxX;
    bool cond2 = nx-(int)sh->m_size/2 < (int)minX;
    bool cond3 = ny+(int)sh->m_size/2 > (int)maxY;
    bool cond4 = ny-(int)sh->m_size/2 < (int)minY;
    while(cond1 || cond2 || cond3 || cond4)
    {
        getNewDirection(sh);
        nx = sh->m_x + m_options.m_movementSpeed*sh->m_dirX;
        ny = sh->m_y + m_options.m_movementSpeed*sh->m_dirY;
        cond1 = nx+(int)sh->m_size/2 > (int)maxX;
        cond2 = nx-(int)sh->m_size/2 < (int)minX;
        cond3 = ny+(int)sh->m_size/2 > (int)maxY;
        cond4 = ny-(int)sh->m_size/2 < (int)minY;
    }
    sh->m_x = nx;
    sh->m_y = ny;
}

void ShapesDemo::getNewDirection(Shape* sh)
{
    sh->m_dirX = ((double) rand() / (RAND_MAX))*2-1;
    sh->m_dirY = ((double) rand() / (RAND_MAX))*2-1;
    //Normalize
    float module = sqrt(pow(sh->m_dirX,2)+pow(sh->m_dirY,2));
    sh->m_dirX /= module;
    sh->m_dirY /= module;
    sh->m_changeDir = false;
}


void ShapesDemo::writeAll()
{
    for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        (*it)->write();
    }
}

void ShapesDemo::setOptions(ShapesDemoOptions& opt)
{
    m_options = opt;
    m_mainWindow->updateInterval(m_options.m_updateIntervalMs);

}

ShapesDemoOptions ShapesDemo::getOptions()
{

    return m_options;
}

void ShapesDemo::removePublisher(ShapePublisher* SP)
{
    cout << "REMOVING PUBLISHER"<<endl;
    for(std::vector<ShapePublisher*>::iterator it = this->m_publishers.begin();
        it!=this->m_publishers.end();++it)
    {
        if(SP->mp_pub->getGuid() == (*it)->mp_pub->getGuid())
        {
            m_publishers.erase(it);
            delete(SP);
            break;
        }
    }

}

void ShapesDemo::removeSubscriber(ShapeSubscriber* SS)
{
    cout << "REMOVING SUBSCRIBER"<<endl;
    for(std::vector<ShapeSubscriber*>::iterator it = this->m_subscribers.begin();
        it!=this->m_subscribers.end();++it)
    {
        if(SS->mp_sub->getGuid() == (*it)->mp_sub->getGuid())
        {
            m_subscribers.erase(it);
            delete(SS);
            break;
        }
    }
}
