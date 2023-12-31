#include "TSU_OutofOrder.h"
#include <iostream>
#include <fstream>

//ofstream of1{ "Read_suspended_cause_write.txt" };
//ofstream of2{ "Flash_read_channel.txt" };
//ofstream of3{ "Flash_write_channel.txt" };

uint64_t READ_COUNT{ 0 };
uint64_t READ_SUS_COUNT{ 0 };
bool SUS_CAUSE_WR{ 0 };


namespace SSD_Components
{

TSU_OutOfOrder::TSU_OutOfOrder(const sim_object_id_type& id, FTL* ftl, NVM_PHY_ONFI_NVDDR2* NVMController, unsigned int ChannelCount, unsigned int chip_no_per_channel,
                               unsigned int DieNoPerChip, unsigned int PlaneNoPerDie, unsigned int HostNumber,
                               sim_time_type WriteReasonableSuspensionTimeForRead,
                               sim_time_type EraseReasonableSuspensionTimeForRead,
                               sim_time_type EraseReasonableSuspensionTimeForWrite,
                               bool EraseSuspensionEnabled, bool ProgramSuspensionEnabled)
    : TSU_Base(id, ftl, NVMController, Flash_Scheduling_Type::OUT_OF_ORDER, ChannelCount, chip_no_per_channel, DieNoPerChip, PlaneNoPerDie, HostNumber,
               WriteReasonableSuspensionTimeForRead, EraseReasonableSuspensionTimeForRead, EraseReasonableSuspensionTimeForWrite,
               EraseSuspensionEnabled, ProgramSuspensionEnabled)
{
    UserReadTRQueue = new Flash_Transaction_Queue*[channel_count];
    UserWriteTRQueue = new Flash_Transaction_Queue*[channel_count];
    GCReadTRQueue = new Flash_Transaction_Queue*[channel_count];
    GCWriteTRQueue = new Flash_Transaction_Queue*[channel_count];
    GCEraseTRQueue = new Flash_Transaction_Queue*[channel_count];
    MappingReadTRQueue = new Flash_Transaction_Queue*[channel_count];
    MappingWriteTRQueue = new Flash_Transaction_Queue*[channel_count];
    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        UserReadTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        UserWriteTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        GCReadTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        GCWriteTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        GCEraseTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        MappingReadTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        MappingWriteTRQueue[channelID] = new Flash_Transaction_Queue[chip_no_per_channel];
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            UserReadTRQueue[channelID][chip_cntr].Set_id("User_Read_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            UserWriteTRQueue[channelID][chip_cntr].Set_id("User_Write_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            GCReadTRQueue[channelID][chip_cntr].Set_id("GC_Read_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            MappingReadTRQueue[channelID][chip_cntr].Set_id("Mapping_Read_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            MappingWriteTRQueue[channelID][chip_cntr].Set_id("Mapping_Write_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            GCWriteTRQueue[channelID][chip_cntr].Set_id("GC_Write_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
            GCEraseTRQueue[channelID][chip_cntr].Set_id("GC_Erase_TR_Queue@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr));
        }
    }
}

TSU_OutOfOrder::~TSU_OutOfOrder()
{
    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        delete[] UserReadTRQueue[channelID];
        delete[] UserWriteTRQueue[channelID];
        delete[] GCReadTRQueue[channelID];
        delete[] GCWriteTRQueue[channelID];
        delete[] GCEraseTRQueue[channelID];
        delete[] MappingReadTRQueue[channelID];
        delete[] MappingWriteTRQueue[channelID];
    }
    delete[] UserReadTRQueue;
    delete[] UserWriteTRQueue;
    delete[] GCReadTRQueue;
    delete[] GCWriteTRQueue;
    delete[] GCEraseTRQueue;
    delete[] MappingReadTRQueue;
    delete[] MappingWriteTRQueue;

    if (true == bHolbAvoidEnabled)
    {
        for (unsigned int channelID = 0; channelID < channel_count; channelID++)
        {
            for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
            {
                delete[] UserReadTRListHAL[channelID][chip_cntr];
            }
            delete[] UserReadTRListHAL[channelID];
        }
        delete[] UserReadTRListHAL;
    }
}

void TSU_OutOfOrder::Start_simulation()
{
}

void TSU_OutOfOrder::Validate_simulation_config()
{
}

void TSU_OutOfOrder::Execute_simulator_event(MQSimEngine::Sim_Event* event)
{
}

void TSU_OutOfOrder::Report_results_in_XML(std::string name_prefix, Utils::XmlWriter& xmlwriter)
{
    name_prefix = name_prefix + +".TSU";
    xmlwriter.Write_open_tag(name_prefix);

    TSU_Base::Report_results_in_XML(name_prefix, xmlwriter);

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            UserReadTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".User_Read_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            UserWriteTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".User_Write_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            MappingReadTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".Mapping_Read_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            MappingWriteTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".Mapping_Write_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            GCReadTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".GC_Read_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            GCWriteTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".GC_Write_TR_Queue", xmlwriter);
        }
    }

