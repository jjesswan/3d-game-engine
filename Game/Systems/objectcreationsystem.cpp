#include "objectcreationsystem.h"
#include "Game/Components/CollisionComponents/CollisionComponent.h"
#include "Game/Components/CollisionComponents/boundingtriangle.h"
#include "Game/Components/DrawComponent.h"
#include "Game/Components/PathfindComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Systems/AI/aibehaviorcomponent.h"
#include <random>

ObjectCreationSystem::ObjectCreationSystem(std::map<std::string, std::shared_ptr<GameObject>>& gameobjects,
                                           std::map<std::string, std::shared_ptr<GameObject>>& dynamic_gameobjects,
                                           std::map<std::string, std::shared_ptr<GameObject>>& rigid_gameobjects,
                                           std::map<std::string, BlackboardData>& global_blackboard,
                                           std::map<std::string, std::vector<std::shared_ptr<GameObject>>>& lootables):
    m_gameobjects(gameobjects),
    m_dynamic_gameobjects(dynamic_gameobjects),
    m_rigid_gameobjects(rigid_gameobjects),
    m_global_blackboard(global_blackboard),
    m_lootables(lootables)

{
    initializeAllObjects();
}

void ObjectCreationSystem::initializeAllObjects(){

    m_ground_level = -.5f;
    initializePlayerObject();
    //initializeSlopedGround();
    initializeGround();
    initializeBackground();


    //generateBall(glm::vec3(1.f), glm::vec3(-5, 5, -2), 1);


    makeLootable("mushroom", "mushroom_1", 10, glm::vec3(1.f));
    makeLootable("amantia", "mushroom_2", 10, glm::vec3(1.f));
    addLight();
}

void ObjectCreationSystem::makeLootable(std::string shapename, std::string filename, int count, glm::vec3 scale){
    std::vector<glm::vec3> obj_data = Global::graphics.addShape_withMaterial(shapename,
                                                                "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/" + filename + ".obj",
                                                                "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/"+ filename + ".mtl", true);

    //std::shared_ptr<Shape> shape = Global::graphics.getShape("sphere");


    for (int i=0; i<count; i++){
        std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
        mt->setScale(scale);

        // get random ground pos
        glm::vec3 randomSurfacePos = m_gameobjects.at("ground")->getComponent<CollisionComponent>()->getCollisionShape<BoundingTriangle>()->getRandomSurfacePos();
        mt->setPos(glm::vec3(randomSurfacePos.x, randomSurfacePos.y, randomSurfacePos.z));

        std::shared_ptr<GameObject> item = std::make_shared<GameObject>();

        item->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShapeGroup(shapename)));
        item->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, shapename, m_global_blackboard));
        //item->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt));
        item->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("dynamic_mesh", mt, mt->getPos()));
        m_lootables[shapename].push_back(item);

        // insert to all game objects to allow draw
        std::string name = shapename + std::to_string(i);
        insertAnyObject(name, item);

    }

}

void ObjectCreationSystem::insertAnyObject(const std::string name, const std::shared_ptr<GameObject> &game_obj){
    m_gameobjects.insert(std::pair<const std::string, std::shared_ptr<GameObject>>(name, game_obj));
}

void ObjectCreationSystem::insertRigidObject(const std::string name, const std::shared_ptr<GameObject> &game_obj){
    m_rigid_gameobjects.insert(std::pair<const std::string, std::shared_ptr<GameObject>>(name, game_obj));
    insertAnyObject(name, game_obj);
}

void ObjectCreationSystem::insertDynamicObject(const std::string name, const std::shared_ptr<GameObject> &game_obj){
    m_dynamic_gameobjects.insert(std::pair<const std::string, std::shared_ptr<GameObject>>(name, game_obj));
    insertAnyObject(name, game_obj);
}

void ObjectCreationSystem::initializePlayerObject(){
    //std::shared_ptr<Shape> shape = Global::graphics.getShape("sphere");
    std::vector<glm::vec3> obj_data = Global::graphics.addShape_withMaterial("mouse", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/mouse2-4.obj",
                                                                             "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/mouse2-4.mtl", true);
    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setScale(glm::vec3(.5f));
    mt->setPos(glm::vec3(-28.9172,20.f,43.0559));

    //std::cout << "player pos: " << mt->getPos().x << ", " << mt->getPos().y << ", " << mt->getPos().z <<std::endl;


    std::shared_ptr<GameObject> player = std::make_shared<GameObject>();

    player->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShapeGroup("mouse")));
    player->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, "player", m_global_blackboard, true));
    player->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("dynamic_mesh", mt, mt->getPos(), obj_data));

    insertDynamicObject("player", player);
}

void ObjectCreationSystem::initializeGround(){
   std::shared_ptr<GameObject> ground = std::make_shared<GameObject>();
   std::vector<glm::vec3> obj_data = Global::graphics.addShape_withMaterial("ground", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/meadow_ground.obj",
                                                              "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/meadow_ground.mtl", true);

    //std::vector<glm::vec3> obj_data = Global::graphics.addShape("ground", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/testplane.obj");
    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setPos(glm::vec3(0.f, 0.f, 0.f));
    ground->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShapeGroup("ground")));
    ground->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, "ground", m_global_blackboard));
    ground->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt, true));

    insertRigidObject("ground", ground);
}

