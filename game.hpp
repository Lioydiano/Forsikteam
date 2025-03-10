#include <chrono>
#include <thread>
#include <future>
#include <random>
#include <time.h>

#include "variables.hpp"

Player player;

void printField() {
    int sum = 0;
    for (auto& enemy : game::enemies)
        sum += enemy.alive;

    clearScreen(); // Clear screen (prevents flickering)
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 50; j++) {
            if (i == player.y && j == player.x) {
                if (player.alive)
                    std::cout << "\x1b[91m$\033[0m";
                else
                    std::cout << "\x1b[91m@\033[0m";
                continue;
            }
            for (int k = 0; k < game::enemies.size(); k++) {
                if (i == game::enemies[k].y && j == game::enemies[k].x) {
                    if (game::enemies[k].alive)
                        std::cout << std::string("\x1b[1m\x1B[94m") + game::enemies[k].skin + std::string("\033[0m");
                    else
                        std::cout << "\x1B[94m@\033[0m";
                    goto escape;
                }
            }
            for (int k = 0; k < game::team_mates.size(); k++) {
                if (i == game::team_mates[k].y && j == game::team_mates[k].x) {
                    if (game::team_mates[k].alive)
                        std::cout << std::string("\x1b[1m\x1B[33m") + game::team_mates[k].skin + std::string("\033[0m");
                    else
                        std::cout << "\x1B[33m@\033[0m";
                    goto escape;
                }
            }
            for (int k = 0; k < game::bosses.size(); k++) {
                if (i == game::bosses[k].y && j == game::bosses[k].x) {
                    if (game::bosses[k].alive)
                        std::cout << std::string("\x1B[90m\x1b[1m") + game::bosses[k].skin + std::string("\033[0m");
                    else
                        std::cout << "\x1B[90m@\033[0m";
                    goto escape;
                }
            }
            for (int k = 0; k < game::bullets.size(); k++) {
                if (!game::bullets[k].active)
                    continue;
                if (i == game::bullets[k].y && j == game::bullets[k].x) {
                    if (game::bullets[k].fired == PLAYER)
                        std::cout << std::string("\x1B[95m\x1b[1m") + game::bullets[k].skin + std::string("\033[0m");
                    else
                        std::cout << std::string("\x1B[92m") + game::bullets[k].skin + std::string("\033[0m");
                    goto escape;
                }
            }
            if (game::obstacles_field[i][j]) {
                std::cout << std::string("\x1B[33m") + OBSTACLE_SKIN + std::string("\033[0m");
                goto escape;
            }
            if (game::mines_field[i][j]) {
                std::cout << std::string("\x1B[35m") + game::field[i][j] + std::string("\033[0m");
                goto escape;
            }
            std::cout << game::field[i][j];
            escape:
                continue;
        }

        if (i == 4)
            std::cout << "        \x1b[1mPOINTS\x1b[0m";
        else if (i == 5)
            std::cout << "        \x1b[1m" << player.points << "\x1b[0m";
        else if (i == 7)
            std::cout << "        \x1b[1mENEMIES\x1b[0m";
        else if (i == 8)
            std::cout << "        \x1b[1m" << sum << "\x1b[0m";
        else if (i == 10)
            std::cout << "        \x1b[1mAMMUNITIONS\x1b[0m";
        else if (i == 11)
            std::cout << "        \x1b[1m" << player.ammunitions << "\x1b[0m";
        else if (i == 13)
            std::cout << "        \x1b[1mKILLS\x1b[0m";
        else if (i == 14)
            std::cout << "        \x1b[1m" << player.kills << "\x1b[0m";

        std::cout << '\n';
    }
    std::cout << std::flush;
}

