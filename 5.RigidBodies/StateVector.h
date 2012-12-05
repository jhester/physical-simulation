//
//  StateVector.h
//  RigidBody
//
//  Created by Josiah Hester on 11/8/12.
//  Copyright (c) 2012 Josiah Hester. All rights reserved.
//

#ifndef __RigidBody__StateVector__
#define __RigidBody__StateVector__

#include <iostream>
#include "Quaternion.h"

class StateVector {
public:
    Vector3d position;
    Vector3d velocity;
    
    StateVector(Vector3d pos, Vector3d vel) : position(pos), velocity(vel) {
    }
    
       
    StateVector operator+(const StateVector& sv1) const {
        return StateVector(Vector3d(position+sv1.position), Vector3d(velocity+sv1.velocity));
    }
    StateVector operator*(double s) const {
        return StateVector(s*position, s*velocity);
    }

};

#endif /* defined(__RigidBody__StateVector__) */
