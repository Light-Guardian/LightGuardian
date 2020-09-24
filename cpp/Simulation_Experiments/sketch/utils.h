#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <sys/time.h>

extern bool verbose;
extern bool debug;

#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",           \
                __LINE__, __FILE__);                                          \
		fflush(stderr);							      						  \
        exit(-1);                                                             \
    }

void dprintf(char *format, ...);
void vprintf(char *format, ...);
void panic(char *format, ...);

class Timer
{
public:
	Timer();
	~Timer();

	void Start();
	double StepTime(); // return step process time (sec)
	double Finish(bool force_update=false);
	double WholeTime();

private:
	struct timeval st_time, ed_time, ls_time;
	bool stopped;
};

#endif