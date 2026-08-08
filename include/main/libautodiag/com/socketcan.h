#ifndef AD_SOCKETCAN_H
#define AD_SOCKETCAN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id;
    uint8_t data[64];
    uint8_t size;
    uint8_t extended;
    uint8_t fd;
} AdCanFrame;

typedef struct {
    int fd;
} AdSocketCan;

int ad_socketcan_open(AdSocketCan *can, const char *interface_name);
void ad_socketcan_close(AdSocketCan *can);
int ad_socketcan_send(AdSocketCan *can, const AdCanFrame *frame);
int ad_socketcan_receive(AdSocketCan *can, AdCanFrame *frame);

#ifdef __cplusplus
}
#endif

#endif