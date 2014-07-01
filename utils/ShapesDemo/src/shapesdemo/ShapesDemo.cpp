/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/


#include <iostream>
#include <sstream>

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include "eprosimashapesdemo/qt/mainwindow.h"

ShapesDemo::ShapesDemo(MainWindow *mw):
    mp_participant(NULL),
    m_isInitialized(false),
    minX(0),minY(0),maxX(0),maxY(0),
    m_mutex(QMutex::Recursive),
    m_mainWindow(mw)
{
    srand (time(NULL));
    minX = 0;
    minY = 0;
    maxX = 500;
    maxY = 500;
}

ShapesDemo::~ShapesDemo()
{
    stop();
}

Participant* ShapesDemo::getParticipant()
{
    if(m_isInitialized && mp_participant !=NULL)
        return mp_participant;
    else
    {
        if(init())
            return mp_participant;
    }
    return NULL;
}

bool ShapesDemo::init()
{
    cout << "Initializing ShapesDemo "<< m_isInitialized <<endl;
    QMutexLocker locker(&m_mutex);
    if(!m_isInitialized)
    {
        cout <<"Creating new participant"<<endl;
        ParticipantAttributes pparam;
        pparam.name = "eProsimaParticipant";
        pparam.discovery.domainId = m_options.m_domainId;
        pparam.discovery.leaseDuration.seconds = 100;
        pparam.discovery.resendDiscoveryParticipantDataPeriod.seconds = 50;
        pparam.defaultSendPort = 10042;
        mp_participant = DomainParticipant::createParticipant(pparam);
        if(mp_participant!=NULL)
        {
           // cout << "Participant Created "<< mp_participant->getGuid() << endl;
            m_isInitialized = true;
            DomainParticipant::registerType(&m_shapeTopicDataType);
            return true;
        }
        return false;
    }
    return true;
}

void ShapesDemo::stop()
{
    QMutexLocker locker(&m_mutex);
    if(m_isInitialized)
    {
        DomainParticipant::removeParticipant(mp_participant);
        cout << "All Stoped, removing"<<endl;
        mp_participant = NULL;
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
        m_isInitialized = false;
    }
}

void ShapesDemo::addPublisher(ShapePublisher* SP)
{
    QMutexLocker locker(&m_mutex);
    m_publishers.push_back(SP);
}

void ShapesDemo::addSubscriber(ShapeSubscriber* SSub)
{
    QMutexLocker locker(&m_mutex);
    m_subscribers.push_back(SSub);
}

bool ShapesDemo::getShapes(std::vector<Shape*> *shvec)
{
    QMutexLocker locker(&m_mutex);
    for(std::vector<ShapePublisher*>::iterator it =m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        shvec->push_back(&(*it)->m_shape);
    }
    for(std::vector<ShapeSubscriber*>::iterator it = m_subscribers.begin();
        it!=m_subscribers.end();++it)
    {
        if((*it)->hasReceived)
            shvec->push_back(&(*it)->m_shape);
    }
    return true;
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
    QMutexLocker locker(&m_mutex);
    for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        moveShape(&(*it)->m_shape);
    }
}

void ShapesDemo::moveShape(Shape* sh)
{
    if(sh->m_changeDir)
    {
        getNewDirection(sh);
    }
    //Apply movement
    int nx = sh->m_mainShape.m_x + m_options.m_movementSpeed*sh->m_dirX;
    int ny = sh->m_mainShape.m_y + m_options.m_movementSpeed*sh->m_dirY;
    //Check if the movement is correct
    bool cond1 = nx+(int)sh->m_mainShape.m_size > (int)maxX;
    bool cond2 = nx-(int)sh->m_mainShape.m_size < (int)minX;
    bool cond3 = ny+(int)sh->m_mainShape.m_size > (int)maxY;
    bool cond4 = ny-(int)sh->m_mainShape.m_size < (int)minY;
    if( cond1 || cond2 || cond3 || cond4)
    {
        sh->m_changeDir = true;
    }
    else
    {
        sh->m_mainShape.m_x = nx;
        sh->m_mainShape.m_y = ny;
    }

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
     QMutexLocker locker(&m_mutex);
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
