#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimashapesdemo/utils/md5.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"

short kind = 0;
short colour = 0;
int n_args = 0;

void setShapeAttributes(ShapePublisher* SP, ShapesDemo *mp_sd);

void printHelp();

void parseArguments(char *argv[]) 
{
	int index = 1;

	if (index < n_args && strcmp(argv[index++], "-t") == 0) {
		if (strcmp(argv[index], "square") == 0) {
			printf("square selected\n");
			kind = 0;
		} else if (strcmp(argv[index], "triangle") == 0) {
			printf("triangle selected\n");
			kind = 1;
		} else if (strcmp(argv[index], "circle") == 0) {
			printf("circle selected\n");
			kind = 2;
		} else {
			printf("Shape not recognized, creating square by default...\n");
			kind = 0;
		}
		index++;
	} 
	
	if (index < n_args && strcmp(argv[index++], "-c") == 0) {
		if (strcmp(argv[index], "purple") == 0) {
			colour = 0;
		} else if (strcmp(argv[index], "blue") == 0) {
			colour = 1;
		} else if (strcmp(argv[index], "red") == 0) {
			colour = 2;
		} else if (strcmp(argv[index], "green") == 0) {
			colour = 3;
		} else if (strcmp(argv[index], "yellow") == 0) {
			colour = 4;
		} else if (strcmp(argv[index], "cyan") == 0) {
			colour = 5;
		} else if (strcmp(argv[index], "magenta") == 0) {
			colour = 6;
		} else if (strcmp(argv[index], "orange") == 0) {
			colour = 7;
		} else if (strcmp(argv[index], "grey") == 0) {
			colour = 8;
		} else if (strcmp(argv[index], "black") == 0) {
			colour = 9;
		}  else {
			printf("Colour not recognized, setting colour to purple by default...\n");
			colour = 0;
		}
		index++;
	}
}

void publish(ShapesDemo *mp_sd, int argc, char *argv[])
{

	parseArguments(argv);
	

	ShapePublisher* SP = new ShapePublisher(mp_sd->getParticipant());
	//Get the different elements
	//ShapeAttributes
	setShapeAttributes(SP, mp_sd);

    //SHAPE/TOPIC:
    
    
    
	switch(kind) {
		case 0:
			SP->m_shape.m_type = SQUARE;
			SP->m_attributes.topic.topicName = "Square";
			break;
		case 1:
			SP->m_shape.m_type = TRIANGLE;
			SP->m_attributes.topic.topicName = "Triangle";
			break;
		case 2:
			SP->m_shape.m_type = CIRCLE;
			SP->m_attributes.topic.topicName = "Circle";
			break;
		default:
			SP->m_shape.m_type = SQUARE;
			SP->m_attributes.topic.topicName = "Square";
			break;
	}
	
	
    
	SP->m_attributes.topic.topicDataType = "ShapeType";
	SP->m_attributes.topic.topicKind = WITH_KEY;

    //History:
    SP->m_attributes.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SP->m_attributes.topic.historyQos.depth = 10;

    //Reliability
   // if(this->ui->checkBox_reliable->isChecked())
        SP->m_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    //LIVELINESS:
   // cout << "LIVELINESS "<<this->ui->comboBox_liveliness->currentIndex()<<endl;
   //if(this->ui->comboBox_liveliness->currentIndex() == 0)
       SP->m_attributes.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
   /*if(this->ui->comboBox_liveliness->currentIndex() == 1)
       SP->m_attributes.qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
   if(this->ui->comboBox_liveliness->currentIndex() == 2)
       SP->m_attributes.qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
   if(this->ui->lineEdit_leaseDuration->text()=="INF")
       SP->m_attributes.qos.m_liveliness.lease_duration = c_TimeInfinite;
   else
   {
        QString value = this->ui->lineEdit_leaseDuration->text();
        if(value.toDouble()>0)
        {
            SP->m_attributes.qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t(value.toDouble());
            SP->m_attributes.qos.m_liveliness.announcement_period = TimeConv::MilliSeconds2Time_t(value.toDouble()/2);
        }
   }*/
   
   //DURABILITY
   
   //cout << "Durability INDEX: "<< this->ui->comboBox_durability->currentIndex() << endl;
   
   //switch(this->ui->comboBox_durability->currentIndex())
   //{
   //case 0: SP->m_attributes.qos.m_durability.kind = VOLATILE_DURABILITY_QOS; break;
   //case 1: SP->m_attributes.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
   SP->m_attributes.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS; //break;
   //}
   
   //Ownership:
   
   //switch(this->ui->comboBox_ownership->currentIndex())
   //{
   //case 0: SP->m_attributes.qos.m_ownership.kind = SHARED_OWNERSHIP_QOS; break;
	SP->m_attributes.qos.m_ownership.kind = SHARED_OWNERSHIP_QOS; //break;
   //case 1: SP->m_attributes.qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS; break;
   //}

   //if(SP->m_attributes.qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
       //SP->m_attributes.qos.m_ownershipStrength.value = this->ui->spin_ownershipStrength->value();
       //SP->m_attributes.qos.m_ownershipStrength.value = 2;
   
    //PARTITIONS:
    
   //if(this->ui->checkBox_Asterisk->isChecked())
       //SP->m_attributes.qos.m_partition.names.push_back("*");
    //if(this->ui->checkBox_A->isChecked())
        //SP->m_attributes.qos.m_partition.names.push_back("A");
    /*if(this->ui->checkBox_B->isChecked())
        SP->m_attributes.qos.m_partition.names.push_back("B");
    if(this->ui->checkBox_C->isChecked())
        SP->m_attributes.qos.m_partition.names.push_back("C");
    if(this->ui->checkBox_D->isChecked())
        SP->m_attributes.qos.m_partition.names.push_back("D");
*/


    if(SP->initPublisher())
     mp_sd->addPublisher(SP);

	for (;;) {
		//SP->write();
		usleep(200000);
		mp_sd->moveAllShapes();
		mp_sd->writeAll();	
	}

}

