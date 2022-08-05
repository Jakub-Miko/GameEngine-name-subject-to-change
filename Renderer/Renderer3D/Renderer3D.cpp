#include "Renderer3D.h"

Renderer3D* Renderer3D::instance = nullptr;

void Renderer3D::Init()
{
	if (!instance) {
		instance = new Renderer3D;
	}
}

void Renderer3D::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

Renderer3D* Renderer3D::Get()
{
	return instance;
}

Renderer3D::Renderer3D() : default_descriptor_heap(500)
{

}
