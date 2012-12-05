// Make sure WebGL is supported and enabled
if (!Detector.webgl)
	Detector.addGetWebGLMessage();

var camera, scene, renderer, controls, clock, container, stats;
// Picking
var projector, ray, voxelPosition = new THREE.Vector3(), tmpVec = new THREE.Vector3(), intersector, mouse2D, particleSystem;
var particleCount = 1800, particles = new THREE.Geometry(), pvels = [];

var Parameters = {
	"gravity" : -9.8,
	Ks : 1000,
	Kd : 0.5,
	"speed" : 15.0,
	wind : -2.8,
	Clear : function() {
		for (var i = 0; i < voxels.length; i++) {
			scene.remove(voxels[i]);	
		}
	},
	Reload : function() {
		window.location.reload();
	}
};

var timeElapsed, gravity = new THREE.Vector3(0.0, -9.8, 0.0);
// Physics state
var mass, restitution, friction, drag;
// Geometry and meshes
var voxels = [];
var rollOverMesh, rollOverMaterial, rollOverGeo;
// Start parameters
var epsilonX = 0.001, epsilonV = 0.001, epsilonA = 0.001;

var grassVoxelMat = (function() {
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

	return new THREE.MeshFaceMaterial([material1, material2, material3]);
})();

init();
animate();

function init() {

	camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 1, 1000);
	camera.position.x = 0;
	camera.position.y = 10;
	camera.position.z = 14;
	camera.lookAt(new THREE.Vector3(0, 8, 0));

	scene = new THREE.Scene();

	makeSolidPlane(24);
	makeTexturedCube(0, 3.5, 0);

	// Timing
	clock = new THREE.Clock();

	// Stats
	container = document.createElement('div');
	document.body.appendChild(container);
	stats = new Stats();
	stats.domElement.style.position = 'absolute';
	stats.domElement.style.top = '0px';
	stats.domElement.style.zIndex = 100;
	container.appendChild(stats.domElement);

	// Lights
	var ambientLight = new THREE.AmbientLight(0xcccccc);
	scene.add(ambientLight);

	var light2 = new THREE.SpotLight(0xffffff, 2.5);
	light2.position.set(-14, 14, -17);
	light2.target.position.set(0, 0, 0);

	light2.castShadow = true;

	light2.shadowCameraNear = 0.1;
	light2.shadowCameraFar = camera.far;
	light2.shadowCameraFov = 60;

	light2.shadowBias = 0.0000022;
	light2.shadowDarkness = 0.5;

	light2.shadowMapWidth = 2048;
	light2.shadowMapHeight = 2048;
	//light2.shadowCameraVisible = true;
	scene.add(light2);

	// Rollover stuff and plane picking
	rollOverGeo = new THREE.CubeGeometry(1, 1, 1);
	rollOverMaterial = new THREE.MeshBasicMaterial({
		color : 0xff0000,
		opacity : 0.5,
		transparent : true
	});
	rollOverMesh = new THREE.Mesh(rollOverGeo, rollOverMaterial);
	scene.add(rollOverMesh);

	projector = new THREE.Projector();

	plane = new THREE.Mesh(new THREE.PlaneGeometry(24, 24, 24, 24), new THREE.MeshBasicMaterial({
		color : 0x000000,
		opacity : 0.5,
		wireframe : true
	}));
	plane.rotation.x = -Math.PI / 2;
	plane.position.x -= 0.5;
	plane.position.z -= 0.5;
	scene.add(plane);

	mouse2D = new THREE.Vector3(0, 0, 0.5);

	// Particles
	// create the particle variables
	// create the particle variables
	var pMaterial = new THREE.ParticleBasicMaterial({
		color : 0xFFFFFF,
		size : 20,
		map : THREE.ImageUtils.loadTexture("particle.png"),
		blending : THREE.AdditiveBlending,
		transparent : true
	});

	// now create the individual particles
	for (var p = 0; p < particleCount; p++) {

		// create a particle with random
		// position values, -250 -> 250
		var pX = Math.random() * 500 - 250, pY = Math.random() * 500 - 250, pZ = Math.random() * 500 - 250, particle = new THREE.Vector3(pX, pY, pZ);
		pvels[p] = new THREE.Vector3();

		// add it to the geometry
		particles.vertices.push(particle);
	}

	// create the particle system
	particleSystem = new THREE.ParticleSystem(particles, pMaterial);
	// also update the particle system to
	// sort the particles which enables
	// the behaviour we want
	particleSystem.sortParticles = true;

	// add it to the scene
	scene.add(particleSystem);

	// Instantiate the renderer
	renderer = new THREE.WebGLRenderer({
		antialias : true,
		clearColor : 0x000000
	});
	renderer.shadowMapEnabled = true;
	renderer.shadowMapSoft = true;
	renderer.setSize(window.innerWidth, window.innerHeight);
	//renderer.sortObjects = false;

	// Create controls tp view scene
	controls = new THREE.TrackballControls(camera, renderer.domElement);
	controls.rotateSpeed = 0.08;
	controls.zoomSpeed = 0.05;
	controls.panSpeed = 0.2;

	controls.noZoom = false;
	controls.noPan = true;
	controls.noRotate = false;

	controls.staticMoving = false;
	controls.dynamicDampingFactor = 0.3;

	controls.minDistance = 4.1;
	controls.maxDistance = 100;

	controls.keys = [65, 83, 68];
	// [ rotateKey, zoomKey, panKey ]

	// GUI controls
	//var text = new Parameters();
	var gui = new dat.GUI({
		height : 5 * 32 - 1
	});
	var f1 = gui.addFolder('Parameters');
	f1.add(Parameters, "gravity", {
		Half : -4.9,
		Regular : -9.8,
		Double : -19.6
	}).onFinishChange(function(value) {
		for (var i = 0; i < voxels.length; i++) {
			voxels[i].changeParams({
				gravity : value
			});
		}
	});

	f1.add(Parameters, 'Ks', 0.0, 2000.0).onFinishChange(function(value) {
		for (var i = 0; i < voxels.length; i++) {
			voxels[i].changeParams({
				Ks : value
			});
		};

	});
	f1.add(Parameters, 'Kd', 0.0, 10.0).onFinishChange(function(value) {
		for (var i = 0; i < voxels.length; i++) {
			voxels[i].changeParams({
				Kd : value
			});
		}
	});
	f1.add(Parameters, 'speed', 0.0, 20.0).onFinishChange(function(value) {
		GRAV_MULTIPLIER = value;
	});
	;
	f1.add(Parameters, 'wind', -10.0, 10.0).onFinishChange(function(value) {
		for (var i = 0; i < voxels.length; i++) {
			voxels[i].changeParams({
				wind : value
			});
		}
	});
	f1.open();
	gui.add(Parameters, 'Clear');
	gui.add(Parameters, 'Reload');
	// Set / create the drawing surface dom element
	document.body.appendChild(renderer.domElement);

	document.addEventListener('mousemove', onDocumentMouseMove, false);
	document.addEventListener('click', onDocumentMouseDown, false);
	/* document.addEventListener('keydown', onDocumentKeyDown, false);
	 document.addEventListener('keyup', onDocumentKeyUp, false); */
}

