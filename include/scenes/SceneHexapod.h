#pragma once
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "Scene.h"
#include "Hexapod.h"

using namespace ci;

class SceneHexapod : public Scene
{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void drawTexts() override;
    void toggleCamUi(bool enable) override;

private:
    CameraPersp mCam;
    CameraUi mCamUi;

    gl::TextureRef mTextTexture;
    gl::BatchRef mWirePlane;
    gl::BatchRef mFloor;

    Hexapod mHexapod;

    mat4 mWorldAxisMatrix;
    vec2 mWorldAxisViewportPos;
    vec2 mWorldAxisViewportSize;

    bool mFollowHexapod = false;
    vec3 mCamOffset = vec3(0, 25, -10);

    //Shadow mapping variables
    unsigned int mDepthMapFBO;
    const unsigned int SHADOW_WIDTH = 1024;    
    const unsigned int SHADOW_HEIGHT = 1024;
    unsigned int mDepthMap;

    void createShadowMap();
    void drawGUI();
};