#pragma once

#include <cstdint>
#include <cmath>

namespace FirstPersonModel {

struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
};

struct Vec2 {
    float x, y;
    
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

class FirstPersonModelMod {
public:
    static FirstPersonModelMod& getInstance();
    
    bool initialize();
    void shutdown();
    
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    const Vec3& getCameraOffset() const { return m_cameraOffset; }
    void setCameraOffset(const Vec3& offset) { m_cameraOffset = offset; }
    
    float getModelScale() const { return m_modelScale; }
    void setModelScale(float scale) { m_modelScale = scale; }
    
    bool shouldRenderModelInFirstPerson() const;
    void onCameraRender();
    void onPlayerRender();
    
private:
    FirstPersonModelMod();
    ~FirstPersonModelMod();
    
    FirstPersonModelMod(const FirstPersonModelMod&) = delete;
    FirstPersonModelMod& operator=(const FirstPersonModelMod&) = delete;
    
    bool m_enabled = true;
    bool m_initialized = false;
    
    Vec3 m_cameraOffset = Vec3(0.0f, -0.1f, 0.3f);
    float m_modelScale = 1.0f;
    
    bool m_isFirstPerson = false;
    void* m_cameraPtr = nullptr;
};

} // namespace FirstPersonModel
