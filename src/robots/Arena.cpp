#include "Arena.h"
#include "render/Renderer.h"
#include "math/Util.h"

void Arena::Update(f32 deltaTime)
{
    //Calculate how many cycles to execute this frame
    //Only whole cycles are executed. If there's only enough time for part of a cycle it'll be accumulated for next frame
    _cycleAccumulator += deltaTime; //Accumulate cycle time
    const f32 timeBetweenCycles = 1.0f / (f32)CyclesPerSecond;
    const u32 cyclesToExecute = truncf(_cycleAccumulator / timeBetweenCycles);
    _cycleAccumulator -= cyclesToExecute * timeBetweenCycles; //Remove time for executed cycles

    auto robot = Robots.begin();
    while (robot != Robots.end())
    {
        //Robot died
        if (robot->Health <= 0)
        {
            robot = Robots.erase(robot);
            continue;
        }

        //Recompile source file if it was edited
        if (RobotAutoReloadEnabled)
            robot->TryReload();

        robot->Update(*this, deltaTime, cyclesToExecute);
        robot++;
    }
    for (Robot& robot : Robots)
    {
        //Recompile source file if it was edited
        if (RobotAutoReloadEnabled)
            robot.TryReload();

        robot.Update(*this, deltaTime, cyclesToExecute);
        //Todo: Push robots back into the arena if they're outside of it
    }

    auto bullet = Bullets.begin();
    while (bullet != Bullets.end())
    {
        bullet->Position += bullet->Direction * Bullet::Speed;
        if (!IsPositionInRect(bullet->Position, Position, Size))
        {
            bullet = Bullets.erase(bullet); //Outside of arena, delete
            continue;
        }
        
        //Check if bullet collided with any robot
        for (Robot& robot : Robots)
        {
            if (robot.ID() == bullet->Creator)
                continue; //Don't interact with the robot that fired the bullet

            std::array<Vec2<f32>, 3> chassis = robot.GetChassisPoints();
            if (IsPositionInTriangle(bullet->Position, chassis[0], chassis[1], chassis[2]))
            {
                //Hit a bot. Damage bot and delete bullet
                robot.Damage(bullet->Damage);
                bullet = Bullets.erase(bullet);
                continue;
            }
        }
        
        bullet++;

        //Todo: Check if bullet colliding with any robot other than its creator. Apply damage if so.
    }
}

void Arena::Draw(Renderer* renderer)
{
    renderer->DrawRectangleFilled(Position, Size, { 64, 64, 64, 255 }); //Floor
    renderer->DrawRectangle(Position, Size, { 200, 0, 0, 255 }); //Border
    for (Robot& robot : Robots)
        robot.Draw(renderer);

    for (Bullet& bullet : Bullets)
        renderer->DrawLine(bullet.Position, bullet.Position + bullet.Direction * Bullet::Length, { 255, 0, 0, 255 });
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