void setShapeAttributes(ShapePublisher* SP, ShapesDemo *mp_sd)
{
    //COLOR:
    
    switch(colour) {
    	case 0:
    		SP->m_shape.define(SD_PURPLE);
    		break;
    	case 1:
    		SP->m_shape.define(SD_BLUE);
    		break;
    	case 2:
    		SP->m_shape.define(SD_RED);
    		break;
    	case 3:
    		SP->m_shape.define(SD_GREEN);
    		break;
    	case 4:
    		 SP->m_shape.define(SD_YELLOW);
    		break;
    	case 5:
    		SP->m_shape.define(SD_CYAN);
    		break;
    	case 6:
    		 SP->m_shape.define(SD_MAGENTA);
    		break;
    	case 7:
		SP->m_shape.define(SD_ORANGE);
    		break;
    	case 8:
    		SP->m_shape.define(SD_GRAY);
    		break;
    	case 9:
		SP->m_shape.define(SD_BLACK);
		break;
	default:
		SP->m_shape.define(SD_PURPLE);
		break;
    }
    
    //SIZE:
    SP->m_shape.m_size = 30;
    //POSITION IS RANDOM:
    SP->m_shape.m_x = mp_sd->getRandomX(SP->m_shape.m_size);
    SP->m_shape.m_y = mp_sd->getRandomY(SP->m_shape.m_size);
    //SP->m_shape.m_x = 100;
    //SP->m_shape.m_y = 100;

}

void printHelp() 
{
	printf("Usage: ShapesDemo [-t <entity_type>] [-c <colour>] \n");
	printf("Where:\n");
	printf("\t<entity_type> can be 'square', 'triangle' or 'circle'.\n");
	printf("\t<colour> can be 'square', 'triangle' or 'circle'.\n");
}

int main(int argc, char *argv[])
{
	std::cout << "argc:" << argc << std::endl;
	n_args = argc;
	
	if (argc == 1 || (argc-1) % 2) {
		printf("Incorrect number of parameters. \n");
		printHelp();
		return -1;
	}

	RTPSLog::setVerbosity(EPROSIMA_INFO_VERB_LEVEL);

	ShapesDemo m_shapesDemo;
	m_shapesDemo.init();
	if(m_shapesDemo.isInitialized())
	{
		//addMessageToOutput(QString("Participant ready in domainId %1").arg(m_shapesDemo.getOptions().m_domainId),true);
		printf("Participant ready in domainId %d\n", m_shapesDemo.getOptions().m_domainId);
	}
	
	publish(&m_shapesDemo, argc, argv);
		
	
    
    
    
}
