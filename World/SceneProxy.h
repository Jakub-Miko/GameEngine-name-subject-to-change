#pragma once
#include <string>
#include <World/Entity.h>

class World;



/**
 * @brief Represents the source of the scene without it being loaded. 
*/
class SceneProxy {
public:

	struct LoadInfo {
		bool has_script; 
		std::string script;
	};

	virtual LoadInfo LoadScene(World& world) = 0;
	virtual const std::string& GetFilePath() const = 0;
	virtual ~SceneProxy() {}
};

class NativeSceneProxy : public SceneProxy {
public:
	NativeSceneProxy();
	NativeSceneProxy(const std::string& path) : path(path) {}

	virtual ~NativeSceneProxy() {}

	const std::string& GetPath() const {
		return path;
	}

	virtual LoadInfo LoadScene(World& world) override;
	virtual const std::string& GetFilePath() const override {
		return path;
	};

private:
	std::string path;
};