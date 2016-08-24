#include "zevent.h"
#include <assert.h>

enum {
	PIPE1_MSG_A,
	PIPE1_MSG_B,
	PIPE1_MSG_C,
};

typedef struct pipe_msg_s
{
	int id;
	int value;
} pipe_msg;

static int timer1_cb(void* param1, void* param2)
{
	int* count = (int*)param1;
	printf("count = %d\n", *count);
	(*count)++;

	return ZEVENT_TIMER_CONTINUE;
}

static int timer2_cb(void* param1, void* param2)
{
	printf("10 sec \n");
	return ZEVENT_TIMER_OK;
}

static int timer_pipe_send_msg(void* param1, void* param2)
{
	pipe_msg msg;
	zevent* pipe;

#ifdef WIN32
	int id = rand()%3;
#else
	int id = random()%3;
#endif
	msg.id = id;
	msg.value = id;

	printf("gen msg:(%d, %d)\n", msg.id, msg.value);
	pipe = (zevent*)param1;
	assert(zevent_pipe_send_msg(pipe, (void*)&msg, sizeof(msg), msg.id) == 0);

	return ZEVENT_TIMER_CONTINUE;
}


static int msg_process(void* data, int data_size, int msg_type,
	void* param1, void* param2)
{
	pipe_msg* msg = (pipe_msg*)data;
	printf("msg = [%d, %d], %d\n", msg->id, msg->value, msg_type);
	switch(msg_type)
	{
	case PIPE1_MSG_A:
		assert(msg->id == msg->value);
		assert(msg->id == PIPE1_MSG_A);
		break;
	case PIPE1_MSG_B:
		assert(msg->id == msg->value);
		assert(msg->id == PIPE1_MSG_B);
		break;
	case PIPE1_MSG_C:
		assert(msg->id == msg->value);
		assert(msg->id == PIPE1_MSG_C);
		break;
	default:break;
	}

	return 0;
}

#if 1
int main(int argc, char* argv[])
{
	//zevent_looper* looper = NULL;
	zevent* timer1 = NULL;
	zevent* timer2 = NULL;
	int count = 1;
	zevent* pipe1 = NULL;
	zevent* pipe_timer = NULL;

	assert(zevent_init() == 0);
	assert(zevent_looper_start(zevent_get_default_looper()) == 0);

	timer1 = zevent_timer_new(1000, timer1_cb);
	zevent_timer_intial(timer1, &count, NULL);
	timer2 = zevent_timer_new(10000, timer2_cb);

	zevent_looper_add_event(zevent_get_default_looper(), timer1);
	zevent_looper_add_event(zevent_get_default_looper(), timer2);

	pipe1 = zevent_pipe_new(msg_process);
	assert(pipe1 != NULL);
	assert(zevent_looper_add_event(zevent_get_default_looper(), pipe1) == 0);

	pipe_timer = zevent_timer_new(1000, timer_pipe_send_msg);
	zevent_timer_intial(pipe_timer, pipe1, NULL);
	assert(pipe_timer != NULL);
	assert(zevent_looper_add_event(zevent_get_default_looper(), pipe_timer) == 0);

	getchar();

	zevent_looper_del_event(zevent_get_default_looper(), timer1);
	zevent_looper_del_event(zevent_get_default_looper(), timer2);
	zevent_looper_del_event(zevent_get_default_looper(), pipe_timer);
	zevent_looper_del_event(zevent_get_default_looper(), pipe1);

	zevent_looper_stop(zevent_get_default_looper());

	zevent_delete(timer1);
	zevent_delete(timer2);
	zevent_delete(pipe_timer);
	zevent_delete(pipe1);

	zevent_deinit();

	return 0;
}
#else

extern llong zevent_current_time;

static int timer_func(void* param1, void* param2)
{
	zevent_update_current_time();
	printf("%s: %lld\n", (char*)param1, zevent_current_time);
	return ZEVENT_TIMER_CONTINUE;
}


int main(int argc, char* argv[])
{
	zevent* timer1 = NULL;
	zevent* timer2 = NULL;
	char timer1_name[] = "timer1";
	char timer2_name[] = "timer2";
	
	assert(zevent_init() == 0);
	assert(zevent_looper_start(zevent_get_default_looper()) == 0);

	timer1 = zevent_timer_new(10000, timer_func);
	zevent_timer_intial(timer1, timer1_name, NULL);
	zevent_looper_add_event(zevent_get_default_looper(), timer1);
	
	timer2 = zevent_timer_new(20000, timer_func);
	zevent_timer_intial(timer2, timer2_name, NULL);
	zevent_looper_add_event(zevent_get_default_looper(), timer2);

	getchar();
	
	zevent_looper_stop(zevent_get_default_looper());
	zevent_delete(timer1);
	zevent_delete(timer2);

	zevent_deinit();

	return 0;
}

#endif

