#ifndef MAILMAN_PD_CAPS_H
#define MAILMAN_PD_CAPS_H

#include <stdbool.h>

// Snapshot of fixed USB-PD source capabilities (currents in Amps).
typedef struct mailman_pd_caps_s {
    bool have5;  float cur5;
    bool have9;  float cur9;
    bool have15; float cur15;
    bool have20; float cur20;
} mailman_pd_caps_t;

void mailman_update_pd_caps(const mailman_pd_caps_t *caps);
bool mailman_get_pd_caps(mailman_pd_caps_t *out);

#endif // MAILMAN_PD_CAPS_H
