#ifndef W5500_ICMP_H
#define W5500_ICMP_H

#include <stdint.h>
#include <stdbool.h>

bool w5500_icmp_init(void);
void w5500_icmp_task10ms(void);

#endif // W5500_ICMP_H
