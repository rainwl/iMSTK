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

#ifndef CORE_CORE_COLLISIONMODELITERATOR_H
#define CORE_CORE_COLLISIONMODELITERATOR_H

/// \brief !!
template <typename T>
class CollisionModelIterator
{

public:
    int startIndex; ///<
    int currentIndex; ///<
    int currentLevel; ///<
    int endIndex; ///<

    inline int start(){ return 0; }
    inline int end(){return 0; }
    inline void operator++(){}
    inline void operator--(){}
    inline T operator[](int /*p_index*/){ return T(); }
};

#endif