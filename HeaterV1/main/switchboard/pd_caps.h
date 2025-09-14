#ifndef SWITCHBOARD_PD_CAPS_H
#define SWITCHBOARD_PD_CAPS_H

#include <stdbool.h>

// Snapshot of fixed USB-PD source capabilities the system cares about.
// Currents are in Amps (max current available for that fixed PDO).
typedef struct switchboard_pd_caps_s {
    bool have5;  float cur5;   // 5V available? max current
    bool have9;  float cur9;   // 9V
    bool have15; float cur15;  // 15V
    bool have20; float cur20;  // 20V
} switchboard_pd_caps_t;

// Producer side (PD controller driver) calls this once capabilities are parsed.
void switchboard_update_pd_caps(const switchboard_pd_caps_t *caps);

// Consumer side (UI) polls; returns true if capabilities have been set at least once.
bool switchboard_get_pd_caps(switchboard_pd_caps_t *out);

#endif // SWITCHBOARD_PD_CAPS_H