void moveAllBullets() {
    for (int i=0; i<game::bullets.size(); i++) {
        for (int j=0; j<game::bullets.size(); j++) {
            if (i == j) continue;

            if (game::bullets[i].x == game::bullets[j].x && game::bullets[i].y == game::bullets[j].y) {
                if (game::bullets[i].fired ==  game::bullets[j].fired)
                    continue; // Ignore when both are fired by same "team"
                game::bullets[i].active = false;
                game::bullets[j].active = false;
            }
        }
    }

    for (int i=0; i<game::bullets.size(); i++) {
        if (!game::bullets[i].active)
            game::bullets.erase(game::bullets.begin() + i);
    }

    for (auto& bullet: game::bullets) {
        if (bullet.active) // This control shouldn't be necessary but it's better to be safe than sorry
            bullet.move();
    }
}

void checkAllMines() {
    for (auto& mine: game::mines) {
        if (mine.triggered && !mine.just_triggered) {
            mine.detonate(player);
        } else if (mine.active) {
            for (auto& enemy: game::enemies) {
                if (enemy.x == mine.x && enemy.y == mine.y) {
                    mine.trigger();
                    break;
                }
            }
        }
    }
    for (auto& mine: game::mines)
        mine.just_triggered = false;
}

void updateField() {
    using namespace game; // This should be automatically unused after the scope dies
    bool died = false;
    emptyField();

    for (int i=0; i<enemies.size(); i++) {
        if (!enemies[i].alive)
            enemies.erase(enemies.begin() + i); // remove enemy
    }

    for (int i=0; i<team_mates.size(); i++) {
        if (!team_mates[i].alive)
            team_mates.erase(team_mates.begin() + i); // remove team mate
    }

    for (int i=0; i<bosses.size(); i++) {
        if (!bosses[i].alive)
            bosses.erase(bosses.begin() + i); // remove boss
    }

    for (int i=0; i<obstacles.size(); i++) {
        if (!obstacles[i].active)
            obstacles.erase(obstacles.begin() + i); // remove obstacle
    }

    for (int i=0; i<mines.size(); i++) {
        if (!mines[i].active)
            mines.erase(mines.begin() + i); // remove mine
    }

    for (auto& obstacle: game::obstacles) {
        if (obstacle.active) // This control shouldn't be necessary but it's better to be safe than sorry
            game::field[obstacle.y][obstacle.x] = obstacle.skin;
    }

    for (auto& mine: game::mines) {
        if (mine.active) // This control shouldn't be necessary but it's better to be safe than sorry
            game::field[mine.y][mine.x] = mine.skin;
    }

    for (int i=0; i<bullets.size(); i++) {
        for (int j=0; j<enemies.size(); j++) {
            if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y && bullets[i].fired == PLAYER) {
                player.ammunitions += enemies[j].value;
                enemies[j].value--;

                if (enemies[j].value == 0) {
                    std::cout << '\x07'; // Beep
                    enemies[j].alive = false;
                    player.kills++;
                }

                bullets[i].active = false;
                bullets.erase(bullets.begin() + i); // remove bullet from the vector
                break;
            }
        }
        for (int j=0; j<team_mates.size(); j++) {
            if (bullets[i].x == team_mates[j].x && bullets[i].y == team_mates[j].y && bullets[i].fired == ENEMY) {
                player.ammunitions += team_mates[j].value;
                team_mates[j].value--;

                if (team_mates[j].value == 0) {
                    std::cout << '\x07'; // Beep
                    team_mates[j].alive = false;
                    player.kills++;
                }

                bullets[i].active = false;
                bullets.erase(bullets.begin() + i); // remove bullet from the vector
                break;
            }
        }
        for (int j=0; j<bosses.size(); j++) {
            if (bullets[i].x == bosses[j].x && bullets[i].y == bosses[j].y && bullets[i].fired == PLAYER) {
                player.ammunitions += bosses[j].value;
                bosses[j].value--;

                if (bosses[j].value == 0) {
                    std::cout << '\x07'; // Beep
                    bosses[j].alive = false;
                    player.kills++;
                }

                bullets[i].active = false;
                bullets.erase(bullets.begin() + i); // remove bullet from the vector
                break;
            }
        }
        if (bullets[i].x == player.x && bullets[i].y == player.y && bullets[i].fired == ENEMY)
            died = true;
    }

    // Get skins for enemies
    for (auto& enemy: enemies) {
        enemy.getSkin();
    }

    if (died) {
        player.alive = false;
        printField();
        std::cout << "\x1b[31m\x1b[1mYou died!\033[0m" << std::endl;
        std::cout << "\x1b[?25h"; // Show cursor
        exit(0); // exit the game
    }
}


