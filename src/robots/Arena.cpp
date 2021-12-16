#include "Arena.h"
#include "render/Renderer.h"
#include "math/Util.h"
#include "utility/Algorithms.h"
#include "utility/Sound.h"
#include <algorithm>

Arena::Arena()
{
    //Get list of robots in the /robots folder
    std::vector<std::string> robotOptions = {};
    for (auto& entry : std::filesystem::directory_iterator(BuildConfig::RobotFolderPath))
        if (entry.is_regular_file() && entry.path().extension() == ".sunyat")
            robotOptions.push_back(entry.path().filename().replace_extension("").string());

    //Pick a random set of robots to use in ::Reset() if none are provided
    size_t robotsToChoose = std::min((size_t)4, robotOptions.size());
    for (size_t i = 0; i < robotsToChoose; i++)
    {
        while (true)
        {
            //Pick a random bot that hasn't already been chosen
            std::string newBot = robotOptions[rand() % robotOptions.size()];
            if (Contains(_robotList, newBot))
                continue;

            _robotList.push_back(newBot);
            break;
        }
    }
}

void Arena::Update(f32 deltaTime)
{
    //Update robots
    if (CyclesPerSecond != 0) //If CyclesPerSecond == 0, cyclesDelta == NaN. Breaks _cycleAccumulator and subsequent logic
    {
        for (Robot* robot : Robots)
        {
            if (RobotAutoReloadEnabled)
                robot->TryReload(); //Recompile source file if it was edited

            robot->Update(*this, deltaTime, CyclesPerSecond);

            //Push out of bounds bot back into the arena
            u64 substepCount = 0;
            const Vec2<f32> velocity = (robot->Position - robot->LastPosition);
            while (!robot->ChassisInRectangle(Position, Size) && substepCount < Arena::MaxRobotCollisionSubsteps)
            {
                robot->Position -= velocity;
                substepCount++;
            }
        }
    }

    //Update bullets
    for (Bullet& bullet : Bullets)
    {
        bullet.Position += bullet.Direction * Bullet::Speed;
        if (!IsPositionInRect(bullet.Position, Position, Size))
            bullet.Alive = false;

        //Detect bullet-robot collisions
        auto anyHit = std::find_if(Robots.begin(), Robots.end(), [&](const Robot* robot)
            { return robot->ID() != bullet.Creator && robot->PointInChassis(bullet.Position); });

        //Handle collision
        if (anyHit != Robots.end())
        {
            //Hit robot. Damage it and delete the bullet
            (*anyHit)->Damage(bullet.Damage);
            Sound::PlaySound("Impact0.wav");
            bullet.Alive = false;
        }
    }

    //Update mines
    for (Mine& mine : Mines)
    {
        //Detect mine-robot collisions
        auto anyHit = std::find_if(Robots.begin(), Robots.end(), [&](const Robot* robot)
            { return robot->ID() != mine.Creator && robot->PointInChassis(mine.Position); });

        //Handle collision
        if (anyHit != Robots.end())
        {
            DetonateMine(mine);
            mine.Alive = false;
        }
    }

    //Play sound effect for robots that died this frame
    for (Robot* robot : Robots)
        if (robot->Armor <= 0)
            Sound::PlaySound("Explosion1.wav");

    //Erase dead objects
    EraseIf(Robots, [](const Robot* robot) { return robot->Armor <= 0; });
    EraseIf(Bullets, [](const Bullet& bullet) { return !bullet.Alive; });
    EraseIf(Mines, [](const Mine& mine) { return !mine.Alive; });
}

void Arena::Draw(Renderer* renderer)
{
    renderer->DrawRectangleFilled(Position, Size, { 64, 64, 64, 255 }); //Floor
    renderer->DrawRectangle(Position, Size, { 200, 0, 0, 255 }); //Border
    for (Robot* robot : Robots)
        robot->Draw(renderer);

    for (Bullet& bullet : Bullets)
        renderer->DrawLine(bullet.Position, bullet.Position + bullet.Direction * Bullet::Length, { 255, 0, 0, 255 });

    for (Mine& mine : Mines)
        renderer->DrawRectangleFilledCentered(mine.Position, { Mine::Size, Mine::Size }, { 255, 0, 0, 255 });
}

void Arena::Reset(const std::vector<std::string>& botsToAdd)
{
    //Clear arena
    for (Robot* robot : Robots)
        delete robot;
    Robots.clear();
    Bullets.clear();
    Mines.clear();

    //Get list of bots to add to the arena
    std::vector<std::string> botsToAddFinal = {};
    if (!botsToAdd.empty())
    {
        //Use provided bots if available
        botsToAddFinal = botsToAdd;
        _seed = time(NULL);
    }
    else
    {
        //Use last set of bots if none provided
        botsToAddFinal = _robotList;
    }

    //Seed rand(), used by RandomFloat() to generate robot positions
    _robotList = botsToAddFinal;
    srand(_seed);

    //Add bots to the arena
    Vec2<f32> arenaCenter = Position + (Size / 2.0f);
    for (const std::string& name : botsToAddFinal)
    {
        //Create bot
        Robot* robot = Robots.emplace_back(new Robot());
        robot->LoadProgramFromSource(BuildConfig::RobotFolderPath + name + ".sunyat");
        
        //Random position
        robot->Position = Position;
        robot->Position.x += RandomFloat(0.05f, 0.95f) * Size.x;
        robot->Position.y += RandomFloat(0.05f, 0.95f) * Size.y;
    }
}

void Arena::CreateBullet(const Vec2<f32>& position, const Vec2<f32>& direction, u64 creator, f32 damage)
{
    Bullets.emplace_back(position, direction, creator, damage);
}

void Arena::CreateMine(const Vec2<f32>& position, u64 creator, f32 damage)
{
    Mines.emplace_back(position, creator, damage);
}

void Arena::DetonateMine(Mine& mine)
{
    //Damage robots within the explosion radius
    for (Robot* robot : Robots)
    {
        if (robot->ID() == mine.Creator)
            continue;

        const f32 distance = robot->Position.Distance(mine.Position);
        if (distance <= Mine::ExplosionRadius)
        {
            const f32 damage = mine.Damage * (1 / distance); //Damage is inversely proportional with distance
            robot->Damage(damage);
        }
    }
    
    Sound::PlaySound("Explosion0.wav");
    mine.Alive = false;
}

Robot* Arena::GetClosestRobot(const Vec2<f32>& position, Robot* exclude, f32 angleMinRadians, f32 angleMaxRadians)
{
    f32 distance = std::numeric_limits<f32>::infinity();
    Robot* out = nullptr;
    for (Robot* robot : Robots)
    {
        f32 angle = (robot->Position - position).Normalized().AngleUnitRadians();
        if (angle < angleMinRadians || angle > angleMaxRadians)
            continue; //Outside of search arc
        if (robot == exclude)
            continue;

        f32 curDist = robot->Position.Distance(position);
        if (curDist < distance)
        {
            distance = curDist;
            out = robot;
        }
    }

    return out;
}