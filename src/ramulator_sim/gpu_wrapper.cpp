#include <map>

#include "gpu_wrapper.h"
#include "Config.h"
#include "Request.h"
#include "MemoryFactory.h"
#include "Memory.h"
#include "DDR3.h"
#include "DDR4.h"
#include "LPDDR3.h"
#include "LPDDR4.h"
#include "GDDR5.h"
#include "WideIO.h"
#include "WideIO2.h"
#include "HBM.h"
#include "SALP.h"


// using namespace ramulator;



static map<string, function<MemoryBase *(const Config&, int)> > name_to_func = {
    {"DDR3", &MemoryFactory<DDR3>::create}, {"DDR4", &MemoryFactory<DDR4>::create},
    {"LPDDR3", &MemoryFactory<LPDDR3>::create}, {"LPDDR4", &MemoryFactory<LPDDR4>::create},
    {"GDDR5", &MemoryFactory<GDDR5>::create},
    {"WideIO", &MemoryFactory<WideIO>::create}, {"WideIO2", &MemoryFactory<WideIO2>::create},
    {"HBM", &MemoryFactory<HBM>::create},
    {"SALP-1", &MemoryFactory<SALP>::create}, {"SALP-2", &MemoryFactory<SALP>::create}, {"SALP-MASA", &MemoryFactory<SALP>::create},
};


GpuWrapper::GpuWrapper(const Config& configs, int cacheline,  memory_partition_unit *mp):
    read_cb_func(std::bind(&GpuWrapper::readComplete, this, std::placeholders::_1)),
    write_cb_func(std::bind(&GpuWrapper::writeComplete, this, std::placeholders::_1))
{
    const string& std_name = configs["standard"];
    assert(name_to_func.find(std_name) != name_to_func.end() && "unrecognized standard name");
    mem = name_to_func[std_name](configs, cacheline);
    tCK = mem->clk_ns();
//    read_cb_func(std::bind(&GpuWrapper::readComplete, this, std::placeholders::_1));
//    write_cb_func(std::bind(&GpuWrapper::writeComplete, this, std::placeholders::_1));
    r_returnq = new fifo_pipeline<mem_fetch>("ramulatorreturnq", 0, 1024);
    m_memory_partition_unit = mp;
}


GpuWrapper::~GpuWrapper() {
    delete mem;
}

void GpuWrapper::cycle()
{
    mem->tick();
}

bool GpuWrapper::send(Request req)
{
    return mem->send(req);
}

void GpuWrapper::finish(void) {
    mem->finish();
}

bool GpuWrapper::full(int request_type, long request_addr )
{
    // 1 is for write, while 0 for read
    if (request_type == 0)
    {
        return mem->full(Request::Type::READ, request_addr);
    } else {
        return mem->full(Request::Type::WRITE, request_addr);
    }
}

// Request GpuWrapper::address_convert(std::mem_fetch &mf) {
//     if (mf.is_write())
//     {
//         Request req((long)mf.get_addr(), Request::Type::WRITE, write_cb_func, mf.get_sid());
//         return req;
//     } else {
//         Request req((long)mf.get_addr(), Request::Type::READ, read_cb_func, mf.get_sid());
//         return req;
//     }
// }

//In this function, I can put the request into the DRAM to L2 queue.
void GpuWrapper::readComplete(Request& req) {
    //if(!r_returnq->full()) FIX ME!!!!!!!!!!
    // fprintf(stderr,"enter here_READ COMPLETE`\n");
    //fprintf(stderr, "Readthe returned req addres is %ld\n", req.mf->get_addr());
    auto& mf_queue = mem_temp_r.find(req.mf->get_addr())->second;
    mem_fetch* mf = mf_queue.front();
    mf_queue.pop_front();
    if (!mf_queue.size())
        mem_temp_r.erase(req.mf->get_addr()) ;
    mf->set_status(IN_PARTITION_MC_RETURNQ, gpu_sim_cycle + gpu_tot_sim_cycle);
    mf->set_reply();
    r_returnq->push(mf);
    //dram_L2_queue_push(mf);
}

void GpuWrapper::writeComplete(Request& req) {
    //fprintf(stderr,"enter here_Write COMPLETE`\n");
    //fprintf(stderr, "Writethe returned req addres is %ld\n", req.mf->get_addr());
    auto& mf_queue = mem_temp_w.find(req.mf->get_addr())->second;
    mem_fetch* mf = mf_queue.front();
    mf_queue.pop_front();
    if (!mf_queue.size())
        mem_temp_w.erase(req.mf->get_addr()) ;
    mf->set_status(IN_PARTITION_MC_RETURNQ, gpu_sim_cycle + gpu_tot_sim_cycle);
    if (!( mf->get_access_type() != L1_WRBK_ACC && mf->get_access_type() != L2_WRBK_ACC )) {
        m_memory_partition_unit->set_done(mf);
        delete mf;
    } else {
        mf->set_reply();
        r_returnq->push(mf);
    }

}


void GpuWrapper::push(mem_fetch* mf)
{
    Request *req;
    fprintf(stderr, "The sid number is %d\n", mf->get_sid());
    if (mf->is_write())
    {
        req = new Request((long)mf->get_addr(), Request::Type::WRITE, write_cb_func, mf->get_sid());
    } else {
        req = new Request((long)mf->get_addr(), Request::Type::READ, read_cb_func, mf->get_sid());
    }
    req->mf = mf;
    bool accepted = send(*req);
    assert(accepted);
    if (accepted)
    {
        if (mf->is_write()) {
            mem_temp_w[mf->get_addr()].push_back(mf);
            // fprintf(stderr, "the pushed_write req addres is %ld \n", mf->get_addr());
        } else {
            mem_temp_r[mf->get_addr()].push_back(mf);
            //fprintf(stderr, "the pushed_read req addres is %ld\n", mf->get_addr());
        }
    }

    delete req;
}

bool GpuWrapper::r_returnq_full() const
{
    return r_returnq->full();
}


class mem_fetch* GpuWrapper::r_return_queue_top() const
{
    return r_returnq->top();
}

class mem_fetch* GpuWrapper::r_return_queue_pop() const
{
    return r_returnq->pop();
}
