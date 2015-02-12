/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimashapesdemo/qt/publishdialog.h"
#include "eprosimashapesdemo/qt/subscribedialog.h"
#include "eprosimashapesdemo/qt/optionsdialog.h"
#include "ui_mainwindow.h"
#include "eprosimashapesdemo/qt/UpdateThread.h"

#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"

#include <QStandardItemModel>
#include <QKeyEvent>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_shapesDemo(this),
    mp_writeThread(NULL),
    m_tableRow(-1)
{
    ui->setupUi(this);
    ui->areaDraw->setShapesDemo(this->getShapesDemo());

    mp_writeThread = new UpdateThread(this,1);
    mp_writeThread->setMainW(this);

    m_pubsub = new QStandardItemModel(0,9,this); //2 Rows and 3 Columns
    m_pubsub->setHorizontalHeaderItem(0, new QStandardItem(QString("Topic")));
    m_pubsub->setHorizontalHeaderItem(1, new QStandardItem(QString("Color")));
    m_pubsub->setHorizontalHeaderItem(2, new QStandardItem(QString("Size")));
    m_pubsub->setHorizontalHeaderItem(3, new QStandardItem(QString("Type")));
    m_pubsub->setHorizontalHeaderItem(4, new QStandardItem(QString("Reliable")));
    m_pubsub->setHorizontalHeaderItem(5, new QStandardItem(QString("History")));
    m_pubsub->setHorizontalHeaderItem(6, new QStandardItem(QString("Partitions")));
    m_pubsub->setHorizontalHeaderItem(7, new QStandardItem(QString("Ownership")));
    m_pubsub->setHorizontalHeaderItem(8, new QStandardItem(QString("Durability")));
    m_pubsub->setHorizontalHeaderItem(9, new QStandardItem(QString("Liveliness")));

    ui->tableEndpoint->setModel(m_pubsub);


    QHeaderView* header = ui->tableEndpoint->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    header->setMinimumSectionSize(70);

    //    ui->tableEndpoint->setColumnWidth(0,60); //Topic
    //    ui->tableEndpoint->setColumnWidth(1,65); //Color
    //    ui->tableEndpoint->setColumnWidth(2,45); //Size
    //    ui->tableEndpoint->setColumnWidth(3,45); //Type
    //    ui->tableEndpoint->setColumnWidth(4,55); //Reliable
    //    ui->tableEndpoint->setColumnWidth(5,55); //History
    //    ui->tableEndpoint->setColumnWidth(6,65); //Partitions
    //    ui->tableEndpoint->setColumnWidth(7,75); //Ownership
    //    ui->tableEndpoint->setColumnWidth(8,75); //Durability
    //    ui->tableEndpoint->setColumnWidth(9,75); //Livleiness


    ui->widget_contentFilter->setVisible(false);

    this->m_shapesDemo.init();
    if(m_shapesDemo.isInitialized())
    {
        addMessageToOutput(QString("RTPSParticipant ready in domainId %1").arg(m_shapesDemo.getOptions().m_domainId),true);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_bt_publish_clicked()
{
    PublishDialog* pd = new PublishDialog(this->getShapesDemo(),this);
    pd->show();
    mp_writeThread->start();

}

void MainWindow::quitThreads()
{
    mp_writeThread->quit();
}


void MainWindow::addMessageToOutput(QString str,bool addtostatus)
{
    if(addtostatus)
    {
        this->ui->statusBar->showMessage(str);
    }
    QDateTime current = QDateTime::currentDateTime();
    QString time = current.toString("hh:mm:ss.zzz");
    this->ui->text_Output->append(QString("[%1]: %2").arg(time).arg(str));
}


void MainWindow::on_bt_subscribe_clicked()
{
    SubscribeDialog* pd = new SubscribeDialog(this->getShapesDemo(),this);
    pd->show();
    //TODO Revisar quien llama al destructor.
}


void MainWindow::writeNewSamples()
{
    // cout << "MOVING TIMER "<<endl;
    this->m_shapesDemo.moveAllShapes();
    this->m_shapesDemo.writeAll();
}

void MainWindow::on_actionPreferences_triggered()
{
    OptionsDialog* od = new OptionsDialog(this,this->getShapesDemo(),this);
    od->show();
}

void MainWindow::updateInterval(uint32_t ms)
{
    this->mp_writeThread->updateInterval(ms);
}

void MainWindow::on_actionStart_triggered()
{
    if(this->m_shapesDemo.init())
    {
        if(m_shapesDemo.isInitialized())
            addMessageToOutput(QString("RTPSParticipant ready in domainId %1").arg(m_shapesDemo.getOptions().m_domainId),true);
    }
}

void MainWindow::on_actionStop_triggered()
{
    this->m_shapesDemo.stop();
    m_pubsub->removeRows(0,m_pubsub->rowCount());
    update();
    addMessageToOutput(QString("ShapesDemo stopped"),true);
}

void MainWindow::on_actionExit_triggered()
{
    on_actionStop_triggered();
    this->close();
}

void MainWindow::on_MainWindow_destroyed()
{
    on_actionStop_triggered();
    this->close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    on_actionStop_triggered();
    this->close();
    event->accept();
}

void MainWindow::addPublisherToTable(ShapePublisher* spub)
{
    QList<QStandardItem*> items;
    items.append(new QStandardItem(getShapeQStr(spub->m_shape.m_type)));
    items.append(new QStandardItem(QString(getColorStr(spub->m_shape.m_color).c_str())));
    items.append(new QStandardItem(QString("%1").arg(spub->m_shape.m_size)));
    items.append(new QStandardItem("Pub"));
    if(spub->m_attributes.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        items.append(new QStandardItem("True"));
    else
        items.append(new QStandardItem("False"));
    items.append(new QStandardItem(QString("%1").arg(spub->m_attributes.topic.historyQos.depth)));
    //PARTITIONS:
    QString partitions;
    std::vector<std::string> part = spub->m_attributes.qos.m_partition.getNames();
    for(std::vector<std::string>::iterator it = part.begin();
        it!=part.end();it++)
    {
        partitions.append(it->c_str());
        partitions.append(" ");
    }
    if(partitions.size()==0)
        partitions.append("-");
    items.append(new QStandardItem(partitions));
    //OWNERSHIP:
    if(spub->m_attributes.qos.m_ownership.kind == SHARED_OWNERSHIP_QOS)
        items.append(new QStandardItem("SHARED"));
    else
    {
        QString value = QString("EXCL - %1").arg(spub->m_attributes.qos.m_ownershipStrength.value);
        items.append(new QStandardItem(value));
    }

    if(spub->m_attributes.qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
        items.append(new QStandardItem("VOLATILE"));
    else
        items.append(new QStandardItem("TRANSIENT"));

    if(spub->m_attributes.qos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
        items.append(new QStandardItem("AUTOMATIC"));
    else if(spub->m_attributes.qos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
        items.append(new QStandardItem("MAN_PARTICIPANT"));
    else
        items.append(new QStandardItem("MAN_TOPIC"));


    m_pubsub->appendRow(items);
    SD_Endpoint sdend;
    sdend.type = PUB;
    sdend.pub = spub;
    sdend.pos = m_pubsub->rowCount()-1;
    this->m_pubsub_pointers.push_back(sdend);
    addMessageToOutput(QString("Publisher created in topic: %2 %1").arg(spub->m_attributes.topic.topicName.c_str()).arg(getColorStr(spub->m_shape.m_color).c_str()),false);

}

void MainWindow::addSubscriberToTable(ShapeSubscriber* ssub)
{
    if(ssub->m_shapeHistory.m_filter.m_useContentFilter)
    {
        this->ui->areaDraw->addContentFilter(ssub);
    }
    QList<QStandardItem*> items;
    items.append(new QStandardItem(getShapeQStr(ssub->m_shapeType)));
    items.append(new QStandardItem("---"));
    items.append(new QStandardItem("---"));
    items.append(new QStandardItem("Sub"));

    if(ssub->m_attributes.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        items.append(new QStandardItem("True"));
    else
        items.append(new QStandardItem("False"));

    items.append(new QStandardItem(QString("%1").arg(ssub->m_attributes.topic.historyQos.depth)));
    //PARTITIONS:
    QString partitions;
    std::vector<std::string> part = ssub->m_attributes.qos.m_partition.getNames();
    for(std::vector<std::string>::iterator it = part.begin();
        it!=part.end();it++)
    {
        partitions.append(it->c_str());
        partitions.append(" ");
    }
    if(partitions.size()==0)
        partitions.append("-");
    items.append(new QStandardItem(partitions));
    //OWNERSHIP:
    if(ssub->m_attributes.qos.m_ownership.kind == SHARED_OWNERSHIP_QOS)
        items.append(new QStandardItem("SHARED"));
    else
        items.append(new QStandardItem("EXCLUSIVE"));


    if(ssub->m_attributes.qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
        items.append(new QStandardItem("VOLATILE"));
    else
        items.append(new QStandardItem("TRANSIENT"));

    if(ssub->m_attributes.qos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
        items.append(new QStandardItem("AUTOMATIC"));
    else if(ssub->m_attributes.qos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
        items.append(new QStandardItem("MAN_PARTICIPANT"));
    else
        items.append(new QStandardItem("MAN_TOPIC"));

    m_pubsub->appendRow(items);
    SD_Endpoint sdend;
    sdend.type = SUB;
    sdend.sub = ssub;
    sdend.pos = m_pubsub->rowCount()-1;
    this->m_pubsub_pointers.push_back(sdend);
    addMessageToOutput(QString("Subscriber created in topic: %1").arg(ssub->m_attributes.topic.getTopicName().c_str()),false);
}

void MainWindow::on_tableEndpoint_customContextMenuRequested(const QPoint &pos)
{
    // cout <<"CONTEXT MENU REQUESTED"<<endl;
    QModelIndex index=this->ui->tableEndpoint->indexAt(pos);
    this->ui->tableEndpoint->selectRow(index.row());
    // cout << index.column()<< " "<< index.row()<<endl;
    this->m_tableRow = index.row();
    if(index.row()>=0)
    {
        QMenu *menu = new QMenu(this);
        menu->addAction(this->ui->actionDelete_Enpoint);
        menu->popup(this->ui->tableEndpoint->viewport()->mapToGlobal(pos));
    }
}

void MainWindow::on_actionDelete_Enpoint_triggered()
{
    cout << "DELETE ENDPOINT" <<endl;
    removeRow(m_tableRow);
}


void MainWindow::removeRow(int row)
{
    //  cout << "REMOVING ROW **************************"<<endl;
    m_pubsub->removeRow(row);
    for(std::vector<SD_Endpoint>::iterator it = this->m_pubsub_pointers.begin();
        it!=this->m_pubsub_pointers.end();++it)
    {
        if(row == it->pos)
        {
            if(it->type == PUB)
            {
                // cout << "REMOVING PUBLISHER "<<endl;
                this->m_shapesDemo.removePublisher(it->pub);
                addMessageToOutput(QString("Removed Publisher"),false);
            }
            else
            {
                //   cout << "REMOVING SUBSCRIBER "<<endl;
                this->m_shapesDemo.removeSubscriber(it->sub);
                addMessageToOutput(QString("Removed Subscriber"),false);
            }

            for(std::vector<SD_Endpoint>::iterator it2 = it;it2!=this->m_pubsub_pointers.end();++it2)
            {
                it2->pos--;
                //  cout << "POSITION -1"<<endl;
            }
            m_pubsub_pointers.erase(it);
            break;
        }
    }
    cout << "FINISH REMOVE ROW"<<endl;
}

void MainWindow::on_tableEndpoint_clicked(const QModelIndex &index)
{
    this->ui->tableEndpoint->selectRow(index.row());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        QItemSelectionModel *select = this->ui->tableEndpoint->selectionModel();
        if(select->hasSelection())
        {
            QModelIndexList list = select->selectedRows();
            for(QModelIndexList::iterator it = list.begin();it!=list.end();++it)
            {
                removeRow(it->row());
            }

        }
    }
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(QString("<p><strong>eProsima Shapes Demo 1.0.0</strong></p><p>Copyright (C) eProsima 2015</p> <p><a href=http://www.eProsima.com>http:://www.eProsima.com</a></p>"));
    msgBox.exec();

}

void MainWindow::on_actionUser_Manual_triggered()
{
    QDesktopServices::openUrl(QUrl("file:///C:/Program Files/eProsima/FastRTPS/doc/pdf/FastRTPS_ShapesDemo_User_Manual.pdf"));
    QString str(QDir::currentPath());
     str.append("/FASTRTPSGEN_User_Manual.pdf");
    QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}

void MainWindow::on_actionInteroperability_Troubleshooting_triggered()
{
    QDesktopServices::openUrl(QUrl("file:///C:/Program Files/eProsima/FastRTPS/doc/pdf/FastRTPS_ShapesDemo_Interoperability_Troubleshooting.pdf"));
    QString str(QDir::currentPath());
     str.append("/FASTRTPSGEN_User_Manual.pdf");
    QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}
