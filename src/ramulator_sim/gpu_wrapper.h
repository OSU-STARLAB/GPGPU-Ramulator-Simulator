#ifndef __GPU_WRAPPER_H
#define __GPU_WRAPPER_H

#include <string>
#include <deque>
#include <functional>
#include <tuple>
#include "Config.h"
#include "../gpgpu-sim/delayqueue.h"
#include "../gpgpu-sim/mem_fetch.h"
#include "../gpgpu-sim/l2cache.h"
#include "../gpgpu-sim/gpu-sim.h"
//using namespace std;

// namespace ramulator
// {

class Request;
class MemoryBase;

class GpuWrapper
{
private:
	MemoryBase *mem;
	fifo_pipeline<mem_fetch> *r_returnq;
	std::map<unsigned long long, std::deque<mem_fetch*> > mem_temp_r;
	std::map<unsigned long long, std::deque<mem_fetch*> > mem_temp_w;
	class  memory_partition_unit *m_memory_partition_unit;
public:
	double tCK;
	GpuWrapper(const Config& configs, int cacheline, memory_partition_unit *mp);
	~GpuWrapper();
	void cycle();
	bool send(Request req);
	void finish(void);
	bool full(int request_type, long request_addr );
	bool r_returnq_full() const;
	class mem_fetch* r_return_queue_top() const;
	class mem_fetch* r_return_queue_pop() const;
	void push(mem_fetch* mf);
	void readComplete(Request& req);
	void writeComplete(Request& req);
	std::function<void(Request&)> read_cb_func;
	std::function<void(Request&)> write_cb_func;



};



// } /*namespace ramulator*/

#endif /*__GPU_WRAPPER_H*/