void ObjectCreationSystem::initializeBackground(){
    std::shared_ptr<GameObject> bg = std::make_shared<GameObject>();

    // "Snowy Mountain - Terrain" (https://skfb.ly/6RzJV) by artfromheath is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
    std::vector<glm::vec3> obj_data = Global::graphics.addShape_withMaterial("bg", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/howl_field_background.obj",
                                                               "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/howl_field_background.mtl", true);

     //std::vector<glm::vec3> obj_data = Global::graphics.addShape("ground", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/testplane.obj");
     std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
     mt->setPos(glm::vec3(0.f, 0.f, 0.f));
     bg->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShapeGroup("bg")));
     bg->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, "bg", m_global_blackboard));
     bg->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt, true));

     insertRigidObject("bg", bg);

}


void ObjectCreationSystem::generateBall(glm::vec3 scale, glm::vec3 pos, int number){
    std::shared_ptr<Shape> shape = Global::graphics.getShape("sphere");
    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setScale(scale);
    mt->setPos(pos);

    std::shared_ptr<GameObject> ball = std::make_shared<GameObject>();
    std::string name = "ball" + std::to_string(number);

    ball->addComponent<DrawComponent>(std::make_unique<DrawComponent>(shape));
    ball->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, name, m_global_blackboard));
    ball->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("dynamic_mesh", mt, mt->getPos()));
    ball->addComponent<AIBehaviorComponent>(std::make_unique<AIBehaviorComponent>(name, m_global_blackboard));

    insertDynamicObject(name, ball);
}



void ObjectCreationSystem::initializeSlopedGround(){
    std::shared_ptr<GameObject> sloped_ground = std::make_shared<GameObject>();
    std::vector<glm::vec3> obj_data = Global::graphics.addShape("sloped_ground", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/floor.obj");

    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setScale(1.f);
    mt->setPos(glm::vec3(0.f));

    sloped_ground->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShape("sloped_ground")));
    sloped_ground->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, "sloped_ground", m_global_blackboard));
    sloped_ground->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt));

    insertRigidObject("sloped_ground", sloped_ground);
}

void ObjectCreationSystem::addLight(){
    std::shared_ptr<Light> light1 = std::make_shared<Light>(LightType::DIRECTIONAL, glm::vec3(0,20,6.5));
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(light1);

    Global::graphics.bindShader("phong");
    Global::graphics.setLights(lights);
}

void ObjectCreationSystem::addObject_NoTransform(std::string shape_name, std::string obj_name, std::string filename){
    std::shared_ptr<GameObject> obj = std::make_shared<GameObject>();
    std::vector<glm::vec3> obj_data = Global::graphics.addShape(shape_name, "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/" + filename);

    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setScale(1.f);
    obj->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShape(shape_name), shape_name));
    obj->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, obj_name, m_global_blackboard));
    obj->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt));

    insertRigidObject(obj_name, obj);
}

void ObjectCreationSystem::makeNavMesh(){
    std::shared_ptr<GameObject> nav = std::make_shared<GameObject>();
    std::vector<glm::vec3> obj_data = Global::graphics.addShape("slope", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/environment3nav.obj");
    //std::vector<glm::vec3> obj_data = Global::graphics.addShape("slope", "/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/dune.obj");

    std::pair<std::vector<glm::vec3>, std::vector<glm::ivec3>> navmesh = Global::graphics.getNavmeshData("/Users/jesswan/Desktop/cs1950u/cs1950u-jjesswan/Resources/Meshes/environment3nav.obj");

    std::shared_ptr<ModelTransform> mt = std::make_shared<ModelTransform>();
    mt->setScale(1.f);
    mt->setPos(glm::vec3(0.f,  0, 0));

    nav->addComponent<DrawComponent>(std::make_unique<DrawComponent>(Global::graphics.getShape("slope")));
    nav->addComponent<TransformComponent>(std::make_unique<TransformComponent>(mt, "navmesh", m_global_blackboard));
    nav->addComponent<CollisionComponent>(std::make_unique<CollisionComponent>("obj", obj_data, mt));
    nav->addComponent<PathfindComponent>(std::make_unique<PathfindComponent>(navmesh.first, navmesh.second));

    m_global_blackboard["navmesh"].environment = nav;
    insertRigidObject("navmesh", nav);
}

void ObjectCreationSystem::makeBVHDemo(){
    addObject_NoTransform("floor", "floor", "floor.obj");
    addObject_NoTransform("cube1", "cube1", "cube1.obj");
    addObject_NoTransform("cube2", "cube2", "cube2.obj");
    addObject_NoTransform("cube3", "cube3", "cube3.obj");
    addObject_NoTransform("cube4", "cube4", "cube4.obj");
    //addObject_NoTransform("nav", "nav", "environment3.obj");

}


void ObjectCreationSystem::draw(){}
void ObjectCreationSystem::update(double deltaTime){}
void ObjectCreationSystem::scrollEvent(double distance){}
void ObjectCreationSystem::mousePosEvent(double xpos, double ypos){}



