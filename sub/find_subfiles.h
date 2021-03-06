/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_FIND_SUBFILES_H
#define MPLAYER_FIND_SUBFILES_H

#include <stdbool.h>

struct subfn {
    int priority;
    char *fname;
    char *lang;
};

struct mpv_global;
struct subfn *find_text_subtitles(struct mpv_global *global, const char *fname);

bool mp_might_be_subtitle_file(const char *filename);

#endif /* MPLAYER_FINDFILES_H */
