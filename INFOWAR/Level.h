#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <map>
#include <vector>
#include "irrlicht.h"

enum BasicCollisionVolume
{
	BCV_NONE = 0,
	BCV_PLANE,
	BCV_SPHERE,
	BCV_BOX,
	BCV_CONVEXHULL,
	BCV_MESH,
	BCV_MAX
};

struct BasicObject
{
	std::string name;
	std::string modelFile;
	std::string textureFile;

	std::vector<std::string> otherTextures;

	irr::core::vector3df initialPosition;
	irr::core::vector3df initialOrientation;

	BasicCollisionVolume collisionType;

	double mass;
	irr::core::vector3df scale;

	BasicObject() { scale = irr::core::vector3df(1.0, 1.0, 1.0); mass = 1.0; collisionType = BCV_NONE; };
};

class Level
{
	private:
		std::string m_filename;
		
		void StartLoading();
		void LoadModels();
		void LoadTextures();
		void LoadPositions();
		void LoadCollisionInfo();
		void FinishLoading();

	public:
		std::string m_levelName;
		std::vector<std::string>	m_textureFiles;
		std::vector<std::string>	m_modelFiles;
		std::map<std::string, BasicObject> m_objects;

		Level();
		Level(std::string filename);
		~Level();

		void Load(){ StartLoading(); LoadModels(); LoadTextures(); LoadPositions(); LoadCollisionInfo(); FinishLoading(); }
		void Load(std::string filename){ m_filename = filename; Load(); }
};

#endif