char getCharOrArrow() {
    char c = getch();
    if (c == KEY_ESC) { // Skip the 224 prefix
        switch (getch()) {
            case AFTER_KEY_UP:
                return BEFORE_KEY_UP;
            case AFTER_KEY_DOWN:
                return BEFORE_KEY_DOWN;
            case AFTER_KEY_LEFT:
                return BEFORE_KEY_LEFT;
            case AFTER_KEY_RIGHT:
                return BEFORE_KEY_RIGHT;
        }
    }
    return c; // This one could be in an else branch but it triggers g++ warnings
}


void mainloop() {
    // Configure the game
    CLS;
    std::cout << "Do you want to configure the game? (y/n)\n> ";
    char c = getch();
    CLS;
    switch (c) {
        case 'y': case 'Y':
            std::cout << "Do you want to load the settings? (y/n)\n> ";
            c = getch();
            CLS;
            switch (c) {
                case 'y': case 'Y': {
                    auto settings = loadSettings();
                    game::frame_duration = std::get<0>(settings);
                    game::starting_enemies = std::get<1>(settings);
                    game::starting_ammunitions = std::get<2>(settings);
                    std::cout << "Settings loaded!\n";
                    getch();
                    CLS;

                    std::cout << "AI difficulty [unsigned float] (0->1): ";
                    while (!(std::cin >> game::AI)) {
                        std::cout << "Invalid input. Try again.\n";
                        std::cout << "AI difficulty [unsigned float]: ";
                        std::cin.clear();
                        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    }
                    if (game::AI > 1) {
                        std::cout << "AI difficulty too high. Setting to 1.\n";
                        game::AI = 1;
                    } else if (game::AI < 0) {
                        std::cout << "AI difficulty too low. Setting to 0.\n";
                        game::AI = 0;
                    }
                    game::AI = closest(AI_DIFFICULTIES, 5, game::AI);
                    getch();
                    CLS;
                }
                    break;
                default:
                    game::configure(false);
                    break;
            }
            break;
        default:
            game::configure(true);
            break;
    }
    srand(time(NULL));

    game::random::addTeammate(rand()%48+1, rand()%18+1);
    for (int i=0; i<game::starting_enemies; i++)
        game::enemies.push_back(Enemy(rand()%48+1, rand()%18+1, SOUTH));
    player.ammunitions = game::starting_ammunitions;

    memset(game::obstacles_field, 0, sizeof(game::obstacles_field)); // Clear the obstacle's field
    memset(game::mines_field, 0, sizeof(game::mines_field)); // Clear the mine's field

    srand(time(NULL));
    std::ios_base::sync_with_stdio(false);
    game::emptyField();
    printField();
    std::cout << "\x1b[?25l"; // Hide cursor


    // Random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution turning_distribution(PROBABILITY_OF_ENEMY_TURNING);
    std::bernoulli_distribution shooting_distribution(PROBABILITY_OF_ENEMY_SHOOTING);
    std::bernoulli_distribution boss_shooting_distribution(PROBABILITY_OF_BOSS_SHOOTING);
    std::bernoulli_distribution moving_distribution(PROBABILITY_OF_ENEMY_MOVING);
    std::bernoulli_distribution appearing_distribution(PROBABILITY_OF_ENEMY_APPEARING);
    std::bernoulli_distribution team_mate_appearing_distribution(PROBABILITY_OF_TEAMMATE_APPEARING);
    std::bernoulli_distribution intelligence_distribution(game::AI);
    std::bernoulli_distribution obstacle_distribution(PROBABILITY_OF_OBSTACLE);
    std::bernoulli_distribution boss_distribution(PROBABILITY_OF_BOSS_APPEARING);

    char choice;
    while (choice != 'q') {
        auto input = std::async(std::launch::async, getCharOrArrow);
        while (input.wait_for(std::chrono::milliseconds(game::frame_duration)) != std::future_status::ready) {
            if (game::status == PAUSED)
                continue;
            
            moveAllBullets();
            checkAllMines();
            updateField();
            printField();

            for (auto& teammate: game::team_mates) {
                if (!teammate.alive)
                    continue;
                if (turning_distribution(gen)) {
                    if (!game::bosses.empty())
                        teammate.turn(true, game::bosses[0]);
                    else
                        teammate.direction = game::random::direction();
                } else if (shooting_distribution(gen)) {
                    teammate.fireBullet();
                } else if (moving_distribution(gen)) {
                    teammate.move();
                }
            }
            for (auto& enemy: game::enemies) {
                if (!enemy.alive)
                    continue;
                if (turning_distribution(gen)) {
                    enemy.turn(intelligence_distribution(gen), player);
                } else if (shooting_distribution(gen)) {
                    enemy.fireBullet();
                } else if (moving_distribution(gen)) {
                    enemy.move();
                }
            }
            for (auto& boss: game::bosses) {
                if (!boss.alive)
                    continue;
                if (boss_shooting_distribution(gen))
                    boss.fireBullet();
                if (turning_distribution(gen))
                    boss.turn(true, player);
                if (moving_distribution(gen))
                    boss.move();
            }
            if (appearing_distribution(gen))
                game::random::addEnemy(rand()%48+1, rand()%18+1);
            if (team_mate_appearing_distribution(gen))
                game::random::addTeammate(rand()%48+1, rand()%18+1);
            if (obstacle_distribution(gen))
                game::random::addObstacle(rand()%48+1, rand()%18+1);
            if (boss_distribution(gen)) {
                if (game::bosses.size() < 1)
                    game::random::addBoss(rand()%48+1, rand()%18+1);
            }

            if (player.auto_fire)
                player.fireBullet();
            player.points++;
        }

        choice = input.get();
        switch (choice) {
            case 'q': case 'Q':
                std::cout << "\x1b[?25h"; // Show cursor
                return;
            case 'p': case 'P':
                game::status = PAUSED;
                break;
            case 'r': case 'R':
                game::status = PLAYING;
                break;
            case 'w': case 'W': case 'a': case 'A': case 's': case 'S': case 'd': case 'D':
                if (game::status == PLAYING)
                    player.movePlayer(choice);
                break;
            case BEFORE_KEY_UP: case BEFORE_KEY_DOWN: case BEFORE_KEY_LEFT: case BEFORE_KEY_RIGHT:
                if (game::status == PLAYING) {
                    player.changeFireDirection(choice);
                    if (!player.mine && !player.build && !player.auto_fire)
                        player.fireBullet();
                    if (player.build)
                        player.buildObstacle();
                    if (player.mine)
                        player.placeMine();
                }
                break;
            case 'x': case 'X':
                if (!player.auto_fire) {
                    if (player.mine)
                        player.mine = false;
                    if (player.build)
                        player.build = false;
                }
                player.auto_fire = !player.auto_fire;
                break;
            case '+':
                player.cross_fire = !player.cross_fire;
                break;
            case 'b': case 'B':
                if (!player.build) {
                    if (player.mine)
                        player.mine = false;
                    if (player.auto_fire)
                        player.auto_fire = false;
                }
                player.build = !player.build;
                break;
            case 'm': case 'M':
                if (!player.mine) {
                    if (player.build)
                        player.build = false;
                    if (player.auto_fire)
                        player.auto_fire = false;
                }
                player.mine = !player.mine;
                break;
            case '1': case '2': case '3':
                player.changeBulletSpeed(choice);
                break;
        }
    }
}
