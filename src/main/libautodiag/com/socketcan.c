#include "libautodiag/com/socketcan.h"

#include <errno.h>
#include <string.h>

#if defined(__linux__) && defined(__has_include)

#if __has_include(<linux/can.h>) && __has_include(<linux/can/raw.h>) && __has_include(<net/if.h>)
#define AD_HAS_SOCKETCAN 1
#endif

#endif

#if AD_HAS_SOCKETCAN

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int ad_socketcan_open(AdSocketCan *can, const char *interface_name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int recv_own_msgs = 0;

    if (can == NULL || interface_name == NULL) {
        errno = EINVAL;
        return -1;
    }

    can->fd = -1;

    can->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can->fd < 0) {
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

    if (ioctl(can->fd, SIOCGIFINDEX, &ifr) < 0) {
        close(can->fd);
        can->fd = -1;
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (setsockopt(
            can->fd,
            SOL_CAN_RAW,
            CAN_RAW_RECV_OWN_MSGS,
            &recv_own_msgs,
            sizeof(recv_own_msgs)) < 0) {
        close(can->fd);
        can->fd = -1;
        return -1;
    }

    if (bind(can->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(can->fd);
        can->fd = -1;
        return -1;
    }

    return 0;
}

void ad_socketcan_close(AdSocketCan *can)
{
    if (can == NULL || can->fd < 0) {
        return;
    }

    close(can->fd);
    can->fd = -1;
}

int ad_socketcan_send(AdSocketCan *can, const AdCanFrame *frame)
{
    struct can_frame can_frame;
    ssize_t result;

    if (can == NULL || frame == NULL || can->fd < 0) {
        errno = EINVAL;
        return -1;
    }

    if (frame->fd || frame->size > CAN_MAX_DLEN) {
        errno = EINVAL;
        return -1;
    }

    memset(&can_frame, 0, sizeof(can_frame));

    can_frame.can_id = frame->id & CAN_SFF_MASK;

    if (frame->extended) {
        can_frame.can_id = frame->id & CAN_EFF_MASK;
        can_frame.can_id |= CAN_EFF_FLAG;
    }

    can_frame.can_dlc = frame->size;
    memcpy(can_frame.data, frame->data, frame->size);

    result = write(can->fd, &can_frame, sizeof(can_frame));

    if (result < 0) {
        return -1;
    }

    if ((size_t)result != sizeof(can_frame)) {
        errno = EIO;
        return -1;
    }

    return 0;
}

int ad_socketcan_receive(AdSocketCan *can, AdCanFrame *frame)
{
    struct can_frame can_frame;
    ssize_t result;

    if (can == NULL || frame == NULL || can->fd < 0) {
        errno = EINVAL;
        return -1;
    }

    result = read(can->fd, &can_frame, sizeof(can_frame));

    if (result < 0) {
        return -1;
    }

    if ((size_t)result != sizeof(can_frame)) {
        errno = EIO;
        return -1;
    }

    memset(frame, 0, sizeof(*frame));

    frame->id = can_frame.can_id & CAN_EFF_MASK;
    frame->extended = (can_frame.can_id & CAN_EFF_FLAG) != 0;
    frame->size = can_frame.can_dlc;

    memcpy(frame->data, can_frame.data, frame->size);

    return 0;
}

#else

int ad_socketcan_open(AdSocketCan *can, const char *interface_name)
{
    (void)interface_name;

    if (can == NULL) {
        errno = EINVAL;
        return -1;
    }

    can->fd = -1;
    errno = ENOTSUP;
    return -1;
}

void ad_socketcan_close(AdSocketCan *can)
{
    if (can != NULL) {
        can->fd = -1;
    }
}

int ad_socketcan_send(AdSocketCan *can, const AdCanFrame *frame)
{
    (void)can;
    (void)frame;

    errno = ENOTSUP;
    return -1;
}

int ad_socketcan_receive(AdSocketCan *can, AdCanFrame *frame)
{
    (void)can;
    (void)frame;

    errno = ENOTSUP;
    return -1;
}

#endif