    for (unsigned int channelID = 0; channelID < channel_count; channelID++)
    {
        for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
        {
            GCEraseTRQueue[channelID][chip_cntr].Report_results_in_XML(name_prefix + ".GC_Erase_TR_Queue", xmlwriter);
        }
    }

    xmlwriter.Write_close_tag();
}

inline void TSU_OutOfOrder::Prepare_for_transaction_submit()
{
    opened_scheduling_reqs++;
    if (opened_scheduling_reqs > 1)
    {
        return;
    }
    transaction_receive_slots.clear();
}

inline void TSU_OutOfOrder::Submit_transaction(NVM_Transaction_Flash* transaction)
{
    transaction_receive_slots.push_back(transaction);
}

bool TSU_OutOfOrder::Schedule()
{
    bool trigger = false;
    bool retval = true;

    opened_scheduling_reqs--;
    if (opened_scheduling_reqs > 0)
    {
        return true;
    }

    if (opened_scheduling_reqs < 0)
    {
        PRINT_ERROR("TSU_OutOfOrder: Illegal status!");
    }

    if (transaction_receive_slots.size() == 0)
    {
        return true;
    }

    uint64_t userioch{ 0 }, useriochip{ 0 };
    bool isUserRead{ 0 };
    uint32_t nChannelID;
    uint32_t nChipID;
    uint32_t nHostID;
    uint32_t nPlaneID;

    for (std::list<NVM_Transaction_Flash*>::iterator it = transaction_receive_slots.begin();
            it != transaction_receive_slots.end(); it++)
    {
        switch ((*it)->Type)
        {
            case Transaction_Type::READ:
                switch ((*it)->Source)
                {
                    case Transaction_Source_Type::CACHE:
                    case Transaction_Source_Type::USERIO:
                        if ((it == transaction_receive_slots.begin()))
                        {
                            //of2 << (*it)->LPA << " " << (*it)->Issue_time <<" "<< (*it)->Address.ChannelID<< (*it)->Address.ChipID << endl;
                            READ_COUNT++;
                            userioch = (*it)->Address.ChannelID;
                            useriochip = (*it)->Address.ChipID;
                            isUserRead = 1;
                        }
                        nChannelID = (*it)->Address.ChannelID;
                        nChipID = (*it)->Address.ChipID;
                        nPlaneID = (*it)->Address.PlaneID;
                        nHostID = (*it)->Stream_id;

                        if (false == bHolbAvoidEnabled)
                        {
                            if (UserReadTRQueue[nChannelID][nChipID].size() < UserReadTRQueue[nChannelID][nChipID].get_max_queue_depth())
                            {
                                UserReadTRQueue[nChannelID][nChipID].push_back((*it));
                            }
                            else
                            {
                                //std::cout << "HOLB issued" << std::endl;
                                return false;
                            }
                        }
                        else
                        {
                            if (UserReadTRListHAL[nChannelID][nChipID][nPlaneID].size() < UserReadTRListHAL[nChannelID][nChipID][nPlaneID].get_max_list_size())
                            {
                                UserReadTRListHAL[nChannelID][nChipID][nPlaneID].insertNode(nHostID,(*it));
                                debugcount[nChannelID][nChipID][nPlaneID]++;
                            }
                            else
                            {
                                return false;
                            }

                        }
                        break;
                    case Transaction_Source_Type::MAPPING:
                        MappingReadTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                        break;
                    case Transaction_Source_Type::GC_WL:
                        GCReadTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                        break;
                    default:
                        PRINT_ERROR("TSU_OutOfOrder: unknown source type for a read transaction!")
                }
                break;
            case Transaction_Type::WRITE:
                switch ((*it)->Source)
                {
                    case Transaction_Source_Type::CACHE:
                    case Transaction_Source_Type::USERIO:
                        if ((it == transaction_receive_slots.begin()))
                        {
                            //of3 << (*it)->LPA << " " << (*it)->Issue_time << " " << (*it)->Address.ChannelID << (*it)->Address.ChipID << endl;
                        }
                        UserWriteTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                        trigger = true;
                        break;
                    case Transaction_Source_Type::MAPPING:
                        MappingWriteTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                        break;
                    case Transaction_Source_Type::GC_WL:
                        GCWriteTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                        break;
                    default:
                        PRINT_ERROR("TSU_OutOfOrder: unknown source type for a write transaction!")
                }
                break;
            case Transaction_Type::ERASE:
                GCEraseTRQueue[(*it)->Address.ChannelID][(*it)->Address.ChipID].push_back((*it));
                break;
            default:
                break;
        }
    }

    for (flash_channel_ID_type channelID = 0; channelID < channel_count; channelID++)
    {
        if (_NVMController->Get_channel_status(channelID) == BusChannelStatus::IDLE)
        {
            for (unsigned int i = 0; i < chip_no_per_channel; i++)
            {
                NVM::FlashMemory::Flash_Chip* chip = _NVMController->Get_chip(channelID, Round_robin_turn_of_channel[channelID]);
                //The TSU does not check if the chip is idle or not since it is possible to suspend a busy chip and issue a new command
                if (!service_read_transaction(chip))
                {
                    if (SUS_CAUSE_WR && isUserRead && userioch == channelID && useriochip == chip->ChipID)
                    {
                        READ_SUS_COUNT++;
                        SUS_CAUSE_WR = 0;
                    }
                    if (!service_write_transaction(chip))
                    {
                        service_erase_transaction(chip);
                    }
                }
                Round_robin_turn_of_channel[channelID] = (flash_chip_ID_type)(Round_robin_turn_of_channel[channelID] + 1) % chip_no_per_channel;
                if (_NVMController->Get_channel_status(chip->ChannelID) != BusChannelStatus::IDLE)
                {
                    break;
                }
            }
        }
    }

    //of1 << READ_COUNT << " " << READ_SUS_COUNT << endl;
    return retval;
}

