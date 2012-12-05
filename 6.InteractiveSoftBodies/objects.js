DEFAULT_DAMPING = 60;
// Why is this not a float?
GRAVITY = new THREE.Vector3(-3.8, -9.8, 0);
GRAV_MULTIPLIER = 15.0;
/**
 * Spring class
 * p1 and p2 are indices
 */
THREE.Spring = function(p1, p2, Ks, Kd, restLength) {
	this.p1 = p1;
	this.p2 = p2;
	this.Ks = Ks;
	this.Kd = Kd;
	this.restLength = restLength;
};

THREE.Spring.prototype = {
	constructor : THREE.Spring
};

/**
 * Soft body cube
 */
THREE.SoftCube = function(x, y, z, Ks, Kd) {
	var textureGrass = THREE.ImageUtils.loadTexture('grass.png');
	textureGrass.magFilter = THREE.NearestFilter;
	textureGrass.minFilter = THREE.LinearMipMapLinearFilter;

	var textureGrassDirt = THREE.ImageUtils.loadTexture('grass_dirt.png');
	textureGrassDirt.magFilter = THREE.NearestFilter;
	textureGrassDirt.minFilter = THREE.LinearMipMapLinearFilter;

	var textureDirt = THREE.ImageUtils.loadTexture('dirt.png');
	textureDirt.magFilter = THREE.NearestFilter;
	textureDirt.minFilter = THREE.LinearMipMapLinearFilter;

	var material1 = new THREE.MeshLambertMaterial({
		map : textureGrass,
		ambient : 0xbbbbbb,
		vertexColors : THREE.VertexColors
	});
	var material2 = new THREE.MeshLambertMaterial({
		map : textureGrassDirt,
		ambient : 0xbbbbbb,
		vertexColors : THREE.VertexColors
	});

	var material3 = new THREE.MeshLambertMaterial({
		map : textureDirt,
		ambient : 0xbbbbbb,
		vertexColors : THREE.VertexColors
	});

	// Geometry
	var voxelGeo = new THREE.CubeGeometry(1, 1, 1);
	voxelGeo.faces[0].materialIndex = 1;
	voxelGeo.faces[1].materialIndex = 1;
	voxelGeo.faces[2].materialIndex = 0;
	voxelGeo.faces[3].materialIndex = 2;
	voxelGeo.faces[4].materialIndex = 1;
	voxelGeo.faces[5].materialIndex = 1;

	// final mesh
	THREE.Mesh.call(this, voxelGeo, new THREE.MeshFaceMaterial([material1, material2, material3]));
	this.position.set(x, y, z);
	this.castShadow = true;
	this.geometry.dynamic = true;
	this.lastX = new THREE.Vector3();
	this.lastV = new THREE.Vector3();

	/* Add springs / struts between vertices */
	this.springs = [];
	this.forces = [];
	this.velocities = [];
	this.Ks = Ks;
	this.Kd = Kd;

	// Each vertice is attached by a spring to every other vertice
	// doubling on springs for now
	for (var i = 0, vl = this.geometry.vertices.length; i < vl; i++) {
		var p1 = this.geometry.vertices[i];
		for (var j = 0; j < vl; j++) {
			if (i !== j) {
				var p2 = this.geometry.vertices[j];
				var deltax = new THREE.Vector3();
				deltax.sub(p1, p2);
				var restLength = Math.sqrt(deltax.dot(deltax));
				this.springs.push(new THREE.Spring(i, j, this.Ks, this.Kd, restLength));
			}
		}

		// Zero out velocity state vectors
		this.velocities.push(new THREE.Vector3());
	};
};

THREE.SoftCube.prototype = Object.create(THREE.Mesh.prototype);

THREE.SoftCube.prototype.calculateForces = function() {
	// Add gravity force
	for (var i = 0, vl = this.geometry.vertices.length; i < vl; i++) {
		this.forces[i] = new THREE.Vector3();
		//if (i != 0)
		this.forces[i].addSelf(GRAVITY);
		//this.forces[i].subSelf(this.velocities[i].multiplyScalar(DEFAULT_DAMPING));
	}

	// Add spring forces
	var dx = new THREE.Vector3();
	var dv = new THREE.Vector3();
	for (var i = 0, vl = this.springs.length; i < vl; i++) {
		var x1 = this.geometry.vertices[this.springs[i].p1];
		var x2 = this.geometry.vertices[this.springs[i].p2];

		var v1 = this.velocities[this.springs[i].p1];
		var v2 = this.velocities[this.springs[i].p2];

		dx.sub(x1, x2);
		dv.sub(v1, v2);
		var dist = dx.length();

		var leftTerm = -1 * this.springs[i].Ks * (dist - this.springs[i].restLength);
		var rightTerm = -1 * this.springs[i].Kd * ((dv.dot(dx)) / dist);
		var springForce = (dx.normalize()).multiplyScalar(leftTerm + rightTerm);

		//if (this.springs[i].p1 != 0)
		this.forces[this.springs[i].p1].addSelf(springForce);

		//if (this.springs[i].p2 != 0)
		this.forces[this.springs[i].p2].subSelf(springForce);

	}

};

