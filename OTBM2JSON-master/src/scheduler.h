////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_SCHEDULER_H
#define __SURVIVALSERV_SCHEDULER_H

#include <functional>
#include "survivalsystem.h"

class Map;

class SchedulerTask : public std::unary_function<Map*, int> {
		  public:
					 inline __int64 getCycle() const {
								return _cycle;
					 }

					 // definition to make sure lower cycles end up front
					 // in the priority_queue used in the scheduler
					 inline bool operator<(const SchedulerTask& other) const {
								return getCycle() > other.getCycle();
					 }

					 virtual result_type operator()(const argument_type&) = 0;

					 virtual void setTicks(const __int64 ticks) {
								_cycle = SURVIVALSYS_TIME() + ticks;;
					 }

					 virtual ~SchedulerTask() { };
		  protected:
					 __int64 _cycle;
};

template<class Functor>
class TSchedulerTask : public SchedulerTask {
		  public:
					 TSchedulerTask(Functor f) : _f(f) { }

					 virtual result_type operator()(const argument_type& arg) {
								_f(arg);
                return 0;
					 }

		  protected:
					 Functor _f;
};

template<class Functor>
TSchedulerTask<Functor>* makeTask(Functor f) {
		  return new TSchedulerTask<Functor>(f);
}

template<class Functor>
TSchedulerTask<Functor>* makeTask(__int64 ticks, Functor f) {
		  TSchedulerTask<Functor> *t = new TSchedulerTask<Functor>(f);
		  t->setTicks(ticks);
		  return t;
}

class lessSchedTask : public std::binary_function<SchedulerTask*, SchedulerTask*, bool> {
		  public:
		  bool operator()(SchedulerTask*& t1, SchedulerTask*& t2) {
					 return *t1 < *t2;
		  }
};

template<class Functor, class Functor2,  class Arg>
class TCallList : public SchedulerTask {
		  public:
					 TCallList(Functor f, Functor2 f2, std::list<Arg>& call_list, __int64 interval) : _f(f), _f2(f2), _list(call_list), _interval(interval) {
					 }

					 result_type operator()(const argument_type& arg) {
                              if(!_f2(arg)){   
								result_type ret = _f(arg, _list.front());
								_list.pop_front();
								if (!_list.empty()) {
										  SchedulerTask* newtask = new TCallList<Functor, Functor2, Arg>(_f, _f2, _list, _interval);
										  newtask->setTicks(_interval);
										  arg->addEvent(newtask);
								}
								return ret;
                          }	
										return result_type();	
					 }
		  protected:
					 Functor _f;
					 Functor2 _f2;
					 std::list<Arg> _list;
					 __int64 _interval;
};

template<class Functor, class Functor2, class Arg>
SchedulerTask* makeTask(__int64 ticks, Functor f, std::list<Arg>& call_list, __int64 interval, Functor2 f2) {
		  TCallList<Functor, Functor2, Arg> *t = new TCallList<Functor, Functor2, Arg>(f, f2, call_list, interval);
		  t->setTicks(ticks);
		  return t;
}

#endif
