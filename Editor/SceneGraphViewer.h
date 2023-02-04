#pragma once 

class SceneGraphViewer {
public:
	SceneGraphViewer();
	~SceneGraphViewer();

	void Render();

private:
	bool is_paste_pressed = false;


};