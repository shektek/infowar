#include "AppManager.h"

AppManager::AppManager()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	irrDevice = nullptr;

	m_width = 1600;
	m_height = 900;
	m_vsync = true;
	m_fullscreen = false;
	m_bpp = 32;
	m_windowName = "SteelEngine";

	m_currentLevel = 0;
}

AppManager::AppManager(int argc, char **argv)
{

	irrDevice = nullptr;

	m_width = 640;
	m_height = 480;
	m_vsync = false;
	m_fullscreen = false;
	m_bpp = 32;
	m_windowName = "SteelEngine";

	m_currentLevel;
}

AppManager::~AppManager()
{

}

bool AppManager::Init()
{
	bool ret = false;
	irrDevice = irr::createDevice(irr::video::EDT_DIRECT3D9, irr::core::dimension2d<irr::u32>(m_width, m_height), m_bpp, m_fullscreen, true, m_vsync, nullptr);

	if (irrDevice)
	{
		wchar_t str[256] = { 0 };

		MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, m_windowName.c_str(), m_windowName.size(), str, 256);

		irrDevice->setWindowCaption(str);

		irrDriver = irrDevice->getVideoDriver();
		irrScene = irrDevice->getSceneManager();	
		irrTimer = irrDevice->getTimer();
		irrGUI = irrDevice->getGUIEnvironment();

		//set up gui stuff
		irrGUI->getSkin()->setFont(irrGUI->getFont("resources/fonts/cormorant_large.png"));
		m_textures["logo"] = irrDriver->getTexture("resources/textures/logo.png");

		//done, set the event receiver
		irrDevice->setEventReceiver(this);

		//get main game file and start
		LoadLevelset("resources/IWAR.gm");
		InitLevel(0);

		m_running = true;
		ret = true;
	}

	return ret;
}

void AppManager::Run()
{
	irr::u32 now = irrTimer->getTime(), delta = 0;

	while (m_running)
	{
		delta = irrTimer->getTime() - now;
		now = irrTimer->getTime();
		lastDelta = delta;

		m_running = irrDevice->run();

		UpdatePhysics(delta);
		
		UpdateCharacters(delta);

		irrDriver->beginScene();

		irrScene->drawAll();

		irrDriver->setMaterial(debugMat);
		irrDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
		btWorld->debugDrawWorld();

		//ground detection
		btVector3 start = currentCharacter->GetRigidBody()->getCenterOfMassPosition();
		btVector3 end = start - btVector3(0, currentCharacter->GetRigidBody()->getCollisionShape()->getLocalScaling().y() *3.0, 0);

		debugDraw->drawLine(start, end, btVector3(0, 1, 1));

		if (currentCharacter->GetCharacterInfo().lookDirection == LD_LEFT)
		{
			end = start - btVector3(currentCharacter->GetRigidBody()->getCollisionShape()->getLocalScaling().x() * 6.0, 0, 0);
		}
		else
		{
			end = start + btVector3(currentCharacter->GetRigidBody()->getCollisionShape()->getLocalScaling().x() * 6.0, 0, 0);
		}

		debugDraw->drawLine(start, end, btVector3(1, 0, 1));

		UpdateGUI();
		irrGUI->drawAll();

		irrDriver->endScene();
	}
}

bool AppManager::OnEvent(const irr::SEvent &sevent)
{
	bool ret = false;

	switch (sevent.EventType)
	{
		case irr::EEVENT_TYPE::EET_GUI_EVENT:
			break;
		

		case irr::EEVENT_TYPE::EET_KEY_INPUT_EVENT:
		m_lastKeyPress = sevent.KeyInput.Key;
		if (currentCharacter->m_keyActionMap.find(sevent.KeyInput.Key) != currentCharacter->m_keyActionMap.end())
		{
			int ac = AT_NONE;
			
			if (sevent.KeyInput.PressedDown) 
				ac = currentCharacter->m_keyActionMap[sevent.KeyInput.Key];
			else 
				ac = AT_NONE + currentCharacter->m_keyActionMap[sevent.KeyInput.Key];

			printf("%d ", ac);
			//before doing this, check the player's grounding state
			//there's no point checking every frame, only on actions
			currentCharacter->SetGrounded(currentCharacter->CheckGrounded(btWorld));
			printf("\n%s ground", currentCharacter->GetGrounded() == true ? "ON" : "ABOVE");
			//this could be dependent on character's grounded state also, for different gameplay rules
			currentCharacter->Action((ActionType)ac, btWorld);
			
			ret = true;
		}
			break;



		default:
			ret = false;
	}

	return ret;
}

