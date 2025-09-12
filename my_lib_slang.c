#include "my_lib_slang.h"

Player new_player(const char* name, int score) {
    Player p = {name, score};
    return p;
}
