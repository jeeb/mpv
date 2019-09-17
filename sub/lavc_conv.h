#ifndef MPV_LAVC_CONV_H
#define MPV_LAVC_CONV_H

struct decoded_subtitle {
    double pts;
    double duration;
    char **lines;
};

#endif