void TSU_OutOfOrder::Set_Holb_Avoid_Enable(bool bEnable, bool bPIREnable)
{
    if (true == bEnable)
    {
        debugcount  = new uint32_t**[channel_count];
        ppnLastPlane = new uint32_t*[channel_count];
        UserReadTRListHAL = new Flash_Transaction_List**[channel_count];
        for (unsigned int channelID = 0; channelID < channel_count; channelID++)
        {
            debugcount[channelID] = new uint32_t*[chip_no_per_channel];
            ppnLastPlane[channelID] = new uint32_t[chip_no_per_channel];
            UserReadTRListHAL[channelID] = new Flash_Transaction_List*[chip_no_per_channel];
            for (unsigned int chip_cntr = 0; chip_cntr < chip_no_per_channel; chip_cntr++)
            {
                debugcount[channelID][chip_cntr] = new uint32_t[plane_no_per_die];
                ppnLastPlane[channelID][chip_cntr] = 0;
                UserReadTRListHAL[channelID][chip_cntr] = new Flash_Transaction_List[plane_no_per_die];
                for (unsigned int plane_cntr = 0; plane_cntr < plane_no_per_die; plane_cntr++)
                {
                    debugcount[channelID][chip_cntr][plane_cntr] = 0;
                    UserReadTRListHAL[channelID][chip_cntr][plane_cntr].Set_id("User_Read_TR_LIST_FOR_HAL@" + std::to_string(channelID) + "@" + std::to_string(chip_cntr) + "@" + std::to_string(plane_cntr));
                    UserReadTRListHAL[channelID][chip_cntr][plane_cntr].set_host_count(host_count);
                }
            }
        }
    }
    bHolbAvoidEnabled = bEnable;
    bPIREnabled = bPIREnable;
}

