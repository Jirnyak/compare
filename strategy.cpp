#define SDL_MAIN_HANDLED

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <unistd.h>
#include <string>
#include <tuple>
#include <math.h>  
using namespace std;

#define WORLD_WIDTH 1000
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 750
#define SIDE_MENU (WINDOW_WIDTH-WINDOW_HEIGHT)/2
#define MAXA 10000
#define DECAY 10

#define RELIEF_TEST 1000

#define PERCENT 100

#define TICK_INTERVAL 30

#define MAX_SPEED 1000

#define ISLANDS 100

/* */

/*

cd /Users/jirnyak/Mirror/DND

makefile 

game:
    CXX=g++-12 CC=gcc-12
    g++-12 strategy.cpp -o strategy -I /Library/Frameworks/SDL2.framework/Headers -F /Library/Frameworks -framework SDL2 -I /Library/Frameworks/SDL2_image.framework/Headers -F /Library/Frameworks -framework SDL2_image -I /Library/Frameworks/SDL2_ttf.framework/Headers -F /Library/Frameworks -framework SDL2_ttf -fopenmp -std=c++11 

./strategy

x86_64-w64-mingw32-objdump -p dukalop_win.exe | grep 'DLL Name:' | sed -e "s/\t*DLL Name: //g"

lldb ./strategy

lldb

file strategy

run

(opt: breakpoint set --file strategy.cpp --line N)


*/

enum type 
{
    player,
    mob,
    plant,
    tree,
    bullet,
    knight
};

enum zemlya
{
    water,
    dirt,
    grass,
    sand,
    fire
};

enum control
{
    erafia,
    erafia_base,
    krigan,
    krigan_base
};

class object
{
    public:

        int x;  
        int y; 

        double x_loc;
        double y_loc;

        double v_x;
        double v_y;

        bool dead;

        double hp;

        double speed;

        int aim;

        short int type;

        short int ally;

        object(int x, int y, enum type typ) 
        { 
            this->x = x;
            this->y = y;

            this->x_loc = x + 0.5;
            this->y_loc = y + 0.5;

            this->v_x = 0.0;
            this->v_y = 0.0;

            this->dead = false;

            this->aim = -1;

            switch(typ)
            {
                case player:
                    this->type = player;
                    this->hp = 100;
                    this->speed = 0.1;
                    break;
                case mob:
                    this->type = mob;
                    this->hp = 1000;
                    this->speed = 0.1;
                    break;
                case plant:
                    this->type = plant;
                    this->hp = 100;
                    this->speed = 0;
                    break;
                case tree:
                    this->type = tree;
                    this->hp = 1000;
                    this->speed = 0;
                    break;
                case bullet:
                    this->type = bullet;
                    this->hp = 100;
                    this->speed = 1;
                    break;
                case knight:
                    this->type = knight;
                    this->hp = 100;
                    this->speed = 0.1;
                    break;
            }

        }
            int cell_number()
            {
                return x*WORLD_WIDTH + y;
            }
};

class projectile: public object
{
public:
    tuple <double, double> velocity;
        projectile(int x, int y, enum type typ,tuple <double, double> velocity, short int ally):object(x,y,typ)
        {
            this->velocity = velocity;
            this->v_x = get<0>(velocity);
            this->v_y = get<1>(velocity);
            this->ally = ally;
        }
};

class cell 
{
    private: 
        int x;  
        int y;   
        int number;
        cell *address;
    public:     
        cell *sosed_up;   
        cell *sosed_left; 
        cell *sosed_down; 
        cell *sosed_right; 
        cell *sosed_upleft;
        cell *sosed_upright;
        cell *sosed_downleft;
        cell *sosed_downright; 

        vector<object> inside;

        int R;
        int G;
        int B;

        bool dead;

        bool terra;

        bool sand;

        int direction;

        bool water;

        bool dirt;

        bool fire;

        int relief;

        short int control;

        cell(int x, int y) 
        { 
            this->x = x;
            this->y = y;

            this->number = x*WORLD_WIDTH + y;

            this->terra = 0;

            this->dead = 0;

            this->sand = 0;

            this->water = 0;

            this->dirt = 0;

            this->fire = 0;

            vector<object> inside;
        }
        void gen_address()
        {
            this->address = this;
        }
        void info()
        {
            cout << number << " " << x << " " << y << " " << sosed_up->x << sosed_up->y << "\n";
        }
        int get_x()
        {
            return x;
        }
        int get_y()
        {
            return y;
        }
        cell* get_address()
        {
            return address;
        }
        void up(cell *c)
        {
            sosed_up = c;
        }
        void down(cell *c)
        {
            sosed_down = c;
        }
        void left(cell *c)
        {
            sosed_left = c;
        }
        void right(cell *c)
        {
            sosed_right = c;
        }
        void upright(cell *c)
        {
            sosed_upright = c;
        }
        void upleft(cell *c)
        {
            sosed_upleft = c;
        }
        void downright(cell *c)
        {
            sosed_downright = c;
        }
        void downleft(cell *c)
        {
            sosed_downleft = c;
        }
        int get_n() const
        {
            return number;
        }
        cell* side(int d) 
        {
            if (d == 0)
                return sosed_up;
            if (d == 1)
                return sosed_upleft;
            if (d == 2)
                return sosed_left;
            if (d == 3)
                return sosed_downleft;
            if (d == 4)
                return sosed_down;
            if (d == 5)
                return sosed_downright;
            if (d == 6)
                return sosed_right;
            if (d == 7)
                return sosed_upright;
            else
                return address;
        }
        cell* side_spiral(int d) const
        {
            if (d == 0)
                return sosed_up;
            if (d == 1)
                return sosed_left;
            if (d == 2)
                return sosed_down;
            if (d == 3)
                return sosed_right;
            else
                return address;
        }
};

