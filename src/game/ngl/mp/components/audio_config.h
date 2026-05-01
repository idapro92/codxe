#pragma once

struct AudioEntry { const char* name; unsigned int offset; unsigned int size; };

static const AudioEntry g_audioTable[] = {
    { "do_you_understand", 0, 75856 },
    { "entirely_possible", 75856, 437472 },
    { "lets_bounce", 513328, 254016 },
    { "paradox_is_garbage", 767344, 269904 },
    { "strike_bounce", 1037248, 229328 },
    { "understand_angle", 1266576, 442768 },
    { 0, 0, 0 }
};
