#include "Level.h"
#include <string>

const std::map<std::string, BasicCollisionVolume> CollisionMap
{
	{ "NONE", BCV_NONE },
	{ "PLANE", BCV_PLANE },
	{ "SPHERE", BCV_SPHERE },
	{ "BOX", BCV_BOX },
	{ "CONVEXHULL", BCV_CONVEXHULL },
	{ "MESH", BCV_MESH }
};

Level::Level()
{
	m_levelName = "test";
	m_filename = "resources/levels/test.lvl";
}

Level::Level(std::string filename)
{
	m_filename = filename;
}

Level::~Level()
{
	
}

void Level::LoadModels()
{
	FILE *f = fopen(m_filename.c_str(), "r");
	if (f)
	{
		//scan the file for obj $(%s) %s
		char line[1024] = { 0 };
		while (fgets(line, 1023, f))
		{
			double sx = 1.0, sy = 1.0, sz = 1.0;
			char name[256] = { 0 };
			char model[256] = { 0 };

			if (sscanf(line, "obj $( %s ) %s", name, model) == 2)
			{
				bool found = false;
				
				for (std::string s : m_modelFiles)
				{
					if (s == model)
						found = true;
				}

				if (!found)
					m_modelFiles.push_back(model);

				m_objects[name].name = name;
				m_objects[name].modelFile = model;
			}

			if (sscanf(line, "scl $( %s ) %lf %lf %lf", name, &sx, &sy, &sz) == 4)
			{
				m_objects[name].name = name;
				m_objects[name].scale = irr::core::vector3df(sx, sy, sz);
			}

			memset(line, 0, 1023);
		}

		fclose(f);
	}
}

void Level::LoadTextures()
{
	FILE *f = fopen(m_filename.c_str(), "r");
	if (f)
	{
		//scan the file for tex $(%s) %s
		char line[1024] = { 0 };
		while (fgets(line, 1023, f))
		{
			char name[256] = { 0 };
			char tex[256] = { 0 };

			if (sscanf(line, "tex $( %s ) %s", name, tex) == 2)
			{
				bool found = false;

				for (std::string s : m_textureFiles)
				{
					if (s == tex)
						found = true;
				}

				if (!found)
					m_textureFiles.push_back(tex);

				m_objects[name].name = name;
				
				if (m_objects[name].textureFile.size()) m_objects[name].otherTextures.push_back(tex);
				else m_objects[name].textureFile = tex;
			}

			memset(line, 0, 1023);
		}

		fclose(f);
	}
}

void Level::LoadPositions()
{
	FILE *f = fopen(m_filename.c_str(), "r");
	if (f)
	{
		//scan the file for pos $(%s) %f %f %f %f %f %f
		char line[1024] = { 0 };
		while (fgets(line, 1023, f))
		{
			char name[256] = { 0 };
			irr::core::vector3df pos;
			irr::core::vector3df rot;

			if (sscanf(line, "pos $( %s ) %f %f %f %f %f %f", name, &pos.X, &pos.Y, &pos.Z, &rot.X, &rot.Y, &rot.Z) == 7)
			{
				m_objects[name].name = name;
				m_objects[name].initialPosition = pos;
				m_objects[name].initialOrientation = rot;
			}
			else if (sscanf(line, "pos $( %s ) %f %f %f", name, &pos.X, &pos.Y, &pos.Z) == 4)
			{
				m_objects[name].name = name;
				m_objects[name].initialPosition = pos;
				m_objects[name].initialOrientation = rot;
			}

			memset(line, 0, 1023);
		}

		fclose(f);
	}
}

void Level::LoadCollisionInfo()
{
	FILE *f = fopen(m_filename.c_str(), "r");
	if (f)
	{
		//scan the file for col $(%s) %s
		char line[1024] = { 0 };
		while (fgets(line, 1023, f))
		{
			double mass = 0.0;
			char name[256] = { 0 };
			char type[256] = { 0 };

			if (sscanf(line, "col $( %s ) %s", name, type) == 2)
			{
				m_objects[name].name = name;
				m_objects[name].collisionType = CollisionMap.at(type);
			}

			if (sscanf(line, "mass $( %s ) %lf", name, &mass) == 2)
			{
				m_objects[name].name = name;
				m_objects[name].mass = mass;
			}
		}

		fclose(f);
	}
}

void Level::StartLoading()
{

}

void Level::FinishLoading()
{
	//we have everything out of the file that we needed

	//for (std::map<std::string, BasicObject>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	//{
	//	//add each model and texture to the irrlicht arrays
	//	if (it->second.modelFile.size() && m_models.find(it->second.modelFile) == m_models.end())
	//	{
	//		m_models[it->second.modelFile] = irrScene->getMesh(it->second.modelFile.c_str());
	//	}

	//	if (it->second.textureFile.size() && m_textures.find(it->second.textureFile) == m_textures.end())
	//	{
	//		m_textures[it->second.textureFile] = irrDriver->getTexture(it->second.textureFile.c_str());
	//	}

	//	if(it->second.modelFile.size()) 
	//		m_modelNodes[it->second.name] = irrScene->addAnimatedMeshSceneNode(m_models[it->second.modelFile], 0, -1, it->second.initialPosition, it->second.initialOrientation, irr::core::vector3df(10, 10, 10));

	//	m_modelNodes[it->second.name]->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	//	m_modelNodes[it->second.name]->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
	//	if(it->second.textureFile.size()) 
	//		m_modelNodes[it->second.name]->setMaterialTexture(0, m_textures[it->second.textureFile]);

	//	m_modelNodes[it->second.name]->setDebugDataVisible(irr::scene::EDS_MESH_WIRE_OVERLAY);

	//	btTransform trans;
	//	trans.setIdentity();
	//	trans.setOrigin(btVector3(it->second.initialPosition.X, it->second.initialPosition.Y, it->second.initialPosition.Z));

	//	btDefaultMotionState *motionstate = new btDefaultMotionState(trans);

	//	//this is where it becomes dependent on the collision type
	//	switch (it->second.collisionType)
	//	{
	//	case BCV_NONE:
	//		break;

	//	case BCV_PLANE:
	//		break;

	//	case BCV_SPHERE:
	//		break;

	//	case BCV_BOX:
	//		break;

	//	case BCV_CONVEXHULL:
	//		break;

	//	case BCV_MESH:
	//		break;

	//	case BCV_MAX:	//shouldn't ever happen!
	//		break;

	//	default:
	//		break;
	//	}
	//}

	//place objects in the world and associate with Bullet rigid bodies
	
}