function makeTexturedCube(x, y, z) {
	var mesh = new THREE.SoftCube(x, y, z, 1000, 0.5);
	scene.add(mesh);
	voxels.push(mesh);
}

function makeSolidPlane(size) {
	var geometry = new THREE.Geometry();
	var dummy = new THREE.Mesh();
	dummy.receiveShadow = true;
	for (var i = -(size / 2); i < size / 2; i++) {
		for (var j = -(size / 2); j < size / 2; j++) {
			var voxi = makeGrassVoxel();
			dummy.position.x = i;
			dummy.position.y = -0.51;
			dummy.position.z = j;
			dummy.geometry = voxi;
			THREE.GeometryUtils.merge(geometry, dummy);
		}
	}

	var mesh = new THREE.Mesh(geometry, grassVoxelMat);
	mesh.receiveShadow = true;
	mesh.name = 'floor';
	scene.add(mesh);
}

function makeGrassVoxel() {
	var voxelGeo = new THREE.CubeGeometry(1, 1, 1);
	voxelGeo.faces[0].materialIndex = 1;
	voxelGeo.faces[1].materialIndex = 1;
	voxelGeo.faces[2].materialIndex = 0;
	voxelGeo.faces[3].materialIndex = 2;
	voxelGeo.faces[4].materialIndex = 1;
	voxelGeo.faces[5].materialIndex = 1;
	return voxelGeo;
}

function getRealIntersector(intersects) {
	for (var i = 0; i < intersects.length; i++) {
		intersector = intersects[i];
		if (intersector.object != rollOverMesh) {
			return intersector;
		}
	}
	return null;
}

function setVoxelPosition(intersector) {
	tmpVec.copy(intersector.face.normal);
	voxelPosition = intersector.point.clone();
	voxelPosition.x = Math.floor(voxelPosition.x);
	voxelPosition.y = 0.5
	voxelPosition.z = Math.floor(voxelPosition.z);
}

function onDocumentMouseMove(event) {
	event.preventDefault();
	mouse2D.x = (event.clientX / window.innerWidth ) * 2 - 1;
	mouse2D.y = -(event.clientY / window.innerHeight ) * 2 + 1;
}

function onDocumentMouseDown(event) {
	makeTexturedCube(rollOverMesh.position.x, 3, rollOverMesh.position.z);
}

function simulateSoftBody(delta) {
	for (var i = 0; i < voxels.length; i++) {
		var vox = voxels[i];
		vox.simulate(delta);
	};
}

function updateParticles() {
	// add some rotation to the system
	particleSystem.rotation.y += 0.001;
	var pCount = particleCount;
	while (pCount--) {
		// get the particle
		var particle = particles.vertices[pCount];

		// check if we need to reset
		if (particle.y < -200) {
			particle.y = 200;
			pvels[pCount].y = 0;
		}
		// update the velocity
		pvels[pCount].y -= Math.random() * .01;

		// and the position
		particle.addSelf(pvels[pCount]);
	}

	particleSystem.geometry.verticesNeedUpdate = true;
}

var timeElapsed = 0;
function animate() {
	requestAnimationFrame(animate);
	updateParticles();

	var delta = clock.getDelta();
	simulateSoftBody(delta);
	timeElapsed += delta;

	// Do picking
	ray = projector.pickingRay(mouse2D.clone(), camera);
	var intersects = ray.intersectObjects(scene.children);
	if (intersects.length > 0) {
		intersector = getRealIntersector(intersects);
		if (intersector) {
			setVoxelPosition(intersector);
			rollOverMesh.position = voxelPosition;
		}
	}

	// Now render the new scene
	controls.update();
	renderer.render(scene, camera);
	stats.update();
}