// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// iMSTK includes
#include "Rendering/SceneTextureShader.h"

SceneTextureShader::SceneTextureShader(const std::string &p_verteShaderFileName, const std::string &p_fragmentFileName)
: Shader(nullptr)
{
    this->log->isOutputtoConsoleEnabled = false;
    this->checkErrorEnabled = true;
    setShaderFileName(p_verteShaderFileName, "", p_fragmentFileName);
    createParam("depthTex");
    createParam("sceneTex");
    createParam("prevTex");
    this->checkErrorEnabled = true;
    log->isOutputtoConsoleEnabled = true;
    this->registerShader();
}

void SceneTextureShader::predraw(std::shared_ptr<Core::BaseMesh>/*p_mesh*/)
{

}

void SceneTextureShader::handleEvent(std::shared_ptr<core::Event> /*p_event*/)
{

}

void SceneTextureShader::initDraw()
{
    Shader::initDraw();
    this->depthTex = getFragmentShaderParam("depthTex");
    this->sceneTex = getFragmentShaderParam("sceneTex");
    this->prevTex = getFragmentShaderParam("prevTex");
}

void SceneTextureShader::draw() const
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);

    glTranslated(0, 0, -5);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0);
    glVertex3d(-1, -1, 0);
    glTexCoord2f(1, 0);
    glVertex3d(1, -1, 0);
    glTexCoord2f(1, 1);
    glVertex3d(1, 1.0, 0);
    glTexCoord2f(0, 1);
    glVertex3d(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}
