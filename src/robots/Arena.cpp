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
    const f32 cyclesDelta = cyclesToExecute * timeBetweenCycles;

    //Update robots
    if (CyclesPerSecond != 0) //If CyclesPerSecond == 0, cyclesDelta == NaN. Breaks _cycleAccumulator and subsequent logic
    {
        _cycleAccumulator -= cyclesDelta;

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

            robot->Update(*this, cyclesDelta, cyclesToExecute);
            robot++;
        }
    }

    //Update bullets
    auto bullet = Bullets.begin(); //Done this way to allow deletion during iteration
    while (bullet != Bullets.end())
    {
        bullet->Position += bullet->Direction * Bullet::Speed;
        if (!IsPositionInRect(bullet->Position, Position, Size))
        {
            bullet = Bullets.erase(bullet); //Outside of arena, delete
            continue;
        }
        
        //Check if bullet collided with any robot
        bool hitRobot = false;
        for (Robot& robot : Robots)
        {
            if (robot.ID() == bullet->Creator)
                continue; //Don't interact with the robot that fired the bullet

            std::array<Vec2<f32>, 3> chassis = robot.GetChassisPoints();
            if (IsPositionInTriangle(bullet->Position, chassis[0], chassis[1], chassis[2]))
            {
                //Hit a bot. Damage bot and delete bullet
                robot.Damage(bullet->Damage);
                hitRobot = true;
                break;
            }
        }
        if (hitRobot)
        {
            bullet = Bullets.erase(bullet);
            continue;
        }
        
        bullet++;
    }

    //Update mines
    auto mine = Mines.begin();
    while (mine != Mines.end())
    {
        //Delete mines that were detonated last frame
        if (!mine->Alive)
        {
            mine = Mines.erase(mine);
            continue;
        }

        //Detect collision with robot
        bool hitRobot = false;
        for (Robot& robot : Robots)
        {
            if (robot.ID() == mine->Creator)
                continue; //Don't interact with the creator

            std::array<Vec2<f32>, 3> chassis = robot.GetChassisPoints();
            if (IsPositionInTriangle(mine->Position, chassis[0], chassis[1], chassis[2]))
            {
                //Hit a bot. Detonate mine
                DetonateMine(*mine);
                hitRobot = true;
                break;
            }
        }

        //Handle collision
        if (hitRobot)
            mine = Mines.erase(mine);
        else
            mine++;
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

    for (Mine& mine : Mines)
        renderer->DrawRectangleCentered(mine.Position, { Mine::Size, Mine::Size }, { 255, 0, 0, 255 });
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