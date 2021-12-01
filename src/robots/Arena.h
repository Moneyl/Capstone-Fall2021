#pragma once
#include "Typedefs.h"
#include "Robot.h"

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

    const static inline f32 Speed = 5.0f;
    const static inline f32 Length = 20.0f; //Length of the trail line drawn for each bullet
};

//Owns and updates all objects in the arena (e.g. robots, mines, bullets, etc)
class Arena
{
public:
    void Update(f32 deltaTime);
    void Draw(Renderer* renderer);
    void Reset(); //Default arena
    void CreateBullet(const Vec2<f32>& position, const Vec2<f32>& direction, u64 creator, f32 damage = 1.0f);

    Vec2<f32> Position = { 400.0f, 50.0f };
    Vec2<f32> Size = { 1000.0f, 1000.0f };
    u32 CyclesPerSecond = 20; //# of VM cycles to run each second
    bool RobotAutoReloadEnabled = true; //Auto recompile robot program when source file is edited
    std::vector<Robot> Robots = {};
    std::vector<Bullet> Bullets = {};

private:
    f32 _cycleAccumulator = 0.0f; //Accumulates time to execute VM cycles
};