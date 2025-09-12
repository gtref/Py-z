typedef struct Player {
    const char* name;
    int score;
} Player;

extern Player new_player(const char* name, int score);