THREE.SoftCube.prototype.changeParams = function(obj) {
	// gravity, Ks, Kd, speed, wind
	if (obj.gravity) {
		GRAVITY.y = obj.gravity;
	}
	if (obj.Ks) {
		this.Ks = obj.Ks;
	}
	if (obj.Kd) {
		this.Kd = obj.Kd;
	}
	if (obj.wind) {
		GRAVITY.x = obj.wind;
	}
	this.calculateForces();
};

THREE.SoftCube.prototype.integrateEuler = function(delta) {
	var mass = 1.0;
	var N = this.geometry.vertices.length;
	this.geometry.verticesNeedUpdate = true;
	this.matrixWorldNeedsUpdate = true;

	for (var i = 0; i < N; i++) {
		this.geometry.vertices[i].addSelf(this.velocities[i].multiplyScalar(delta));

		// f = ma, a = f / m
		var newacc = this.forces[i].multiplyScalar(delta / mass);

		// Get world coords
		var worldPosition = this.matrixWorld.multiplyVector3(this.geometry.vertices[i].clone());

		if (worldPosition.y < 0) {
			var n = new THREE.Vector3(0, 1.0, 0.0);
			this.velocities[i] = reflect(this.velocities[i], n, 0.8, 1.0);
		} else {
			this.velocities[i].addSelf(newacc);
		}
	}
};

/**
 * This is all the RK4 stuff
 */
function State() {
	// position
	this.x = new THREE.Vector3();
	// velocity
	this.v = new THREE.Vector3();

};

function Derivative() {
	// derivative of position: velocity
	this.dx = new THREE.Vector3();
	// derivative of velocity: acceleration
	this.dv = new THREE.Vector3();
};

function evaluate(initial, t, dt, derivative) { State
	var state = new State();
	//state.x = initial.x + d.dx * dt;
	state.x.add(initial.x, derivative.dx.multiplyScalar(dt));
	//state.v = initial.v + d.dv * dt;
	state.v.add(initial.v, derivative.dv.multiplyScalar(dt));

	var output = new Derivative();
	output.dx = state.v;
	output.dv = acceleration(state, t);
	return output;
};

function acceleration(state, t) {
	// Just use t as force for now
	return t.multiplyScalar(GRAV_MULTIPLIER);
};

THREE.SoftCube.prototype.integrateRK4 = function(delta) {
	var N = this.geometry.vertices.length;
	this.geometry.verticesNeedUpdate = true;
	this.matrixWorldNeedsUpdate = true;
	for (var i = 0; i < N; i++) {
		var t = this.forces[i];
		var state = new State();
		state.x = this.geometry.vertices[i].clone();
		state.v = this.velocities[i].clone();

		var a = evaluate(state, t, 0.0, new Derivative());
		var b = evaluate(state, t, delta * 0.5, a);
		var c = evaluate(state, t, delta * 0.5, b);
		var d = evaluate(state, t, delta, c);

		//vec3 dxdt = 1.0/6.0 * (a.dx + 2.0*(b.dx + c.dx) + d.dx);
		var dxdt = new THREE.Vector3();
		dxdt.add(b.dx, c.dx).multiplyScalar(2.0).addSelf(a.dx).addSelf(d.dx);
		dxdt.multiplyScalar(1.0 / 6.0);

		//vec3 dvdt = 1.0/6.0 * (a.dv + 2.0*(b.dv + c.dv) + d.dv)
		var dvdt = new THREE.Vector3();
		dvdt.add(b.dv, c.dv).multiplyScalar(2.0).addSelf(a.dv).addSelf(d.dv);
		dvdt.multiplyScalar(1.0 / 6.0);

		// Check for collision
		var worldPosition = this.matrixWorld.multiplyVector3(this.geometry.vertices[i].clone());
		if (worldPosition.y < 0) {
			var n = new THREE.Vector3(0, 1.0, 0.0);
			this.velocities[i] = reflect(this.velocities[i], n, 0.5, 1.0);
		} else {
			//state.x = state.x + dxdt * dt;
			this.geometry.vertices[i].addSelf(dxdt.multiplyScalar(delta));
			//state.v = state.v + dvdt * dt;
			this.velocities[i].addSelf(dvdt.multiplyScalar(delta));
		}
	}
};

THREE.SoftCube.prototype.simulate = function(delta) {
	this.calculateForces();
	//this.integrateEuler(delta);
	this.integrateRK4(delta);
};

function reflect(collisionVelocity, normal, rho, frict) {
	var vp = normal.clone().multiplyScalar(collisionVelocity.dot(normal));
	var vt = new THREE.Vector3().sub(collisionVelocity, vp.multiplyScalar(2.0));

	vp.multiplyScalar(-1 * rho);
	vp.addSelf(vt.multiplyScalar(1 - frict));
	return vp;
}