using rng_t = std::mt19937;

std::random_device dev;

std::mt19937 rng(dev());

uint32_t randomer(rng_t& rng, uint32_t range) 
{
    range += 1;
    uint32_t x = rng();
    uint64_t m = uint64_t(x) * uint64_t(range);
    uint32_t l = uint32_t(m);
    if (l < range) {
        uint32_t t = -range;
        if (t >= range) {
            t -= range;
            if (t >= range) 
                t %= range;
        }
        while (l < t) {
            x = rng();
            m = uint64_t(x) * uint64_t(range);
            l = uint32_t(m);
        }
    }
    return m >> 32;
}

int tor_cord(int x)
{
    if (x < 0)
    {
        x = WORLD_WIDTH + x%WORLD_WIDTH;
    }
    else if (x >= WORLD_WIDTH)
    {
        x = x%WORLD_WIDTH;
    }
    return x;
}

int tile_spiral(int d)
{
            if (d == 0)
                return -1;
            if (d == 1)
                return -1;
            if (d == 2)
                return +1;
            if (d == 3)
                return +1;
            else
                return 0;
}
/*
int spiral_aim(const vector<cell>& world, int size, int start, int target) 
{
    int x = start % size;
    int y = start / size;
    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};
    int d = 0;
    int i = 0;
    int r = 1;
    for (i = 0; i < size * size; i++) 
    {
        if (!world[y * size + x].inside.empty() && world[y * size + x].inside[0].type == target)
        {
            return y * size + x;
        }
        x += dx[d];
        y += dy[d];
        if (x < r || x >= size - r || y < r || y >= size - r) 
        {
            d = (d + 1) % 4;
            if (y >= r && y < size - r)
            {
                r++;
            }
        }
    }
    return -1;
}
*/


int spiral_aim(const vector<cell>& world, int size, int start, int goal) 
{
    const cell* pos = &world[start];
    bool stop = 0;
    int use = 0;
    int k = 0;
    int aim = -1;
    for (int i = 0; i < size; i++)
    {
        for (int a = 0; a < use; a++)   
        {
            pos = pos->side_spiral(k%4);
            if (!pos->inside.empty())
            {
                if (pos->inside[0].type == goal)
                {
                    aim = pos->get_n();
                    stop = true;
                }
            }
        } 
        if (stop == true)
            break;        
        k += 1;
        for (int b = 0; b < use; b++)   
        {
            pos = pos->side_spiral(k%4);
            if (!pos->inside.empty())
            {
                if (pos->inside[0].type == goal)
                {
                        aim = pos->get_n();
                        stop = true;
                }
            }
        }  
        if (stop == true)
            break;    
        k += 1;
        use += 1;
    }
    return aim;
}


tuple<double,double> beam(int x1, int y1, int x2, int y2, double c)
{
    double k;
    double dx;
    double sin = y1-y2;
    double cos = x1-x2;
    if (abs(sin) >= abs(cos))
    {
        k = (sin)/(cos);
        if (x1 < x2)
        {
            dx = c/sqrt(k*k+1);
            return {dx, k*dx};
        }
        else
        {
            dx = -c/sqrt(k*k+1);
            return {dx, k*dx};
        }
    }
    else 
    {
        k = (cos)/(sin);
        if (y1 < y2)
        {
            dx = c/sqrt(k*k+1);
            return {k*dx, dx};
        }
        else
        {
            dx = -c/sqrt(k*k+1);
            return {k*dx, dx};
        }    
    }
}   

vector<cell> read_map(string map, vector<cell> world)
{
    string line;
    ifstream file(map);
    if (!file.is_open()) {
        cout << "Error: could not open file " << map << endl;
        exit(1);
    }

    int world_n = 0, RGB_n = 0;
    while (getline(file, line))
    {
        switch (RGB_n)
        {
        case 0:
            world[world_n].R = stoi(line);
            RGB_n++;
            break;
        case 1:
            world[world_n].G = stoi(line);
            RGB_n++;
            break;
        case 2:
            world[world_n].B = stoi(line);
            RGB_n = 0;
            world_n++;
            break;
        }
    }

    file.close();
    return world;
}

