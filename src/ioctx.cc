#include <iostream>
#include <functional>
#include <unordered_map>
#include <queue>

#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include "ioctx.h"

uint64_t current_time_ms() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

Timeout::Timeout(int ms, std::function<int (IOCtx*)> cb)
: ms(ms),
  cb(cb)
{
}

bool operator <(const Timeout& t1, const Timeout& t2) {
	return t1.ms > t2.ms;
}

#define EPOLL_EVENTS 256
IOCtx::IOCtx()
: epoll_events(EPOLL_EVENTS)
{
	this->efd = epoll_create1(0);
	if (this->efd == -1) {
		std::cerr << strerror(errno) << std::endl;
		exit(-1);
	}
}

IOCtx::~IOCtx() {
}

void IOCtx::add_socket(int s, std::function<int (IOCtx *, int)> cb) {
	// Yes we could just back this callback into the event, but this is
	// meant to be transposable between select vs epoll
	struct epoll_event evt = {EPOLLIN, (epoll_data_t){.u64=(uint64_t)s}};
	if (-1 == epoll_ctl(this->efd, EPOLL_CTL_ADD, s, &evt)) {
		std::cerr << strerror(errno) << std::endl;
		exit(-1);
	}
	this->cbmap[s] = cb;
}

void IOCtx::remove_socket(int s) {
	epoll_ctl(this->efd, EPOLL_CTL_DEL, s, NULL);
}

void IOCtx::set_timeout(int ms, std::function<int (IOCtx *)> cb) {
	int absolute = current_time_ms() + ms;
	this->timeouts.push(Timeout(absolute, cb));
}

void IOCtx::ioloop_run() {
	int ready;
	struct epoll_event events[this->epoll_events];
	while (1) {
		int sleeptime = -1;
		int tms, now;
		if (!this->timeouts.empty()) {
			sleeptime = 0;
			tms = this->timeouts.top().ms;
			now = current_time_ms();
			if (now < tms) {
				sleeptime = tms - now;
			}
		}
		
		if (-1 == (ready = epoll_wait(this->efd, events, this->epoll_events, sleeptime))) {
			std::cerr << "epoll_wait:" << strerror(errno) << std::endl;
			exit(-1);
		}
		/* Sockets */
		for (int i = 0; i < ready; i++) {
			int s = (int)(events[i].data.u64);
			this->cbmap[s](this, s);
		}

		/* Timeouts */
		if (!this->timeouts.empty()) {
			now = current_time_ms();
			tms = this->timeouts.top().ms;
			while (tms <= now) {
				this->timeouts.top().cb(this);
				this->timeouts.pop();
				if (this->timeouts.empty())
					break;
				tms = this->timeouts.top().ms;
			}
		}
	}

}
