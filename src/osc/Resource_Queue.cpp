#include "Resource_Queue.h"

namespace SSD_Components
{
    Resource_Queue::Resource_Queue(const sim_object_id_type& id, int queue_max_size):MQSimEngine::Sim_Object(id)
    {
        this->queue_max_size = queue_max_size;
    }

    Resource_Queue::~Resource_Queue()
    {
        this->queue_max_size = 0;
    }

    void Resource_Queue::Execute_simulator_event(MQSimEngine::Sim_Event *ev)
    {
        User_Request* request = tag_resource_queue.front();
        bool* pFlag = new bool;
        *pFlag = true;
        this->host_interface->broadcast_user_request_arrival_signal(request, pFlag);
        if (*pFlag == true)
        {
            tag_resource_queue.pop();
        }
        else
        {
            MQSimEngine::EventTreeNode* second_minNode = Simulator->_EventList->Get_second_min_node();
            MQSimEngine::Sim_Event* ev = second_minNode->FirstSimEvent;
            if (Simulator->Time() == ev->Fire_time)
            {
                Simulator->Register_sim_event(ev->Fire_time + 1000, this, 0, 0);
            }
            else
            {
                Simulator->Register_sim_event(ev->Fire_time, this, 0, 0);
            }
        }
    }
}
