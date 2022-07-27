#pragma once

class Renderer3D {
public:

    Renderer3D(const Renderer3D& ref) = delete;
    Renderer3D(Renderer3D&& ref) = delete;
    Renderer3D& operator=(const Renderer3D& ref) = delete;
    Renderer3D& operator=(Renderer3D&& ref) = delete;

    static void Init();
    static void Shutdown();
    static Renderer3D* Get();

private:
    Renderer3D();
    static Renderer3D* instance;

};
