#include <conio.h>
#include <chrono>
#include <thread>
#include <future>
#include <time.h>

#include "variables.hpp"


namespace game {
    Player player; // player
};


void moveAllBullets() {
    for (auto& bullet: game::bullets) {
        if (bullet.active)
            bullet.move();
    }
}

void updateField() {
    using namespace game; // This should be automatically unused after the scope dies
    emptyField();

    for (int i=0; i<bullets.size(); i++) {
        for (int j=0; j<enemies.size(); j++) {
            if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y && bullets[i].fired == PLAYER) {
                enemies[j].value--;
                player.ammunitions += enemies[j].value;
                if (enemies[j].value == 0)
                    enemies.erase(enemies.begin() + j); // remove enemy

                bullets[i].active = false; // remove bullet from the vector
                i--;
                break;
            }
        }
        if (bullets[i].x == player.x && bullets[i].y == player.y && bullets[i].fired == ENEMY) {
            std::cout << "You died!\n";
            exit(0); // game over
        }
    }

    // Insert bullets into the field
    for (auto& bullet: bullets) {
        if (bullet.active)
            field[bullet.y][bullet.x] = direction_to_skin[bullet.direction];
    }

    // Insert enemies into the field
    for (auto& enemy: enemies)
        field[enemy.y][enemy.x] = enemy.getSkin();
    
    // Insert player into the field
    field[player.y][player.x] = PLAYER_SKIN;
}


void printField() {
    system("cls");
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++)
            std::cout << game::field[i][j];
        std::cout << std::endl;
    }
    std::cout << "Points: " << game::player.points << std::endl;
    std::cout << "Ammunitions: " << game::player.ammunitions << std::endl;
}


void mainloop() {
    srand(time(NULL));
    game::emptyField();
    printField();

    game::enemies.push_back(Enemy(rand()%18+1, rand()%18+1, SOUTH));
    game::enemies.push_back(Enemy(rand()%18+1, rand()%18+1, SOUTH));
    game::enemies.push_back(Enemy(rand()%18+1, rand()%18+1, SOUTH));
    game::enemies.push_back(Enemy(rand()%18+1, rand()%18+1, SOUTH));

    char choice;
    while (choice != 'q') {
        auto input = std::async(std::launch::async, getch);
        while (input.wait_for(std::chrono::milliseconds(FRAME_DURATION)) != std::future_status::ready) {
            if (game::status == PAUSED) // if the game is paused, do nothing
                continue; // just wait for the next char
            
            moveAllBullets(); // move all bullets
            updateField(); // update the field
            printField(); // print the field

            for (auto& enemy: game::enemies) {
                if (rand()%3 == 0) {
                    enemy.turn();
                    enemy.fireBullet();
                } else if (rand()%2 == 0) {
                    enemy.movePlayer(game::random::directionalChar());
                }
            }
            if (rand()%50 == 0)
                game::random::addEnemy(rand()%18+1, rand()%18+1);

            if (game::player.auto_fire)
                game::player.fireBullet();
            game::player.points++;
        }
        printField(); // print the field

        choice = input.get();
        switch (choice) {
            case 'q': case 'Q':
                return;
            case 'p': case 'P':
                game::status = PAUSED;
                break;
            case 'r': case 'R':
                game::status = PLAYING;
                break;
            case 'w': case 'W': case 'a': case 'A': case 's': case 'S': case 'd': case 'D':
                if (game::status == PLAYING)
                    game::player.movePlayer(choice);
                break;
            case 'f': case 'F':
                if (game::status == PLAYING && game::player.ammunitions > 0 && !game::player.auto_fire)
                    game::player.fireBullet();
                break;
            case 'x': case 'X':
                game::player.auto_fire = !game::player.auto_fire;
                break;
        }
    }
}