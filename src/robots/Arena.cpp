#include "Arena.h"
#include "render/Renderer.h"
#include "math/Util.h"
#include "utility/Algorithms.h"
#include <algorithm>

void Arena::Update(f32 deltaTime)
{
    //Calculate how many cycles to execute this frame
    //Only whole cycles are executed. If there's only enough time for part of a cycle it'll be accumulated for next frame
    _cycleAccumulator += deltaTime; //Accumulate cycle time
    const f32 timeBetweenCycles = 1.0f / (f32)CyclesPerSecond;
    const u32 cyclesToExecute = truncf(_cycleAccumulator / timeBetweenCycles);
    const f32 cyclesDelta = cyclesToExecute * timeBetweenCycles;

    //Update robots
    if (CyclesPerSecond != 0) //If CyclesPerSecond == 0, cyclesDelta == NaN. Breaks _cycleAccumulator and subsequent logic
    {
        _cycleAccumulator -= cyclesDelta;
        for (Robot& robot : Robots)
        {
            if (robot.Health <= 0)
                continue;

            //Recompile source file if it was edited
            if (RobotAutoReloadEnabled)
                robot.TryReload();

            robot.Update(*this, cyclesDelta, cyclesToExecute);
        }
    }

    //Update bullets
    for (Bullet& bullet : Bullets)
    {
        bullet.Position += bullet.Direction * Bullet::Speed;
        if (!IsPositionInRect(bullet.Position, Position, Size))
            bullet.Alive = false;

        //Detect bullet-robot collisions
        auto anyHit = std::find_if(Robots.begin(), Robots.end(), [&](const Robot& robot)
            { return robot.ID() != bullet.Creator && robot.Collides(bullet.Position); });

        //Handle collision
        if (anyHit != Robots.end())
        {
            //Hit robot. Damage it and delete the bullet
            anyHit->Damage(bullet.Damage);
            bullet.Alive = false;
        }
    }

    //Update mines
    for (Mine& mine : Mines)
    {
        //Detect mine-robot collisions
        auto anyHit = std::find_if(Robots.begin(), Robots.end(), [&](const Robot& robot)
            { return robot.ID() != mine.Creator && robot.Collides(mine.Position); });

        //Handle collision
        if (anyHit != Robots.end())
        {
            DetonateMine(mine);
            mine.Alive = false;
        }
    }

    //Erase dead objects
    EraseIf(Robots, [](const Robot& robot) { return robot.Health <= 0; });
    EraseIf(Bullets, [](const Bullet& bullet) { return !bullet.Alive; });
    EraseIf(Mines, [](const Mine& mine) { return !mine.Alive; });
}

void Arena::Draw(Renderer* renderer)
{
    renderer->DrawRectangleFilled(Position, Size, { 64, 64, 64, 255 }); //Floor
    renderer->DrawRectangle(Position, Size, { 200, 0, 0, 255 }); //Border
    for (Robot& robot : Robots)
        robot.Draw(renderer);

    for (Bullet& bullet : Bullets)
        renderer->DrawLine(bullet.Position, bullet.Position + bullet.Direction * Bullet::Length, { 255, 0, 0, 255 });

    for (Mine& mine : Mines)
        renderer->DrawRectangleFilledCentered(mine.Position, { Mine::Size, Mine::Size }, { 255, 0, 0, 255 });
}

void Arena::Reset()
{
    Robots.clear();

    //Create robots
    Vec2<i32> arenaCenter = Position + (Size / 2);
    Robot& robot = Robots.emplace_back();
    robot.LoadProgramFromSource(BuildConfig::AssetFolderPath + "tests/Test0.sunyat");
    robot.Position.x = arenaCenter.x;
    robot.Position.y = arenaCenter.y;
    Robot& robot2 = Robots.emplace_back();
    robot2.LoadProgramFromSource(BuildConfig::AssetFolderPath + "tests/Test1.sunyat");
    robot2.Position.x = arenaCenter.x - 100;
    robot2.Position.y = arenaCenter.y - 50;
}

void Arena::CreateBullet(const Vec2<f32>& position, const Vec2<f32>& direction, u64 creator, VmValue damage)
{
    Bullets.emplace_back(position, direction, creator, damage);
}

void Arena::CreateMine(const Vec2<f32>& position, u64 creator, VmValue damage)
{
    Mines.emplace_back(position, creator, damage);
}

void Arena::DetonateMine(Mine& mine)
{
    //Damage robots within the explosion radius
    for (Robot& robot : Robots)
    {
        if (robot.ID() == mine.Creator)
            continue;

        const f32 distance = robot.Position.Distance(mine.Position);
        if (distance <= Mine::ExplosionRadius)
        {
            const f32 damage = mine.Damage * (1 / distance); //Damage is inversely proportional with distance
            robot.Damage(damage);
        }
    }
    
    mine.Alive = false;
}

Robot* Arena::GetClosestRobot(const Vec2<f32>& position, Robot* exclude, f32 angleMinRadians, f32 angleMaxRadians)
{
    f32 distance = std::numeric_limits<f32>::infinity();
    Robot* out = nullptr;
    for (Robot& robot : Robots)
    {
        f32 angle = (robot.Position - position).Normalized().AngleUnitRadians();
        if (angle < angleMinRadians || angle > angleMaxRadians)
            continue; //Outside of search arc
        if (&robot == exclude)
            continue;

        f32 curDist = robot.Position.Distance(position);
        if (curDist < distance)
        {
            distance = curDist;
            out = &robot;
        }
    }

    return out;
}