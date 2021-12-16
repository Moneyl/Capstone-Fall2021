#pragma once
#include "Typedefs.h"
#include "Robot.h"
#include "Math/Util.h"
#include <unordered_map>

class Renderer;

//Bullet fired by robot turrets. Robots take damage when hit by them
struct Bullet
{
    Bullet(const Vec2<f32>& position, const Vec2<f32>& direction, u64 creator, f32 damage)
        : Position(position), Direction(direction.Normalized()), Creator(creator), Damage(damage)  {}

    Vec2<f32> Position;
    Vec2<f32> Direction;
    u64 Creator; //Unique ID of the robot that fired it
    f32 Damage;
    bool Alive = true; //False signals the Arena to delete it

    const static inline f32 Speed = 5.0f;
    const static inline f32 Length = 20.0f; //Length of the trail line drawn for each bullet
};

//Mine dropped by robots. Robots take damage if they hit them
struct Mine
{
    Mine(const Vec2<f32>& position, u64 creator, f32 damage)
        : Position(position), Creator(creator), Damage(damage) {}

    Vec2<f32> Position;
    u64 Creator; //Unique ID of the robot that fired it
    f32 Damage;
    bool Alive = true; //False signals the Arena to delete it
    
    const static inline f32 ExplosionRadius = 10.0f;
    const static inline f32 Size = 5.0f;
};

enum class ArenaState
{
    Normal, //Just robots doing their thing. No score or stages.
    Tournament, //Robots fight NumStages times. The last robot alive in that stage gets a point
    TournamentStageDone,
    TournamentComplete, //Tournament complete. Gui should draw score & stats window.
};

//Owns and updates all objects in the arena (e.g. robots, mines, bullets, etc)
class Arena
{
public:
    Arena();
    void Update(f32 deltaTime);
    void Draw(Renderer* renderer);
    void Reset(const std::vector<std::string>& botsToAdd = {}); //Clear arena and add robots to it. If no robots are provided it will randomly pick a few.
    void CreateBullet(const Vec2<f32>& position, const Vec2<f32>& direction, u64 creator, f32 damage);
    void CreateMine(const Vec2<f32>& position, u64 creator, f32 damage);
    void DetonateMine(Mine& mine);
    //Get closest robot to a position. Can optionally exclude a single robot exclude robots outside a specific arc
    Robot* GetClosestRobot(const Vec2<f32>& position, Robot* exclude = nullptr, f32 angleMinRadians = 0.0f, f32 angleMaxRadians = 2 * PI);
    //Get robot by unique ID. Returns nullptr if it can't find the bot.
    Robot* GetRobotById(u64 id);

    Vec2<f32> Position = { 400.0f, 50.0f };
    Vec2<f32> Size = { 1000.0f, 1000.0f };
    u32 CyclesPerSecond = 50; //# of VM cycles to run each second
    bool RobotAutoReloadEnabled = true; //Auto recompile robot program when source file is edited
    std::vector<Robot*> Robots = {}; //Stored as pointers so VM port bindings can reference them and not risk invalidation if Robots resizes.
    std::vector<Bullet> Bullets = {};
    std::vector<Mine> Mines = {};
    u32 Stage = 0; //Tournament stage
    u32 NumStages = 5;
    ArenaState State = ArenaState::Normal;
    std::unordered_map<std::string, u32> Scores = {};
    std::string Winner;

    //Maximum amount of collision substeps per robot per frame to use pushing a bot back into the arena
    const static inline u64 MaxRobotCollisionSubsteps = 10;

private:
    //Robot list & rand() seed used last. Arena::Reset() uses these if not provided with a new list
    std::vector<std::string> _robotList = {};
    unsigned int _seed = 0;
};