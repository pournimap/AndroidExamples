//
// Created by pourn on 09-09-2020.
//

#ifndef OBJECTPICKER_MOUSEPICKER_H
#define OBJECTPICKER_MOUSEPICKER_H


class MousePicker {
private:
    int RECURSION_COUNT = 200;


    glm::vec3 currentRay;
    glm::vec3 currentIntersectionPoint;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;


public:
    MousePicker()
    {

    }

    void setCurrentMatrices(glm::mat4 viewM, glm::mat4 projM)
    {
        viewMatrix = viewM;
        projectionMatrix = projM;
    }

    glm::vec3 getCurrentIntersectionPoint() {
        return currentIntersectionPoint;
    }

    glm::vec3 getCurrentRay()
    {
        return currentRay;
    }

    void update()
    {
        //viewMatrix = glm::mat4(1.0);
        //currentRay = calculateMouseRay()
    }

    glm::vec2 getNormalisedDeviceCoordinates(float mouseX, float mouseY)
    {
        //ndc conversion
        float x = (2.0f * mouseX) / gWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / gHeight;

        return (glm::vec2(x, y));
    }

    glm::vec4 toEyeCoords(glm::vec4 clipCoords)
    {
        glm::mat4 invertedProjection = glm::inverse(projectionMatrix);
        glm::vec4 eyeCoords = invertedProjection * clipCoords;
        return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
    }

    glm::vec3 toWorldCoords(glm::vec4 eyeCoords)
    {
       // glm::mat4 viewM = glm::mat4(1.0f);
        glm::mat4 invertedView = glm::inverse(viewMatrix);

        glm::vec4 rayWorld = invertedView * eyeCoords;
        glm::vec3 mouseRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
        mouseRay = glm::normalize(mouseRay);
        return mouseRay;
    }

    glm::vec3 calculateMouseRay(float x, float y) {
        //viewport space
        float mouseX = x;
        float mouseY = y;

        glm::vec2 normalizedCoords = getNormalisedDeviceCoordinates(mouseX, mouseY);
        glm::vec4 clipCoords = glm::vec4(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
        glm::vec4 eyeCoords = toEyeCoords(clipCoords);
        glm::vec3 worldRay = toWorldCoords(eyeCoords);

        return worldRay;
    }

    void addMouseCoordinatesToVector(float xCoordinate, float yCoordinate)
    {
        glm::vec2 pos = glm::vec2(xCoordinate, yCoordinate);
        mouseCoordinateStore.push_back(pos);
    }

    glm::vec3 getPointOnRay(glm::vec3 ray, float distance)
    {
        glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 start = glm::vec3(camPos.x, camPos.y, camPos.z);
        glm::vec3 scaledRay = glm::vec3(ray.x * distance, ray.y * distance, ray.z * distance );
        return (start+scaledRay);
    }

    glm::vec3 binarySearch(int count, float start, float finish, glm::vec3 ray)
    {
        float half = start + ((finish - start) / 2.0f);
        if (count >= RECURSION_COUNT) {
            glm::vec3 endPoint = getPointOnRay(ray, half);
            return endPoint;
        }
        if(intersectionInRange(start, half, ray))
        {
            return binarySearch(count + 1, start, half, ray);
        } else
        {
            return binarySearch(count + 1, half, finish, ray);
        }
    }

    bool intersectionInRange(float start, float finish, glm::vec3 ray)
    {

        glm::vec3 startPoint = getPointOnRay(ray, start);
        glm::vec3 endPoint = getPointOnRay(ray, finish);
        //check whether object is in between start and end point
        if(!isUnderGround(startPoint) && isUnderGround(endPoint))
        {
            return true;
        } else{
            return false;
        }
    }

    bool isUnderGround(glm::vec3 testPoint)
    {
        //here our scene is simple in xy plane, that's why i take z as scene depth
        //but in case of any scene on xz plane, we have to calculate height of object
        //on which we have to place object & compare it with y.
        float sceneDepth = -6; //TO DO : scene depth
        if(testPoint.z < sceneDepth)
        {
            return true;
        } else
        {
            return false;
        }
    }

    std::vector<glm::vec2> mouseCoordinateStore;
};


#endif //OBJECTPICKER_MOUSEPICKER_H
