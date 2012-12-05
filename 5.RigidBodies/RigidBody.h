//
//  RigidBody.h
//  RigidBody
//
//  Created by Josiah Hester on 11/5/12.
//  Copyright (c) 2012 Josiah Hester. All rights reserved.
//

#ifndef __RigidBody__RigidBody__
#define __RigidBody__RigidBody__

#include <iostream>
#include "Quaternion.h"


class RigidBody {
public:
    
    struct State {
        // Constant
        double mass;
        double size;
        Matrix3x3 Ibody;
        Matrix3x3 Ibodyinv;
        
        // Primary
        Vector3d position; // x(t)
        Quaternion rotation; // q(t) : R(t) quaternion representation
        Vector3d linearMomentum; // P(t)
        Vector3d angularMomentum; // L(t)
        
        // Seconary
        Vector3d linearVelocity; // v(t) = xdot(t)
        Quaternion spin; // qdot(t)
        Matrix3x3 Iinv;
        Vector3d omega; // w(t), angularVelocity
        
        float inertiaTensor;
        float inverseInertiaTensor;
        
        void calculate() {
            // Translational
            linearVelocity = linearMomentum * (1/mass); // Iinv??

            // Rotational
            rotation = rotation.normalize();
            Matrix3x3 R = rotation.rotation();
            Iinv = R * Ibodyinv * R.transpose();
            omega = angularMomentum * Iinv;
            spin = 0.5f * rotation * omega;
        }
    };
    
    State current;
    State previous;
    float DragCoefficient;
    float Ks;
    float Kd;
    Vector3d targetPoint; // world coordinates
    Vector3d pointOnBody; // body coordinates
    float restLength;
    float GravityVY;
    
    struct Derivative {
        Vector3d velocity; // xdot(t)
        Vector3d force; // Pdot(t)
        Quaternion spin; // qdot(t)
        Vector3d torque; // Ldot(t)
    };
    
    Derivative evaluate(const State &state, float t)
	{
		Derivative output;
		output.velocity = state.linearVelocity;
		output.spin = state.spin;
		forces(state, t, output.force, output.torque);
		return output;
	}
    
    Derivative evaluate(State state, float t, float dt, const Derivative &derivative)
	{
		state.position = state.position + derivative.velocity * dt;
		state.linearMomentum = state.linearMomentum + derivative.force * dt;
		state.rotation = state.rotation + derivative.spin * dt;
		state.angularMomentum = state.angularMomentum + derivative.torque * dt;
		state.calculate();
		
		Derivative output;
		output.velocity = state.linearVelocity;
		output.spin = state.spin;
		forces(state, t+dt, output.force, output.torque);
		return output;
	}
    
    void integrate(State &state, float t, float dt)
	{
		Derivative a = evaluate(state, t);
		Derivative b = evaluate(state, t, dt*0.5f, a);
		Derivative c = evaluate(state, t, dt*0.5f, b);
		Derivative d = evaluate(state, t, dt, c);
		
		state.position = state.position + 1.0f/6.0f * dt * (a.velocity + 2.0f*(b.velocity + c.velocity) + d.velocity);
		state.linearMomentum = state.linearMomentum + 1.0f/6.0f * dt * (a.force + 2.0f*(b.force + c.force) + d.force);
		state.rotation = state.rotation + 1.0f/6.0f * dt * (a.spin + 2.0f*(b.spin + c.spin) + d.spin);
		state.angularMomentum = state.angularMomentum + 1.0f/6.0f * dt * (a.torque + 2.0f*(b.torque + c.torque) + d.torque);
        
		state.calculate();
	}
    
    void forces(const State &state, float t, Vector3d &force, Vector3d &torque)
	{
		/*// attract towards origin
        
		force = -10 * state.position;
        
		// sine force to add some randomness to the motion
        
		force.x += 10 * ::sin(t*0.9f + 0.5f);
		force.y += 11 * ::sin(t*0.5f + 0.4f);
		force.z += 12 * ::sin(t*0.7f + 0.9f);
        
		// sine torque to get some spinning action
        
		torque.x = 1.0f * ::sin(t*0.9f + 0.5f);
		torque.y = 1.1f * ::sin(t*0.5f + 0.4f);
		torque.z = 1.2f * ::sin(t*0.7f + 0.9f);
        
		 */
        
        // We need to convert pointOnBody to world coordinates
        Quaternion q = state.rotation;
        Vector3d newpointOnBody = pointOnBody * q.rotation() + state.position;
        Vector3d x = (targetPoint - newpointOnBody);
        force = Ks * (x.norm() - restLength) * x.normalize() -Kd * state.linearVelocity;
        
        // T = F cross (p - x)
        torque = force % (newpointOnBody - state.position);
        
        // Gravity on all points
        force = force + Vector3d(0.0, GravityVY, 0.0);
        
        // damping
        torque = torque - DragCoefficient * state.omega;
        force = force - DragCoefficient * state.linearVelocity;
	}
    
    void update(float t, float dt) {
        previous = current;
        integrate(current, t, dt);
    }
    
    RigidBody() {
        current.size = 8;
		current.mass = 1;
		current.position = Vector3d(25,15,0);
		current.linearMomentum = Vector3d(0,0,0);
		current.rotation.identity();
		current.angularMomentum = Vector3d(0,0,0);
		current.inertiaTensor = current.mass * current.size * current.size * 1.0f / 6.0f;
		current.inverseInertiaTensor = 1.0f / current.inertiaTensor;
        double inTense = current.mass/12.0 * (current.size*current.size + current.size*current.size);
        current.Ibody = Matrix3x3(inTense,0,0,
                                  0,inTense,0,
                                  0,0,inTense);
        current.Ibodyinv = current.Ibody.inv();
		current.calculate();
		previous = current;
        
        GravityVY = -4.9;
        targetPoint = Vector3d(0.0, 30.0, 0.0);
        pointOnBody = Vector3d(4.0, 4.0, 4.0);
        restLength = (targetPoint - (pointOnBody + current.position)).norm() - 13.0;
    };
    
    const Vector3d getPosition() const {
        return current.position;
    }
    
    const Quaternion getRotation() const {
        return current.rotation;
    }
    
    void setParameters(double mass, float Ksn, float Kdn, float Damping, float grav) {
        current.mass = mass;
        Ks = Ksn;
        Kd = Kdn;
        DragCoefficient = Damping;
        GravityVY = grav;
    }
};

#endif /* defined(__RigidBody__RigidBody__) */
