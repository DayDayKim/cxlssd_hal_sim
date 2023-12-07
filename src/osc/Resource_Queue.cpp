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
            Simulator->Register_sim_event(Simulator->Time()+100, this, 0, 0);
        }
    }
}
