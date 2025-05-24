#include <stdbool.h>
#include <inttypes.h>

struct msg_display_data {
	double velocity;
	bool is_over_limit;
};

int display_api_send(double velocity, bool is_over_limit, k_timeout_t timeout);