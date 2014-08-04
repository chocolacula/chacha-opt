#if defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>
#endif

static cycles_t
cpucycles_impl(void) {
#if defined(HAVE_GETTIMEOFDAY)
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((cycles_t)t.tv_sec * 1000000) + (cycles_t)t.tv_usec;
#else
	static cycles_t cludge = 0;
	return cludge++;
#endif
}

static const char *
cpucycles_units_impl(void) {
#if defined(HAVE_GETTIMEOFDAY)
		return "us";
#else
		return "cludges";
#endif
}