int main(int argc, char **argv) //int argc, char **argv
{

    int TILE_SIZE = 10;

    int tiles_in_window = WINDOW_HEIGHT/TILE_SIZE;

    int center_tile = tiles_in_window/2;

    vector<cell> world;
    vector<cell>::iterator it;

    vector<object> objects_buff;
    vector<object>::iterator it_obj;
    vector<object>::iterator it_inside;

    for (int i=0; i<WORLD_WIDTH; ++i)
    {
        for (int j=0; j<WORLD_WIDTH; ++j)
        {       
            world.push_back(cell(i,j));
        }
    }

    for (it = world.begin(); it != world.end(); ++it)
    {
        it->gen_address();
    }

    int delilo = WORLD_WIDTH*WORLD_WIDTH;

    for (it = world.begin(); it != world.end(); ++it)
    {
        vector<cell>::iterator it1;
        it->up(&world[tor_cord(it->get_x())*WORLD_WIDTH + tor_cord(it->get_y()-1)]);
        it->down(&world[tor_cord(it->get_x())*WORLD_WIDTH + tor_cord(it->get_y()+1)]);
        it->left(&world[tor_cord(it->get_x()-1)*WORLD_WIDTH + tor_cord(it->get_y())]);
        it->right(&world[tor_cord(it->get_x()+1)*WORLD_WIDTH + tor_cord(it->get_y())]);
        it->upleft(&world[tor_cord(it->get_x()-1)*WORLD_WIDTH + tor_cord(it->get_y()-1)]);
        it->upright(&world[tor_cord(it->get_x()+1)*WORLD_WIDTH + tor_cord(it->get_y()-1)]);
        it->downleft(&world[tor_cord(it->get_x()-1)*WORLD_WIDTH + tor_cord(it->get_y()+1)]);
        it->downright(&world[tor_cord(it->get_x()+1)*WORLD_WIDTH + tor_cord(it->get_y()+1)]);
    }

    int LIMIT = 300000+randomer(rng, 100000);
    int gena = 0;
    int perbor = 0;
    int drop;
    int zapas = 0;

    int krigan_score = 0;
    int erafia_score = 0;

    world = read_map("politik.txt", world);

    for (it = world.begin(); it != world.end(); ++it)
    {
        if (it->R > it->G)
        {
            it->control = krigan;
            krigan_score += 1;
        }
        else if (it->R < it->G)
        {
            it->control = erafia;
            erafia_score += 1;
        }
        else if (it->R < 100 and it->G < 100 and it->B < 100)
        {
            it->control = erafia_base;
            erafia_score += 100;
        }
        else if (it->R > 200 and it->G > 200 and it->B > 200)
        {
            it->control = krigan_base;
            krigan_score += 100;
        }
        else
        {
            it->control = krigan;
            krigan_score += 1;
        }
    }

    world = read_map("enrot.txt", world); 

    #pragma omp parallel
    #pragma omp for
    for (it = world.begin(); it != world.end(); ++it)
    {
        if (it->B > 90 and it->R < 50 and it->G < 100)
        {
            it->terra = 0;
            it->sand = 0;
            it->water = 1;
            it->dirt = 0;
        }
        else if (it->B < 100 and it->R > 100 and it->G > 100)
        {
            it->sand = 1;
            it->terra = 0;
            it->water = 0;
            it->dirt = 0;
        }
        else if (it->B < 100 and it->R < 100 and it->G > 50)
        {
            it->sand = 0;
            it->terra = 1;
            it->water = 0;
            it->dirt = 0;
        }
        else
            {
            it->terra = 0;
            it->dirt = 1;
            it->sand = 0;
            it->water = 0;
            }
    } 

    #pragma omp parallel
    #pragma omp for
    for (it = world.begin(); it != world.end(); ++it)
    {
        if (it->water != 1)
        {
            for (int i = 0; i < 8; i++) 
            {
                if (it->side(i)->water == 1)
                {
                    it->sand = 1;
                    break;
                }
            }
        }
    }

    perbor = 0;
    gena = 0;
    drop = 0;

    while (gena < 1000)
    {
        drop  = randomer(rng, WORLD_WIDTH*WORLD_WIDTH);
        if (world[drop].inside.empty() and world[drop].terra == 1)
        {
            world[drop].inside.push_back(object(world[drop].get_x(),world[drop].get_y(), plant));
            gena += 1;
        }
    }

    gena = 0;

    while (gena < krigan_score/100)
    {
        drop  = randomer(rng, WORLD_WIDTH*WORLD_WIDTH);
        if (world[drop].inside.empty() and world[drop].water == 0 and world[drop].control == krigan)
        {
            world[drop].inside.push_back(object(world[drop].get_x(),world[drop].get_y(), mob));
            gena += 1;
        }
    }

    gena = 0;

    while (gena < erafia_score/100)
    {
        drop  = randomer(rng, WORLD_WIDTH*WORLD_WIDTH);
        if (world[drop].inside.empty() and world[drop].water == 0 and world[drop].control == erafia)
        {
            world[drop].inside.push_back(object(world[drop].get_x(),world[drop].get_y(), knight));
            gena += 1;
        }
    }

    gena = 0;

    while (gena < 10000)
    {
        drop  = randomer(rng, WORLD_WIDTH*WORLD_WIDTH);
        if (world[drop].inside.empty() and world[drop].terra == 1)
        {
            world[drop].inside.push_back(object(world[drop].get_x(),world[drop].get_y(), tree));
            gena += 1;
        }
    }

    int player_n;

    while (true)
    {
        drop  = randomer(rng, WORLD_WIDTH*WORLD_WIDTH);
        if (world[drop].inside.empty() and world[drop].terra == 1)
        {
            player_n = drop;
            break;
        }
    }

    world[player_n].inside.push_back(object(world[player_n].get_x(),world[player_n].get_y(), player));

    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    //SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    int curs_x;
    int curs_y;

    double koef;

    bool stop;

    bool buttonPressed = false;
    bool map_mod = false;
    bool politik_mod = false;
    bool paused = false;

    bool armag_mod = false;
    int light_x;
    int light_y;
    double aim_x;
    double aim_y;

    int key;

    SDL_Rect tile;
    tile.w = TILE_SIZE;
    tile.h = TILE_SIZE;

    SDL_Rect small_tile;
    small_tile.w = TILE_SIZE/10;
    small_tile.h = TILE_SIZE/10;

    SDL_Texture *player_texture[6];
    player_texture[0] = IMG_LoadTexture(renderer, "Untitled_Artwork-1.png");
    player_texture[1] = IMG_LoadTexture(renderer, "Untitled_Artwork-2.png");
    player_texture[2] = IMG_LoadTexture(renderer, "Untitled_Artwork-3.png");
    player_texture[3] = IMG_LoadTexture(renderer, "Untitled_Artwork-4.png");
    player_texture[4] = IMG_LoadTexture(renderer, "Untitled_Artwork-5.png");
    player_texture[5] = IMG_LoadTexture(renderer, "Untitled_Artwork-6.png");

    SDL_Rect long_tile;
    long_tile.w = TILE_SIZE;
    long_tile.h = TILE_SIZE;

    SDL_Rect minimap_dot;
    minimap_dot.w = 10;
    minimap_dot.h = 10;

    SDL_Rect pixel_tile;
    pixel_tile.w = 1;
    pixel_tile.h = 1;

    int player_anim = 0;

    int player_speed = 1000;

    //TEXTURES
    //SDL_Texture *_texture = IMG_LoadTexture(renderer, ".png");

    SDL_Texture *plant_texture = IMG_LoadTexture(renderer, "plant.png");
    SDL_Texture *tree_texture = IMG_LoadTexture(renderer, "palm.png");
    SDL_Texture *cow_texture = IMG_LoadTexture(renderer, "cow.png");
    SDL_Texture *water_texture = IMG_LoadTexture(renderer, "water.png");
    SDL_Texture *grass_texture = IMG_LoadTexture(renderer, "grass.png");
    SDL_Texture *sand_texture = IMG_LoadTexture(renderer, "sand.png");
    SDL_Texture *dirt_texture = IMG_LoadTexture(renderer, "dirt.png");
    SDL_Texture *armag_texture = IMG_LoadTexture(renderer, "armag.png");
    SDL_Texture *crater_texture = IMG_LoadTexture(renderer, "crater.png");
    SDL_Texture *red_zone = IMG_LoadTexture(renderer, "red_zone.png");
    SDL_Texture *girl_texture = IMG_LoadTexture(renderer, "aim.png");
    SDL_Texture *red_zone_texture = IMG_LoadTexture(renderer, "zone1.png");
    SDL_Texture *green_zone_texture = IMG_LoadTexture(renderer, "zone0.png");
    SDL_Texture *footman_texture = IMG_LoadTexture(renderer, "footman.png");

    SDL_Texture *panel_on_left = IMG_LoadTexture(renderer, "panel_on_left.png");
    SDL_Texture *panel_on_right = IMG_LoadTexture(renderer, "panel_on_right.png");
    SDL_Texture *panel_off_left = IMG_LoadTexture(renderer, "panel_off_left.png");
    SDL_Texture *panel_off_right = IMG_LoadTexture(renderer, "panel_off_right.png");

    SDL_Rect left_panel;
    left_panel.w = SIDE_MENU;
    left_panel.h = WINDOW_HEIGHT+1;
    left_panel.x = 0;
    left_panel.y = 0;

    SDL_Rect right_panel;
    right_panel.w = SIDE_MENU;
    right_panel.h = WINDOW_HEIGHT+1;
    right_panel.x = WINDOW_WIDTH - SIDE_MENU;
    right_panel.y = 0;

    SDL_Rect minimap;
    minimap.w = WORLD_WIDTH;
    minimap.h = WORLD_WIDTH;

    string text;

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("Roboto-Black.ttf", 20);

    SDL_Surface *info1 = TTF_RenderText_Solid(font, "planet: Enroth", {0,0,0});
    SDL_Texture *info1_text = SDL_CreateTextureFromSurface(renderer, info1);

    SDL_Surface *info2 = TTF_RenderText_Solid(font, "active spell: Fire cone", {0,0,0});
    SDL_Texture *info2_text = SDL_CreateTextureFromSurface(renderer, info2);

    SDL_Surface *info3;
    SDL_Texture *info3_text;

    SDL_Surface *info_suck = TTF_RenderText_Solid(font, "sucking on", {255,0,0});
    SDL_Texture *info_suck_text = SDL_CreateTextureFromSurface(renderer, info_suck);

    SDL_Surface *info_suck_off = TTF_RenderText_Solid(font, "sucking off", {0,0,0});
    SDL_Texture *info_suck_off_text = SDL_CreateTextureFromSurface(renderer, info_suck_off);

    SDL_Rect info1_rect;
    info1_rect.x = WORLD_WIDTH/2;  //controls the rect's x coordinate 
    info1_rect.y = 0; // controls the rect's y coordinte
    info1_rect.w = 400; // controls the width of the rect
    info1_rect.h = 100; // controls the height of the rect

    SDL_Rect info2_rect;
    info2_rect.x = WORLD_WIDTH/2;  //controls the rect's x coordinate 
    info2_rect.y = 100; // controls the rect's y coordinte
    info2_rect.w = 400; // controls the width of the rect
    info2_rect.h = 100; // controls the height of the rect

    SDL_Rect info3_rect;
    info3_rect.x = WORLD_WIDTH/2;  //controls the rect's x coordinate 
    info3_rect.y = 200; // controls the rect's y coordinte
    info3_rect.w = 400; // controls the width of the rect
    info3_rect.h = 100; // controls the height of the rect

    SDL_Rect info_suck_rect;
    info_suck_rect.x = WORLD_WIDTH;  //controls the rect's x coordinate 
    info_suck_rect.y = 0; // controls the rect's y coordinte
    info_suck_rect.w = 200; // controls the width of the rect
    info_suck_rect.h = 100; // controls the height of the rect

    SDL_Surface *mocha_info;
    SDL_Texture *mocha_info_text;
    std::string mocha;
    std::string govno;

    SDL_Rect info_mocha_rect;
    info_mocha_rect.x = WORLD_WIDTH;  //controls the rect's x coordinate 
    info_mocha_rect.y = 100; // controls the rect's y coordinte
    info_mocha_rect.w = 200; // controls the width of the rect
    info_mocha_rect.h = 100; // controls the height of the rect

    SDL_Rect info_govno_rect;
    info_govno_rect.x = WORLD_WIDTH;  //controls the rect's x coordinate 
    info_govno_rect.y = 200; // controls the rect's y coordinte
    info_govno_rect.w = 200; // controls the width of the rect
    info_govno_rect.h = 100; // controls the height of the rect

    mocha = std::to_string(42);
    govno = std::to_string(42);

    bool occupied = false;

    bool quit = 0;

    cell* pos;

    int use;
    int k;

    while (quit == false) 
    {
        //USER INPUT CHECK

        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                SDL_GetMouseState(&curs_x, &curs_y);
                for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                    {
                        if (it_inside->type == player)
                        {
                            it_inside->y += (curs_y-WINDOW_HEIGHT/2)/TILE_SIZE;
                            it_inside->x += (curs_x-WINDOW_WIDTH/2)/TILE_SIZE;
                            it_inside->x = tor_cord(it_inside->x);
                            it_inside->y = tor_cord(it_inside->y);
                            player_n = it_inside->cell_number();
                            break;
                        }
                    }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_f:
                         armag_mod = not armag_mod;
                         break;
                    case SDLK_l:
                            SDL_GetMouseState(&light_x, &light_y);
                            for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                            {
                                if (it_inside->type == player)
                                {
                                    aim_y = it_inside->y + (light_y-WINDOW_HEIGHT/2)/TILE_SIZE;
                                    aim_x = it_inside->x + (light_x-WINDOW_WIDTH/2)/TILE_SIZE;
                                    aim_y = tor_cord(aim_y);
                                    aim_x = tor_cord(aim_x);
                                    objects_buff.push_back(projectile(it_inside->x, it_inside->y,bullet,beam(WINDOW_WIDTH/2.0, WINDOW_HEIGHT/2.0, light_x/1.0, light_y/1.0, 1), mob));
                                    break;
                                }
                            }
                        break;
                    case SDLK_m:
                        map_mod = not map_mod;
                        break;
                    case SDLK_UP:
                        for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                        {
                            if (it_inside->type == player)
                            {
                                it_inside->y -= 1;
                                it_inside->y = tor_cord(it_inside->y);
                                player_n = it_inside->cell_number();
                                break;
                            }
                        }
                        break;
                    case SDLK_DOWN:
                        for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                        {
                            if (it_inside->type == player)
                            {
                                it_inside->y += 1;
                                it_inside->y = tor_cord(it_inside->y);
                                player_n = it_inside->cell_number();
                                break;
                            }
                        }
                        break;
                    case SDLK_LEFT:
                        for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                        {
                            if (it_inside->type == player)
                            {
                                it_inside->x -= 1;
                                it_inside->x = tor_cord(it_inside->x);
                                player_n = it_inside->cell_number();
                                break;
                            }
                        }
                        break;
                    case SDLK_RIGHT:
                        for (it_inside = world[player_n].inside.begin(); it_inside != world[player_n].inside.end(); ++it_inside)
                        {
                            if (it_inside->type == player)
                            {
                                it_inside->x += 1;
                                it_inside->x = tor_cord(it_inside->x);
                                player_n = it_inside->cell_number();
                                break;
                            }
                        }
                        break;
                    case SDLK_EQUALS:
                        TILE_SIZE += 10;
                        if (TILE_SIZE > 50)
                            TILE_SIZE = 50;
                        tiles_in_window = WINDOW_HEIGHT/TILE_SIZE;
                        center_tile = tiles_in_window/2;
                        tile.w = TILE_SIZE;
                        tile.h = TILE_SIZE;
                        small_tile.w = TILE_SIZE/10;
                        small_tile.h = TILE_SIZE/10;
                        long_tile.w = TILE_SIZE;
                        long_tile.h = TILE_SIZE;
                        break;
                    case SDLK_MINUS:
                        TILE_SIZE -= 10;
                        if (TILE_SIZE < 10)
                            TILE_SIZE = 10;
                        tiles_in_window = WINDOW_HEIGHT/TILE_SIZE;
                        center_tile = tiles_in_window/2;
                        tile.w = TILE_SIZE;
                        tile.h = TILE_SIZE;
                        small_tile.w = TILE_SIZE/10;
                        small_tile.h = TILE_SIZE/10;
                        long_tile.w = TILE_SIZE;
                        long_tile.h = TILE_SIZE;
                        break;
                    case SDLK_SPACE:
                        paused = !paused;
                        break;
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
        }

            //PHYSICS

            if (paused == false)
            {
                for (it = world.begin(); it != world.end(); ++it)
                {
                    if (it->fire == 1)
                    {
                        //#pragma omp parallel for
                        for (it_inside = it->inside.begin(); it_inside != it->inside.end(); ++it_inside)
                        {
                            it_inside->dead = 1;
                        }
                        it->fire = 0;
                    }
                }

                for (it = world.begin(); it != world.end(); ++it)
                {
                    if (!it->inside.empty())
                    {
                        for (it_inside = it->inside.begin(); it_inside != it->inside.end(); ++it_inside)
                        {
                            switch (it_inside->type)
                            {
                                case mob:                        
                                    if (it_inside->aim == -1)
                                    {     
                                        it_inside->aim = spiral_aim(world, 10, it_inside->cell_number(), knight);
                                    }
                                    else if (it_inside->aim != -1)
                                    {
                                        objects_buff.push_back(projectile(it_inside->x, it_inside->y,bullet,beam(it_inside->x, it_inside->y, world[it_inside->aim].get_x(), world[it_inside->aim].get_y(), 1), knight));
                                        it_inside->aim = -1;
                                    }
                                    if (it_inside->aim == -1)
                                    {
                                        if (it->water == 1)
                                        {
                                            it_inside->v_x = 0;
                                            it_inside->v_y = 0;
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/100;
                                        }
                                        else if (it->control != krigan)
                                        {
                                            it_inside->v_x += it_inside->v_x;
                                            it_inside->v_y += it_inside->v_y;
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/10;
                                        }
                                        else
                                        {
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/10;
                                        }
                                    }
                                    if (abs(it_inside->v_x) > abs(it_inside->speed))
                                        it_inside->v_x = it_inside->v_x/abs(it_inside->v_x)*it_inside->speed;
                                    if (abs(it_inside->v_y) > abs(it_inside->speed))
                                        it_inside->v_y = it_inside->v_y/abs(it_inside->v_y)*it_inside->speed;
                                    it_inside->x_loc += it_inside->v_x;
                                    it_inside->y_loc += it_inside->v_y;
                                    it_inside->x = it_inside->x_loc;
                                    it_inside->y = it_inside->y_loc;
                                    it->control = krigan;
                                    break;

                                case knight:
                                    if (it_inside->aim == -1)
                                    {
                                        it_inside->aim = spiral_aim(world, 10, it_inside->cell_number(), mob);
                                    }
                                    else if (it_inside->aim != -1)
                                    {
                                        objects_buff.push_back(projectile(it_inside->x, it_inside->y,bullet,beam(it_inside->x, it_inside->y, world[it_inside->aim].get_x(), world[it_inside->aim].get_y(), 1), mob));
                                        it_inside->aim = -1;
                                    }
                                    if (it_inside->aim == -1)
                                    {
                                        if (it->water == 1)
                                        {
                                            it_inside->v_x = 0;
                                            it_inside->v_y = 0;
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/100;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/100;
                                        }
                                        else if (it->control != erafia)
                                        {
                                            it_inside->v_x += it_inside->v_x;
                                            it_inside->v_y += it_inside->v_y;
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/10;
                                        }
                                        else
                                        {
                                            it_inside->v_x += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_x -= it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y += it_inside->speed*randomer(rng,10)/10;
                                            it_inside->v_y -= it_inside->speed*randomer(rng,10)/10;
                                        }
                                    }
                                    if (abs(it_inside->v_x) > abs(it_inside->speed))
                                        it_inside->v_x = it_inside->v_x/abs(it_inside->v_x)*it_inside->speed;
                                    if (abs(it_inside->v_y) > abs(it_inside->speed))
                                        it_inside->v_y = it_inside->v_y/abs(it_inside->v_y)*it_inside->speed;
                                    it_inside->x_loc += it_inside->v_x;
                                    it_inside->y_loc += it_inside->v_y;
                                    it_inside->x = it_inside->x_loc;
                                    it_inside->y = it_inside->y_loc;
                                    it->control = erafia;
                                    break;

                                case bullet:
                                    for (it_obj = it->inside.begin(); it_obj != it->inside.end(); ++it_obj)
                                    {
                                        if (it_obj->type == it_inside->ally)
                                        {
                                            it_obj->dead = true;
                                            it_inside->dead = true;
                                            break;
                                        }
                                    }
                                    it_inside->x_loc += it_inside->v_x;
                                    it_inside->y_loc += it_inside->v_y;
                                    it_inside->x = it_inside->x_loc;
                                    it_inside->y = it_inside->y_loc;
                                    it_inside->hp -= 1;
                                    if (it_inside->hp<0)
                                        it_inside->dead = true;
                                    break;
                            }
                        } 
                    }
                }
            }
            
        //DRAW

        
        if (!paused)
        {
            SDL_RenderCopy(renderer, panel_on_left, NULL, &left_panel);
            SDL_RenderCopy(renderer, panel_on_right, NULL, &right_panel);
        } 
        else 
        {
            SDL_RenderCopy(renderer, panel_off_left, NULL, &left_panel);
            SDL_RenderCopy(renderer, panel_off_right, NULL, &right_panel);
        } 
        pos = &world[player_n];
        k = 0;
        use = 0;
        gena = 1000;
        drop = 0;
        SDL_SetRenderDrawColor(renderer, pos->R, pos->G, pos->B, 255);
        SDL_RenderFillRect(renderer, &tile);
        tile.x = center_tile*TILE_SIZE+SIDE_MENU;
        tile.y = center_tile*TILE_SIZE;
        long_tile.x = center_tile*TILE_SIZE+SIDE_MENU;
        long_tile.y = center_tile*TILE_SIZE;
        if (pos->terra == 1)
            SDL_RenderCopy(renderer, grass_texture, NULL, &tile);
        if (pos->sand == 1)
            SDL_RenderCopy(renderer, sand_texture, NULL, &tile);
        if (pos->water == 1)
            SDL_RenderCopy(renderer, water_texture, NULL, &tile);
        if (pos->dirt == 1)
            SDL_RenderCopy(renderer, dirt_texture, NULL, &tile);
        if (!pos->inside.empty() and pos->inside[0].type == plant)
                {
                    SDL_RenderCopy(renderer, plant_texture, NULL, &tile);
                } 
        if (!pos->inside.empty() and pos->inside[0].type == mob)
                {
                    SDL_RenderCopy(renderer, cow_texture, NULL, &tile);
                } 
        for (int i = 0; i <= tiles_in_window; i++)
        {
            for (int a = 0; a < use; a++)   
            {
                pos = pos->side_spiral(k%4);
                SDL_SetRenderDrawColor(renderer, pos->R, pos->G, pos->B, 255);
                if ((k%4)%2 != 0)
                    tile.x += tile_spiral(k%4)*TILE_SIZE;
                else if ((k%4)%2 == 0)
                    tile.y += tile_spiral(k%4)*TILE_SIZE;
                SDL_RenderFillRect(renderer, &tile);
                if (pos->terra == 1)
                    SDL_RenderCopy(renderer, grass_texture, NULL, &tile);
                if (pos->sand == 1)
                    SDL_RenderCopy(renderer, sand_texture, NULL, &tile);
                if (pos->water == 1)
                    SDL_RenderCopy(renderer, water_texture, NULL, &tile);
                if (pos->dirt == 1)
                    SDL_RenderCopy(renderer, dirt_texture, NULL, &tile);
                if (pos->dead == 1 and pos->water == 0)
                    SDL_RenderCopy(renderer, crater_texture, NULL, &tile);
                if (!pos->inside.empty() and pos->inside[0].type == plant)
                {
                    SDL_RenderCopy(renderer, plant_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == mob)
                {
                    SDL_RenderCopy(renderer, cow_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == knight)
                {
                    SDL_RenderCopy(renderer, footman_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == tree)
                {
                    SDL_RenderCopy(renderer, tree_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == bullet)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                } 
                if (armag_mod == true and gena > 0)
                {
                    SDL_RenderCopy(renderer, red_zone, NULL, &tile);
                    drop = randomer(rng, 20);
                    if (drop == 20)
                    {
                        SDL_RenderCopy(renderer, armag_texture, NULL, &tile);
                        pos->dead = 1;
                        pos->fire = 1;
                        gena -= 1;
                    }
                }
            }
            k += 1;
            for (int b = 0; b < use; b++)   
            {
                pos = pos->side_spiral(k%4); 
                SDL_SetRenderDrawColor(renderer, pos->R, pos->G, pos->B, 255);
                if ((k%4)%2 != 0)
                    tile.x += tile_spiral(k%4)*TILE_SIZE;
                else if ((k%4)%2 == 0)
                    tile.y += tile_spiral(k%4)*TILE_SIZE;
                SDL_RenderFillRect(renderer, &tile);
                if (pos->terra == 1)
                    SDL_RenderCopy(renderer, grass_texture, NULL, &tile);
                if (pos->sand == 1)
                    SDL_RenderCopy(renderer, sand_texture, NULL, &tile);
                if (pos->water == 1)
                    SDL_RenderCopy(renderer, water_texture, NULL, &tile);
                if (pos->dirt == 1)
                    SDL_RenderCopy(renderer, dirt_texture, NULL, &tile);
                if (pos->dead == 1 and pos->water == 0)
                    SDL_RenderCopy(renderer, crater_texture, NULL, &tile);
                if (!pos->inside.empty() and pos->inside[0].type == plant)
                {
                    SDL_RenderCopy(renderer, plant_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == mob)
                {
                    SDL_RenderCopy(renderer, cow_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == knight)
                {
                    SDL_RenderCopy(renderer, footman_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == tree)
                {
                    SDL_RenderCopy(renderer, tree_texture, NULL, &tile);
                } 
                if (!pos->inside.empty() and pos->inside[0].type == bullet)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 150, 255);
                    small_tile.x = tile.x + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    small_tile.y = tile.y + randomer(rng, TILE_SIZE) - randomer(rng, TILE_SIZE) + TILE_SIZE/20;
                    SDL_RenderFillRect(renderer, &small_tile);
                } 
                if (armag_mod == true and gena > 0)
                {
                    SDL_RenderCopy(renderer, red_zone, NULL, &tile);
                    drop = randomer(rng, 20);
                    if (drop == 20)
                    {
                        SDL_RenderCopy(renderer, armag_texture, NULL, &tile);
                        gena -= 1;
                        pos->dead = 1;
                        pos->fire = 1;
                    }
                }
            }
            k += 1;
            use += 1;
        }
        SDL_RenderCopy(renderer, red_zone, NULL, &long_tile);
        SDL_RenderCopy(renderer, girl_texture, NULL, &long_tile);

        //INTERFACE AND INFO

        if (map_mod == true)
        {
            while (map_mod == true)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderClear(renderer);
                if (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_KEYDOWN)
                    {
                        switch(event.key.keysym.sym)
                        {
                            case SDLK_m:
                                map_mod = false;
                                politik_mod = false;
                                break;
                            case SDLK_p:
                                politik_mod = not politik_mod;
                                break;
                            case SDLK_ESCAPE:
                                map_mod = false;
                                quit = true;
                                break;
                        }
                    }
                }
                for (it = world.begin(); it != world.end(); ++it)
                {
                    pixel_tile.x = it->get_x()/2;
                    pixel_tile.y = it->get_y()/2;
                    SDL_SetRenderDrawColor(renderer, it->R, it->G, it->B, 255); 
                    SDL_RenderDrawPoint(renderer, pixel_tile.x, pixel_tile.y);
                    if (politik_mod == true)
                    {
                        switch(it->control)
                        {
                            case krigan:
                                SDL_RenderCopy(renderer, red_zone_texture, NULL, &pixel_tile);
                                break;
                            case erafia:
                                SDL_RenderCopy(renderer, green_zone_texture, NULL, &pixel_tile);
                                break;
                        }
                    }
                }
                text = "x: " + to_string(world[player_n].get_x()) + " y: " + to_string(world[player_n].get_y());
                info3 = TTF_RenderText_Solid(font, text.c_str(), {0,0,0});
                info3_text = SDL_CreateTextureFromSurface(renderer, info3);
                SDL_RenderCopy(renderer, info1_text, NULL, &info1_rect);
                SDL_RenderCopy(renderer, info2_text, NULL, &info2_rect);
                SDL_RenderCopy(renderer, info3_text, NULL, &info3_rect);
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                minimap_dot.x = world[player_n].get_x()/2-5;
                minimap_dot.y = world[player_n].get_y()/2-5;
                SDL_RenderFillRect(renderer, &minimap_dot);

                SDL_RenderPresent(renderer);
            }
        }

        SDL_RenderPresent(renderer);

        //CLEANING AND REFILING BUFFERS

        for (it = world.begin(); it != world.end(); ++it)
        {
            if (!it->inside.empty())
            {
                for (it_inside = it->inside.begin(); it_inside != it->inside.end(); ++it_inside)
                {
                    if (it_inside->dead == false)
                    {
                        it_inside->x = tor_cord(it_inside->x);
                        it_inside->y = tor_cord(it_inside->y);
                        objects_buff.push_back(*it_inside);
                    }
                }
                it->inside.clear();
            }
        }

        for (it_obj = objects_buff.begin(); it_obj != objects_buff.end(); ++it_obj)
        {
            world[it_obj->cell_number()].inside.push_back(*it_obj);
        }

        objects_buff.clear();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
 
}