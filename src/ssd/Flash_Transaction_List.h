#ifndef FLASH_TRANSACTION_LIST_H
#define FLASH_TRANSACTION_LIST_H

#include <string>
#include "Queue_Probe.h"
#include "NVM_Transaction_Flash.h"
#include "../sim/Sim_Reporter.h"


namespace SSD_Components
{

class Flash_Transaction_List : public std::list<NVM_Transaction_Flash*>
{
public:
    Flash_Transaction_List();
    Flash_Transaction_List(std::string id);
    void Set_id(std::string id);
    void insertNode(stream_id_type , NVM_Transaction_Flash* const&);
    NVM_Transaction_Flash* findNode(stream_id_type nHostId);
    NVM_Transaction_Flash* findNode();
    void removeNode(NVM_Transaction_Flash*);
    void set_max_list_size(unsigned int list_size) { max_list_size = list_size; };
    unsigned int get_max_list_size() { return max_list_size; };
    void set_host_count(unsigned int count) { host_count = count; };
private:
    std::string id;
    Queue_Probe RequestQueueProbe;
    stream_id_type nLastHostId { 0 };
    unsigned int host_count { 1 };
    unsigned int max_list_size { 1024 };
};

}

#endif // FLASH_TRANSACTION_LIST_H
