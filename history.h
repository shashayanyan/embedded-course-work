#ifndef _HISTORY_H_
#define _HISTORY_H_

void history_init(void);
void history_add(const char* line);

// Asynchronous request: dir = -1 (older/UP), 1 (newer/DOWN)
void history_request(int dir, void (*callback)(const char*));

#endif /* _HISTORY_H_ */