bool TSU_OutOfOrder::service_read_transaction(NVM::FlashMemory::Flash_Chip* chip)
{
    Flash_Transaction_Queue *sourceQueue1 = NULL, *sourceQueue2 = NULL;

    //Flash transactions that are related to FTL mapping data have the highest priority
    if (MappingReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
    {
        sourceQueue1 = &MappingReadTRQueue[chip->ChannelID][chip->ChipID];
        if (ftl->GC_and_WL_Unit->GC_is_in_urgent_mode(chip) && GCReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue2 = &GCReadTRQueue[chip->ChannelID][chip->ChipID];
        }
        else if (UserReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue2 = &UserReadTRQueue[chip->ChannelID][chip->ChipID];
        }
    }
    else if (ftl->GC_and_WL_Unit->GC_is_in_urgent_mode(chip))
    {
        //If flash transactions related to GC are prioritzed (non-preemptive execution mode of GC), then GC queues are checked first

        if (GCReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &GCReadTRQueue[chip->ChannelID][chip->ChipID];
            if (UserReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
            {
                sourceQueue2 = &UserReadTRQueue[chip->ChannelID][chip->ChipID];
            }
        }
        else if (GCWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            return false;
        }
        else if (GCEraseTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            return false;
        }
        else if (UserReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &UserReadTRQueue[chip->ChannelID][chip->ChipID];
        }
        else
        {
            return false;
        }
    }
    else
    {
        //If GC is currently executed in the preemptive mode, then user IO transaction queues are checked first
        //int debug_GCW = GCWriteTRQueue[chip->ChannelID][chip->ChipID].size();
        ////std::cout << "[service_read_transaction()] UserRQ: " << UserReadTRQueue[chip->ChannelID][chip->ChipID].size() <<", UserWQ: " << UserWriteTRQueue[chip->ChannelID][chip->ChipID].size() << ", GCRQ: " << GCReadTRQueue[chip->ChannelID][chip->ChipID].size()<<", GCWQ: "<< GCWriteTRQueue[chip->ChannelID][chip->ChipID].size() << std::endl;
        if (UserReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &UserReadTRQueue[chip->ChannelID][chip->ChipID];
            if (GCReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
            {
                sourceQueue2 = &GCReadTRQueue[chip->ChannelID][chip->ChipID];
            }
        }
        else if (UserWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            return false;
        }
        else if (GCReadTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &GCReadTRQueue[chip->ChannelID][chip->ChipID];
        }
        else
        {
            if (true == bHolbAvoidEnabled)
            {
                unsigned int plane_no;
                for (plane_no = 0; plane_no < plane_no_per_die; plane_no++)
                {
                    if (UserReadTRListHAL[chip->ChannelID][chip->ChipID][plane_no].size() > 0)
                    {
                        break;
                    }
                }
                if (plane_no == plane_no_per_die)
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }

    bool suspensionRequired = false;
    ChipStatus cs = _NVMController->GetChipStatus(chip);
    switch (cs)
    {
        case ChipStatus::IDLE:
            break;
        case ChipStatus::WRITING:
            if (!programSuspensionEnabled || _NVMController->HasSuspendedCommand(chip))
            {
                return false;
            }
            if (_NVMController->Expected_finish_time(chip) - Simulator->Time() < writeReasonableSuspensionTimeForRead)
            {
                return false;
            }
            suspensionRequired = true;
            SUS_CAUSE_WR = suspensionRequired;
        case ChipStatus::ERASING:
            if (!eraseSuspensionEnabled || _NVMController->HasSuspendedCommand(chip))
            {
                return false;
            }
            if (_NVMController->Expected_finish_time(chip) - Simulator->Time() < eraseReasonableSuspensionTimeForRead)
            {
                return false;
            }
            suspensionRequired = true;
        default:
            return false;
    }


    flash_die_ID_type dieID;
    flash_page_ID_type pageID;

    if (false == bHolbAvoidEnabled)
    {
        dieID = sourceQueue1->front()->Address.DieID;
        pageID = sourceQueue1->front()->Address.PageID;
    }

#if PATCH_SEGMENT_REQ

    /*
    flash_page_ID_type blockID = sourceQueue1->front()->Address.BlockID;
    int cnt_mergible_subpgs = 0;
    for (unsigned int i = 0; i < die_no_per_chip; i++) {

        for (Flash_Transaction_Queue::iterator it = sourceQueue1->begin(); it != sourceQueue1->end();) {
            if ((*it)->Address.DieID == dieID && (*it)->Address.BlockID == blockID && (*it)->Address.PageID == pageID) {
                cnt_mergible_subpgs++;
            }
            it++;
        }

        if (sourceQueue2 != NULL) {
            for (Flash_Transaction_Queue::iterator it = sourceQueue2->begin(); it != sourceQueue2->end();) {
                if ((*it)->Address.DieID == dieID && (*it)->Address.BlockID == blockID && (*it)->Address.PageID == pageID) {
                    cnt_mergible_subpgs++;
                }
                it++;
            }
        }
    }
    std::cout << "cnt_mergible_subpgs:" <<cnt_mergible_subpgs << std::endl;
    */
#endif
    if (false == bHolbAvoidEnabled)
    {
        unsigned int planeVector = 0;
        for (unsigned int i = 0; i < die_no_per_chip; i++)
        {
            transaction_dispatch_slots.clear();
            planeVector = 0;
            for (Flash_Transaction_Queue::iterator it = sourceQueue1->begin(); it != sourceQueue1->end();)
            {
                if ((*it)->Address.DieID == dieID && !(planeVector & 1 << (*it)->Address.PlaneID))
                {
                    //Check for identical pages when running multiplane command
                    if (planeVector == 0 || (*it)->Address.PageID == pageID)
                    {
                        (*it)->SuspendRequired = suspensionRequired;
                        planeVector |= 1 << (*it)->Address.PlaneID;
                        transaction_dispatch_slots.push_back(*it);
                        sourceQueue1->remove(it++);
                        continue;
                    }
                }
                it++;
            }

            if (sourceQueue2 != NULL && transaction_dispatch_slots.size() < plane_no_per_die)
            {
                for (Flash_Transaction_Queue::iterator it = sourceQueue2->begin(); it != sourceQueue2->end();)
                {
                    if ((*it)->Address.DieID == dieID && !(planeVector & 1 << (*it)->Address.PlaneID))
                    {
                        //Check for identical pages when running multiplane command
                        if (planeVector == 0 || (*it)->Address.PageID == pageID)
                        {
                            (*it)->SuspendRequired = suspensionRequired;
                            planeVector |= 1 << (*it)->Address.PlaneID;
                            transaction_dispatch_slots.push_back(*it);
                            sourceQueue2->remove(it++);
                            continue;
                        }
                    }
                    it++;
                }
            }

            if (transaction_dispatch_slots.size() > 0)
            {
                _NVMController->Send_command_to_chip(transaction_dispatch_slots, 0);
            }
            transaction_dispatch_slots.clear();
            dieID = (dieID + 1) % die_no_per_chip;
        }
    }
    else
    {
        NVM_Transaction_Flash* pstDispatch;
        unsigned int nLastPlane = ppnLastPlane[chip->ChannelID][chip->ChipID];
        unsigned int nPlane;
        transaction_dispatch_slots.clear();
        for (unsigned int plane_no = 0; plane_no < plane_no_per_die; plane_no++)
        {
            if (false == bPIREnabled)
            {
                nPlane = (nLastPlane + plane_no) % plane_no_per_die;
                if (transaction_dispatch_slots.size() == 0)
                {
                    if (UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].size() > 0)
                    {
                        pstDispatch = UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].findNode();
                        if (nullptr != pstDispatch)
                        {
                            transaction_dispatch_slots.push_back(pstDispatch);
                            UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].removeNode(pstDispatch);
                            pageID = (*transaction_dispatch_slots.begin())->Address.PageID;
                            ppnLastPlane[chip->ChannelID][chip->ChipID] = (nPlane + 1) % plane_no_per_die;
                        }
                    }
                }
                else
                {
                    if (UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].size() > 0)
                    {
                        pstDispatch = UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].findNode();
                        if ((nullptr != pstDispatch) && (pageID == pstDispatch->Address.PageID))
                        {
                            transaction_dispatch_slots.push_back(pstDispatch);
                            UserReadTRListHAL[chip->ChannelID][chip->ChipID][nPlane].removeNode(pstDispatch);
                            ppnLastPlane[chip->ChannelID][chip->ChipID] = (nPlane + 1) % plane_no_per_die;
                        }
                    }
                }
            }
            else
            {
                if (UserReadTRListHAL[chip->ChannelID][chip->ChipID][plane_no].size() > 0)
                {
                    pstDispatch = UserReadTRListHAL[chip->ChannelID][chip->ChipID][plane_no].findNode();
                    if (nullptr != pstDispatch)
                    {
                        transaction_dispatch_slots.push_back(pstDispatch);
                        UserReadTRListHAL[chip->ChannelID][chip->ChipID][plane_no].removeNode(pstDispatch);
                    }
                }
            }
        }
        if (transaction_dispatch_slots.size() > 0)
        {
            _NVMController->Send_command_to_chip(transaction_dispatch_slots, 0);
        }
        transaction_dispatch_slots.clear();
    }

    return true;
}

