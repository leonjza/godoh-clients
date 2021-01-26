#ifndef GODOH_OPTIONS_H
#define GODOH_OPTIONS_H

/*
    These options should be modified according
    to how the implant should behave. When using
    the build.sh script and Cmake, the DOMAIN
    and MAX_SLEEP should be specified there.
*/

#ifndef DOMAIN
#define DOMAIN "c2.test"
#endif

#ifndef MAX_SLEEP
#define MAX_SLEEP 2
#endif

#define KEY "2589213f0c51583dcbaacbe0005e5908"

#endif //GODOH_OPTIONS_H