void AppManager::LoadLevelset(std::string gameFile)
{
	FILE *f = fopen(gameFile.c_str(), "r");
	if (f)
	{
		char str[256] = { 0 };
		while (fgets(str, 255, f))
		{
			str[strcspn(str, "\n")] = 0;
			m_levels.push_back(new Level(str));
		}
		fclose(f);
	}
}

void AppManager::UnloadLevel()
{
	for (btRigidBody *rb : btObjects)
	{
		irr::scene::ISceneNode *n = static_cast<irr::scene::ISceneNode*>(rb->getUserPointer());
		n->remove();
		btWorld->removeRigidBody(rb);
		delete rb->getMotionState();
		delete rb->getCollisionShape();
		delete rb;
		rb = nullptr;
	}

	btObjects.clear();

	delete btWorld;
	delete btSolver;
	delete btDispatcher;
	delete btBroadPhase;
	delete btCollisionConfig;

	m_modelNodes.clear();
}

void AppManager::InitLevel(int index)
{
	if (index >= 0 && index < m_levels.size())
	{
		//unload the current level
		UnloadLevel();

		//load the new one
		m_currentLevel = index;

		btBroadPhase = new btAxisSweep3(btVector3(-200, -200, -200), btVector3(200, 200, 200));
		btCollisionConfig = new btDefaultCollisionConfiguration();
		btDispatcher = new btCollisionDispatcher(btCollisionConfig);
		btSolver = new btSequentialImpulseConstraintSolver();
		btWorld = new btDiscreteDynamicsWorld(btDispatcher, btBroadPhase, btSolver, btCollisionConfig);
		btWorld->getSolverInfo().m_solverMode |= SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;
		btWorld->getSolverInfo().m_numIterations = 5;
		btWorld->setGravity(btVector3(0, -9.8, 0));

		m_cameras[m_levels[index]->m_levelName] = irrScene->addCameraSceneNode();

		//irr::core::matrix4 mat;
		//mat.buildProjectionMatrixOrthoLH(m_width, m_height, 0, 500);
		//m_cameras[m_levels[index]->m_levelName]->setProjectionMatrix(mat, true);

		m_cameras[m_levels[index]->m_levelName]->setPosition(irr::core::vector3df(-20, 20, -45));

		m_levels[index]->Load();
		
		//first go through the list of individual textures and models and load them
		for (std::string mdlFile : m_levels[index]->m_modelFiles)
		{
			m_models[mdlFile] = irrDevice->getSceneManager()->getMesh(mdlFile.c_str());
		}

		for (std::string texFile : m_levels[index]->m_textureFiles)
		{
			m_textures[texFile] = irrDriver->getTexture(texFile.c_str());
		}

		for (std::map<std::string, BasicObject>::iterator it = m_levels[index]->m_objects.begin(); it != m_levels[index]->m_objects.end(); it++)
		{
			m_modelNodes[it->second.name] = irrDevice->getSceneManager()->addAnimatedMeshSceneNode(m_models[it->second.modelFile]);
			//, 0, -1, it->second.initialPosition, it->second.initialOrientation);
			m_modelNodes[it->second.name]->setRotation(it->second.initialOrientation);
			m_modelNodes[it->second.name]->setPosition(it->second.initialPosition);
			m_modelNodes[it->second.name]->setScale(it->second.scale);

			m_modelNodes[it->second.name]->setMaterialFlag(irr::video::EMF_LIGHTING, false);
			m_modelNodes[it->second.name]->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, false);
			m_modelNodes[it->second.name]->setMaterialTexture(0, m_textures[it->second.textureFile]);
			
			int count = 1;
			for (std::string ot : it->second.otherTextures)
			{
				m_modelNodes[it->second.name]->setMaterialTexture(count, m_textures[ot]);
				//m_modelNodes[it->second.name]->setMaterialType(irr::video::EMT_DETAIL_MAP);

				count++;
			}

			//create bullet objects
			m_modelNodes[it->second.name]->updateAbsolutePosition();

			btTransform trans;
			trans.setIdentity();
			btVector3 p = { it->second.initialPosition.X, it->second.initialPosition.Y, it->second.initialPosition.Z };
			//btVector3 p = { m_modelNodes[it->second.name]->getMesh()->getBoundingBox().getCenter().X, m_modelNodes[it->second.name]->getMesh()->getBoundingBox().getCenter().Y, m_modelNodes[it->second.name]->getMesh()->getBoundingBox().getCenter().Y };
			trans.setOrigin(p);
			btVector3 o = { it->second.initialOrientation.X, it->second.initialOrientation.Y, it->second.initialOrientation.Z };
			trans.setRotation(btQuaternion(o.x(), o.y(), o.z()));
			btDefaultMotionState *ms = new btDefaultMotionState(trans);

			btCollisionShape *shape = nullptr;

			//get the maximum extent
			irr::scene::IAnimatedMeshSceneNode *meshnode = m_modelNodes[it->second.name];
			irr::scene::IAnimatedMesh *m = m_modelNodes[it->second.name]->getMesh();
			irr::core::aabbox3df ab = m->getBoundingBox();
			
			//stupid thing
			const irr::core::vector3df nodescale = meshnode->getScale();
			//irr::scene::IMesh *mesh = meshnode->getMesh();
			const size_t bufsize = m->getMeshBufferCount();
//			btVector3 pos(meshnode->getPosition().X, meshnode->getPosition().Y, meshnode->getPosition().Z);

			std::vector<irr::video::S3DVertex> verts;
			std::vector<int> inds;

			//for (int i = 0; i < bufsize; i++)
			//{
			int i = 0;	//whatever
				irr::scene::IMeshBuffer *buffer = m->getMeshBuffer(i);
				const irr::video::E_VERTEX_TYPE vertexType = buffer->getVertexType();
				const irr::video::E_INDEX_TYPE indexType = buffer->getIndexType();
				const int numVerts = buffer->getVertexCount();
				const int numInds = buffer->getIndexCount();

				verts.resize(verts.size() + numVerts);
				inds.resize(inds.size() + numInds);

				void *vertices = buffer->getVertices();
				void *indices = buffer->getIndices();

				irr::video::S3DVertex *standard = reinterpret_cast<irr::video::S3DVertex*>(vertices);
				irr::video::S3DVertex2TCoords *two2coords = reinterpret_cast<irr::video::S3DVertex2TCoords*>(vertices);
				irr::video::S3DVertexTangents *tangents = reinterpret_cast<irr::video::S3DVertexTangents*>(vertices);

				irr::u16 *ind16 = reinterpret_cast<irr::u16*>(indices);
				irr::u32 *ind32 = reinterpret_cast<irr::u32*>(indices);

				btVector3 minv = { verts[0].Pos.X, verts[0].Pos.Y, verts[0].Pos.Z };
				btVector3 maxv = minv;

				for (int v = 0; v < numVerts; v++)
				{
					auto &vert = verts[v];

					switch (vertexType)
					{
						case irr::video::EVT_STANDARD:
							{
								const auto &irrv = standard[v];
								vert = irrv;

								if (minv.x() > vert.Pos.X) minv.setX(vert.Pos.X);
								if (minv.y() > vert.Pos.Y) minv.setY(vert.Pos.Y);
								if (minv.z() > vert.Pos.Z) minv.setZ(vert.Pos.Z);

								if (maxv.x() < vert.Pos.X) maxv.setX(vert.Pos.X);
								if (maxv.y() < vert.Pos.Y) maxv.setY(vert.Pos.Y);
								if (maxv.z() < vert.Pos.Z) maxv.setZ(vert.Pos.Z);
							}
						break;

						default:
						//unknown
						break;
					}
				}

				for (int n = 0; n < numInds; n++)
				{
					auto &index = inds[n];

					switch (indexType)
					{
					case irr::video::EIT_16BIT:
					index = ind16[n];
					break;

					case irr::video::EIT_32BIT:
					index = ind32[n];
					break;

					default:
					//unknown
					break;
					}
				}
			//}

			switch (it->second.collisionType)
			{
				case BCV_NONE:
				shape = nullptr;
					break;

				case BCV_PLANE:
				{
					//get these from irrlicht, somehow...
								  //no actually - the orientation of it takes care of that
								  btVector3 norm = { 0, 1, 0 };
					btScalar con = 1.0;
					shape = new btStaticPlaneShape(norm, con);
					
					
				}
					break;

				case BCV_SPHERE:
				{
					btScalar radius = 0.0;

					radius = maxv.length() * it->second.scale.X;

					shape = new btSphereShape(radius);
					
				}
					break;

				case BCV_BOX:
				{
					btVector3 halfext = { maxv.x() * (btScalar)it->second.scale.X, maxv.y() * (btScalar)it->second.scale.Y, maxv.z() * (btScalar)it->second.scale.Z };
					shape = new btBoxShape(halfext);
				}
					break;

				case BCV_CONVEXHULL:
				{
									   /*mass = 1.0;
									   shape = new btConvexHullShape()*/
				}
					break;

				case BCV_MESH:

					break;

				case BCV_MAX:	//shouldn't ever happen!
					break;

				default:
					break;
			}

			btVector3 li;
			shape->calculateLocalInertia(it->second.mass, li);

			btRigidBody *rb = new btRigidBody(it->second.mass, ms, shape, li);
			rb->setFriction(2.0);
			rb->setUserPointer((void*)m_modelNodes[it->second.name]);
			btWorld->addRigidBody(rb);
			btObjects.push_back(rb);

			//set idle animation frames...
			//VERY temporary
			if (it->second.name == "goblin")
			{
				//m_modelNodes[it->second.name]->setMD2Animation("stand");
				currentCharacter = new PlayerCharacterController( *btObjects.getLast(), new MD2AnimationController(m_modelNodes[it->second.name]));
			}
		}		

		//set up the character
		//btObjects.getLast()


		debugDraw = new DebugDraw(irrDevice);
		debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe |
			btIDebugDraw::DBG_DrawAabb |
			btIDebugDraw::DBG_DrawContactPoints |
			btIDebugDraw::DBG_DrawConstraints);
		btWorld->setDebugDrawer(debugDraw);
		debugMat.Lighting = false;

	}
}

void AppManager::UpdateGUI()
{
	//nothing yet
}

void AppManager::UpdatePhysics(irr::u32 delta)
{
	btWorld->stepSimulation(delta * 0.001, 60);

	for (btRigidBody *rb : btObjects)
	{
		irr::scene::ISceneNode *node = static_cast<irr::scene::ISceneNode*>(rb->getUserPointer());

		btVector3 point = rb->getCenterOfMassPosition();
		node->setPosition(irr::core::vector3df((irr::f32)point[0], (irr::f32)point[1], (irr::f32)point[2]));

		btVector3 eulers;
		QuaternionToEuler(rb->getOrientation(), eulers);
		node->setRotation(irr::core::vector3df(eulers[0], eulers[1], eulers[2]));
	}
}

//eventually include AI processing in here too
void AppManager::UpdateCharacters(irr::u32 delta)
{
	currentCharacter->Update(delta);
}

void AppManager::Destroy()
{
	UnloadLevel();

	irrDevice->drop();
}