#ifndef _STREAM_H_
#define _STREAM_H_

#include <stdint.h>
#include <stddef.h>

void stream_init(void);

// event-oriented specification from the lecture
void stream_set_read_listener(int stream, void (*listener) (void*), void* cookie);
int stream_read(int stream, uint8_t* buffer, size_t length);

void stream_set_write_listener(int stream, void (*listener) (void*), void* cookie);
int stream_write(int stream, uint8_t* buffer, size_t length);

#endif /* _STREAM_H_ */