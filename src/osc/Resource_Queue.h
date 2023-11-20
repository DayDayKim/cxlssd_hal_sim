#ifndef RESOURCE_QUEUE_H
#define RESOURCE_QUEUE_H

#include <queue>
#include "../sim/Sim_Defs.h"
#include "../sim/Sim_Object.h"
#include "../sim/Sim_Reporter.h"
#include "../ssd/SSD_Defs.h"
#include "../ssd/User_Request.h"
#include "../ssd/Host_Interface_Base.h"

namespace SSD_Components
{
class User_Request;
class Resource_Queue : public MQSimEngine::Sim_Object
{
public:
    Resource_Queue(const sim_object_id_type& id, int queue_max_size);
    ~Resource_Queue();
    void Setup_triggers() {};
    void Start_simulation() {};
    void Validate_simulation_config() {};
    void Execute_simulator_event(MQSimEngine::Sim_Event*);
    bool Is_available()
    {
        return (tag_resource_queue.size() < queue_max_size);
    }
    void InsertQueue(User_Request* request)
    {
        tag_resource_queue.push(request);
    }

    User_Request* PopQueue()
    {
        User_Request* ret = tag_resource_queue.front();
        tag_resource_queue.pop();
        return ret;
    }

    SSD_Components::Host_Interface_Base* host_interface;
protected:
    unsigned int queue_max_size {4096};
    std::queue<User_Request*> tag_resource_queue;

};

}

#endif // RESOURCE_QUEUE_H
