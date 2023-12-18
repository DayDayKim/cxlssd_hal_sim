#include "Flash_Transaction_List.h"

namespace SSD_Components
{
Flash_Transaction_List::Flash_Transaction_List()
{
}

Flash_Transaction_List::Flash_Transaction_List(std::string id) : id(id)
{
}

void Flash_Transaction_List::Set_id(std::string id)
{
    this->id = id;
}

void Flash_Transaction_List::insertNode(stream_id_type nHostId, NVM_Transaction_Flash* const &pstPtr)
{
    auto it = list<NVM_Transaction_Flash*>::begin();

    while (it != list<NVM_Transaction_Flash*>::end())
    {
        if ((*it)->Stream_id > nHostId)
        {
            list<NVM_Transaction_Flash*>::insert(it, pstPtr);
            return;
        }
        else if ((*it)->Stream_id == nHostId)
        {
            auto nextIt = std::next(it);
            while (nextIt != list<NVM_Transaction_Flash*>::end() && (*nextIt)->Stream_id == nHostId)
            {
                it = nextIt;
                nextIt = std::next(it);
            }
            list<NVM_Transaction_Flash*>::insert(nextIt, pstPtr);
            return;
        }
        ++it;
    }

    list<NVM_Transaction_Flash*>::push_back(pstPtr);
}

NVM_Transaction_Flash* Flash_Transaction_List::findNode(stream_id_type nHostId)
{
    for (auto it = list<NVM_Transaction_Flash*>::begin(); it != list<NVM_Transaction_Flash*>::end(); ++it)
    {
        if ((*it)->Stream_id == nHostId)
        {
            return *it;
        }
    }
    return nullptr;
}

NVM_Transaction_Flash* Flash_Transaction_List::findNode(void)
{
    NVM_Transaction_Flash* pstPtr;
    for (stream_id_type nHostId = 0; nHostId < host_count; nHostId++)
    {
        pstPtr = findNode((nHostId + nLastHostId) % host_count);
        if (nullptr != pstPtr)
        {
            nLastHostId = (nHostId + nLastHostId + 1) % host_count;
            break;
        }
    }
    return pstPtr;
}

void Flash_Transaction_List::removeNode(NVM_Transaction_Flash* node)
{
    for (auto it = list<NVM_Transaction_Flash*>::begin(); it != list<NVM_Transaction_Flash*>::end(); ++it)
    {
        if (*it == node)
        {
            list<NVM_Transaction_Flash*>::erase(it);
            break;
        }
    }
}

}
