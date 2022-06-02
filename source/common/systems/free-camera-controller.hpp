#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        void update(World* world, float deltaTime) {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if(camera && controller) break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if(!(camera && controller)) return;
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity* entity = camera->getOwner();
            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked){
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
            // If the left mouse button is released, we unlock and unhide the mouse.
            } else if(!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked) {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            // NOT Assume that the first child in the player entity is the car/truck.
            Entity* player = NULL;
            for(auto detected : world->getEntities()){
                if(detected->materialName == "player"){
                    player = detected;
                    break;
                }
            }
            

            // We get a reference to the entity's position and rotation
            glm::vec3& position = entity->localTransform.position;
            glm::vec3& rotation = entity->localTransform.rotation;
            glm::vec3& positionPlayer = player->localTransform.position;
            glm::vec3& rotationPlayer = player->localTransform.rotation;

            // If the left mouse button is pressed, we get the change in the mouse location
            // and use it to update the camera rotation
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)){
                glm::vec2 delta = app->getMouse().getMouseDelta();
                rotation.x -= delta.y * controller->rotationSensitivity; // The y-axis controls the pitch
                rotation.y -= delta.x * controller->rotationSensitivity; // The x-axis controls the yaw
            }

            // We prevent the pitch from exceeding a certain angle from the XZ plane to prevent gimbal locks
            if(rotation.x < -glm::half_pi<float>() * 0.99f) rotation.x = -glm::half_pi<float>() * 0.99f;
            if(rotation.x >  glm::half_pi<float>() * 0.99f) rotation.x  = glm::half_pi<float>() * 0.99f;
            // This is not necessary, but whenever the rotation goes outside the 0 to 2*PI range, we wrap it back inside.
            // This could prevent floating point error if the player rotates in single direction for an extremely long time. 
            rotation.y = glm::wrapAngle(rotation.y);

            // We update the camera fov based on the mouse wheel scrolling amount
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            camera->fovY = fov;

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                      up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)), 
                      right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            // If the LEFT SHIFT key is pressed, we multiply the position sensitivity by the speed up factor
            if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)) current_sensitivity *= controller->speedupFactor;

            // We change the camera position based on the keys WASD/QE
            // S & W moves the player back and forth
            if(app->getKeyboard().isPressed(GLFW_KEY_W)) 
            {
                position += front * (deltaTime * current_sensitivity.z);
                positionPlayer += front * (deltaTime * current_sensitivity.z);
            }    
            if(app->getKeyboard().isPressed(GLFW_KEY_S))
            {
                position -= front * (deltaTime * current_sensitivity.z);
                positionPlayer -= front * (deltaTime * current_sensitivity.z);
            }
            // Q & E moves the player up and down
            if(app->getKeyboard().isPressed(GLFW_KEY_Q)) position += up * (deltaTime * current_sensitivity.y);
            if(app->getKeyboard().isPressed(GLFW_KEY_E)) position -= up * (deltaTime * current_sensitivity.y);
            // A & D moves the player left or right 
            if(app->getKeyboard().isPressed(GLFW_KEY_D)) 
            {
            position += right * (deltaTime * current_sensitivity.x);
            positionPlayer += right * (deltaTime * current_sensitivity.x);
            }
            if(app->getKeyboard().isPressed(GLFW_KEY_A)) 
            {
                position -= right * (deltaTime * current_sensitivity.x);
                positionPlayer -= right * (deltaTime * current_sensitivity.x);
            }
            // Add movement through right & left arrows
            if(app->getKeyboard().isPressed(GLFW_KEY_LEFT))
            {
                position += front * glm::vec3(0.2, 0.2, 0.2);
                if(rotation.y == 0) rotation.y = glm::half_pi<float>() / 90.0;
                else rotation.y += glm::half_pi<float>() / 90.0 ;// The x-axis controls the yaw
                rotation.y = glm::wrapAngle(rotation.y);
            }
            if(app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
            {
                position += front * glm::vec3(0.2, 0.2, 0.2);
                if(rotation.y == 0) rotation.y = - glm::half_pi<float>() / 90.0;
                else rotation.y -= glm::half_pi<float>() / 90.0 ;// The x-axis controls the yaw
                rotation.y = glm::wrapAngle(rotation.y);
            }
            // entity->localTransform.position= position;
            // std::cout<<entity->localTransform.position.x<<" "<<entity->localTransform.position.y<<" "<<entity->localTransform.position.z<<std::endl;
            // world->detectCollision();
            if (player != NULL)
            for(auto detected : world->getEntities())
               {
                //    std::cout<<entity->name<<std::endl;
               
                    //    std::string entitymaterialName =entity->getComponent<MeshRendererComponent>()->material;
                     if(detected->materialName=="battery")
                       {    //  check collision logic
                            // if(world->collisionlogic(entity,detected))
                            // {
                             std::cout<<"I am here"<<std::endl;
                        //        // add value for energy
                            // }  
                            double dx,dy,dz;
                            dx= player->localTransform.position.x - detected->localTransform.position.x;
                            dy = player->localTransform.position.y - detected->localTransform.position.y;
                            dz = player->localTransform.position.z - detected->localTransform.position.z;
                            std::cout<<player->localTransform.position.x<<" "<<player->localTransform.position.y<<" "<<player->localTransform.position.z<<std::endl;
                            std::cout<<detected->localTransform.position.x<<" "<<detected->localTransform.position.y<<" "<<detected->localTransform.position.z<<std::endl;
                            double distance = sqrt(dx*dx + dy*dy + dz*dz);
                            double radiusx=0, raduisy=0;
                            Mesh *meshx;
                            Mesh *meshy;
                          //  entity->getComponent<>()
                            meshx = player->getComponent<MeshRendererComponent>()->mesh;
                            meshy = detected->getComponent<MeshRendererComponent>()->mesh;
                            radiusx = meshx->raduis;
                            raduisy = meshy->raduis;
                            if(distance <=radiusx+raduisy)
                            {
                                std::cout<<"Collision!!";  
                                world->markForRemoval(detected);
                            }
                            std::cout<<"radiusx"<<radiusx<<"raduisy"<<raduisy<<"distance"<<distance<<std::endl;
                            // return distance <=radiusx+raduisy;
                       }
                   
               }
                world->deleteMarkedEntities();
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

    };

}