bool TSU_OutOfOrder::service_write_transaction(NVM::FlashMemory::Flash_Chip* chip)
{
    Flash_Transaction_Queue *sourceQueue1 = NULL, *sourceQueue2 = NULL;

    //If flash transactions related to GC are prioritzed (non-preemptive execution mode of GC), then GC queues are checked first
    if (ftl->GC_and_WL_Unit->GC_is_in_urgent_mode(chip))
    {
        if (GCWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &GCWriteTRQueue[chip->ChannelID][chip->ChipID];
            if (UserWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
            {
                sourceQueue2 = &UserWriteTRQueue[chip->ChannelID][chip->ChipID];
            }
        }
        else if (GCEraseTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            return false;
        }
        else if (UserWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &UserWriteTRQueue[chip->ChannelID][chip->ChipID];
        }
        else
        {
            return false;
        }
    }
    else
    {
        //If GC is currently executed in the preemptive mode, then user IO transaction queues are checked first

        if (UserWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            //std::cout << ".";
            sourceQueue1 = &UserWriteTRQueue[chip->ChannelID][chip->ChipID];
            if (GCWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
            {
                sourceQueue2 = &GCWriteTRQueue[chip->ChannelID][chip->ChipID];
            }
        }
        else if (GCWriteTRQueue[chip->ChannelID][chip->ChipID].size() > 0)
        {
            sourceQueue1 = &GCWriteTRQueue[chip->ChannelID][chip->ChipID];
        }
        else
        {
            return false;
        }
    }


    bool suspensionRequired = false;
    ChipStatus cs = _NVMController->GetChipStatus(chip);
    switch (cs)
    {
        case ChipStatus::IDLE:
            break;
        case ChipStatus::ERASING:
            if (!eraseSuspensionEnabled || _NVMController->HasSuspendedCommand(chip))
                return false;
            if (_NVMController->Expected_finish_time(chip) - Simulator->Time() < eraseReasonableSuspensionTimeForWrite)
                return false;
            suspensionRequired = true;
        default:
            return false;
    }

    flash_die_ID_type dieID = sourceQueue1->front()->Address.DieID;
    flash_page_ID_type pageID = sourceQueue1->front()->Address.PageID;
    flash_block_ID_type blockID = sourceQueue1->front()->Address.BlockID;
    bool bypass = false;
    unsigned int planeVector = 0;
    NVM_Transaction_Flash_WR* bypassed_wr = NULL;

    for (unsigned int i = 0; i < die_no_per_chip; i++)
    {
        transaction_dispatch_slots.clear();
        planeVector = 0;

        for (Flash_Transaction_Queue::iterator it = sourceQueue1->begin(); it != sourceQueue1->end();)
        {
            if ((((NVM_Transaction_Flash_WR*)*it)->RelatedRead != NULL) && (bypass != true))
            {
                bypass = true;
                bypassed_wr = (NVM_Transaction_Flash_WR*)*it;
            }

            if (((NVM_Transaction_Flash_WR*)*it)->RelatedRead == NULL && (*it)->Address.DieID == dieID && !(planeVector & 1 << (*it)->Address.PlaneID))
            {
                //Check for identical pages when running multiplane command
                if (planeVector == 0 || (*it)->Address.PageID == pageID)
                {
                    (*it)->SuspendRequired = suspensionRequired;
                    planeVector |= 1 << (*it)->Address.PlaneID;
                    transaction_dispatch_slots.push_back(*it);

                    // sourceQueue1->remove(it++);
                    it++;
                    continue;
                }
            }
            it++;
        }

        if (transaction_dispatch_slots.size() > 0)
        {
            if ((bypass == true) && (bypassed_wr->Address.PageID == transaction_dispatch_slots.front()->Address.PageID))
            {
                //when bypass is true, and transaction address is same with bypassed transaction. clear dispatch slots queue.- for multi-plane operation later.
                transaction_dispatch_slots.clear();
            }
            else
            {
                //when bypass is false, delete transactions from sourceQueue. --> normal execution.
                //when bypass is true, but transaction address is different.  --> normal execution.
                for (std::list<NVM_Transaction_Flash*>::iterator it = transaction_dispatch_slots.begin(); it != transaction_dispatch_slots.end(); it++)
                {
                    sourceQueue1->remove(*it);
                }
            }
        }

        if (sourceQueue2 != NULL)
        {
            for (Flash_Transaction_Queue::iterator it = sourceQueue2->begin(); it != sourceQueue2->end();)
            {
                if (((NVM_Transaction_Flash_WR*)*it)->RelatedRead == NULL && (*it)->Address.DieID == dieID && !(planeVector & 1 << (*it)->Address.PlaneID))
                {
                    //Check for identical pages when running multiplane command
                    if (planeVector == 0 || (*it)->Address.PageID == pageID)
                    {
                        (*it)->SuspendRequired = suspensionRequired;
                        planeVector |= 1 << (*it)->Address.PlaneID;
                        transaction_dispatch_slots.push_back(*it);
                        sourceQueue2->remove(it++);
                        continue;
                    }
                }
                it++;
            }
        }

        if (transaction_dispatch_slots.size() > 0)
        {
            _NVMController->Send_command_to_chip(transaction_dispatch_slots, 0);
        }
        transaction_dispatch_slots.clear();
        dieID = (dieID + 1) % die_no_per_chip;
    }

    return true;
}

bool TSU_OutOfOrder::service_erase_transaction(NVM::FlashMemory::Flash_Chip* chip)
{
    if (_NVMController->GetChipStatus(chip) != ChipStatus::IDLE)
    {
        return false;
    }

    Flash_Transaction_Queue* source_queue = &GCEraseTRQueue[chip->ChannelID][chip->ChipID];
    if (source_queue->size() == 0)
    {
        return false;
    }

    flash_die_ID_type dieID = source_queue->front()->Address.DieID;
    unsigned int planeVector = 0;
    for (unsigned int i = 0; i < die_no_per_chip; i++)
    {
        transaction_dispatch_slots.clear();
        planeVector = 0;

        for (Flash_Transaction_Queue::iterator it = source_queue->begin(); it != source_queue->end();)
        {
            if (((NVM_Transaction_Flash_ER*)*it)->Page_movement_activities.size() == 0 && (*it)->Address.DieID == dieID && !(planeVector & 1 << (*it)->Address.PlaneID))
            {
                planeVector |= 1 << (*it)->Address.PlaneID;
                transaction_dispatch_slots.push_back(*it);
                source_queue->remove(it++); //it++
                continue;
            }
            it++;
        }
        if (transaction_dispatch_slots.size() > 0)
        {
            _NVMController->Send_command_to_chip(transaction_dispatch_slots, 0);
        }
        transaction_dispatch_slots.clear();
        dieID = (dieID + 1) % die_no_per_chip;
    }

    return true;
}
}
