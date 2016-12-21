class IOCtx;

class Timeout {
	public:
		Timeout(int ms, std::function<int (IOCtx*)>);
		friend bool operator <(const Timeout& t1, const Timeout& t2);

		std::function<int (IOCtx*)>cb;
		int ms;
};

class IOCtx {
	public:
		IOCtx();
		~IOCtx();

		void add_socket(int s, std::function<int (IOCtx*, int)> cb);
		void remove_socket(int s);
		void ioloop_run();
		void set_timeout(int ms, std::function<int (IOCtx*)> cb);

	private:
		std::unordered_map<int, std::function<int (IOCtx*, int)>> cbmap;
		std::priority_queue<Timeout, std::vector<Timeout>, std::less<Timeout>> timeouts;
		int efd;

		const int epoll_events;
};
