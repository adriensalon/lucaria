#pragma once

#include <btBulletDynamicsCommon.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    struct float32x3_hash {
        std::size_t operator()(const float32x3& vec) const
        {
            std::size_t _h1 = std::hash<float32> {}(vec.x);
            std::size_t _h2 = std::hash<float32> {}(vec.y);
            std::size_t _h3 = std::hash<float32> {}(vec.z);
            return _h1 ^ (_h2 << 1) ^ (_h3 << 2);
        }
    };

    struct rendering_guizmos : public btIDebugDraw {

        std::unordered_map<float32x3, std::vector<float32x3>, float32x3_hash> positions = {};
        std::unordered_map<float32x3, std::vector<uint32x2>, float32x3_hash> indices = {};

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
        {
            const float32x3 _color(color.x(), color.y(), color.z());
            std::vector<float32x3>& _positions = positions[_color];
            std::vector<uint32x2>& _indices = indices[_color];
            const uint32 _from_index = static_cast<uint32>(_positions.size());
            const uint32 _to_index = _from_index + 1;
            _positions.emplace_back(from.x(), from.y(), from.z());
            _positions.emplace_back(to.x(), to.y(), to.z());
            _indices.emplace_back(uint32x2(_from_index, _to_index));
        }

        virtual void reportErrorWarning(const char* warning) override
        {
            std::cout << "Bullet warning: " << warning << std::endl;
        }

        virtual void drawContactPoint(const btVector3& point_on_b, const btVector3& normal_on_b, btScalar distance, int lifetime, const btVector3& color) override
        {
            drawLine(point_on_b, point_on_b + normal_on_b * distance, color);
        }

        virtual void draw3dText(const btVector3& location, const char* text) override
        {
            std::cout << "Bullet 3D text: " << text << " at (" << location.x() << ", " << location.y() << ", " << location.z() << ")" << std::endl;
        }

        virtual void setDebugMode(int mode) override
        {
            _debug_mode = mode;
        }

        virtual int getDebugMode() const override
        {
            return _debug_mode;
        }

    private:
        int _debug_mode = DBG_DrawWireframe;
    };

}
}