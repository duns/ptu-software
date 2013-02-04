#ifndef __EVENT_H__
#define __EVENT_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifndef EV_SYN
#define EV_SYN 0
#endif

extern  char *events[EV_MAX + 1];
extern char *keys[KEY_MAX + 1];
extern char *absval[5];
extern char *relatives[REL_MAX + 1];
extern char *absolutes[ABS_MAX + 1];
extern char *misc[MSC_MAX + 1];
extern char *leds[LED_MAX + 1];
extern char *repeats[REP_MAX + 1];
extern char *sounds[SND_MAX + 1];
extern char **names[EV_MAX + 1];

int event_loop (char *device, int pipefd);
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
#endif

