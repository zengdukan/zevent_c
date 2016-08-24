/************************************************************************/
// 1.�����¼���ʱ�Ƴ���ɾ���ͷſռ�
// 2.��־λ�Ŀ���
// 3.������ʱ��ôɾ���¼�
// 4.connecter��Ĺرղ���Ҫע��ͬ��������Ҫ����
/************************************************************************/
#ifndef _ZEVENT_H_
#define _ZEVENT_H_


#include "zevent_connector.h"
#include "zevent_accepter.h"
#include "zevent_event.h"
#include "zevent_looper.h"
#include "zevent_pipe.h"
#include "zevent_reactor.h"
#include "zevent_timer.h"
#include "zevent_typedef.h"
#include "zbaselib_typedef.h"

ZBASELIB_BEGIN_DECL

// ����ʱ��
ZLIB_API void zevent_update_current_time();

//��ȡ��ǰϵͳʱ�����ֵ
ZLIB_API llong zevent_get_current_time();

/*
��ʼ��ģ��:
1.��ʼ��socket
2.����default_looper
*/
ZLIB_API int zevent_init();

/*
ע��ģ��:
1.ע��socket
2.����default_looper
*/
ZLIB_API void zevent_deinit();

// ��ȡdefault_looper
ZLIB_API zevent_looper* zevent_get_default_looper();

ZBASELIB_END_DECL

#endif
