#include "pr.hpp"
#include <iostream>
#include <memory>

// return { U, V, W }, where U * v1 + V * v2 + W * v0 is the point
inline glm::vec3 closestBarycentricCoordinateOnTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 P)
{
    glm::vec3 d0 = v0 - P;
    glm::vec3 d1 = v1 - P;
    glm::vec3 d2 = v2 - P;
    glm::vec3 e0 = v2 - v0;
    glm::vec3 e1 = v0 - v1;
    glm::vec3 e2 = v1 - v2;
    glm::vec3 Ng = glm::cross( e2, e0 );

    // bc inside the triangle
    // barycentric coordinate from tetrahedron volumes
    float U = glm::dot( cross( d2, d0 ), Ng );
    float V = glm::dot( cross( d0, d1 ), Ng );
    float W = glm::dot( cross( d1, d2 ), Ng );

    // bc outside the triangle
    if( U < 0.0f )
    {
        U = 0.0f;
        V = glm::dot( -d0, e0 );
        W = glm::dot( d2, e0 );
    } 
    else if ( V < 0.0f )
    {
        V = 0.0f;
        W = glm::dot( -d1, e1 );
        U = glm::dot( d0, e1 );
    }
    else if ( W < 0.0f )
    {
        W = 0.0f;
        U = glm::dot( -d2, e2 );
        V = glm::dot( d1, e2 );
    }

    glm::vec3 bc = glm::max( glm::vec3( 0.0f ), { U, V, W } );
    return bc / ( bc.x + bc.y + bc.z );
}

int main() {
    using namespace pr;

    Config config;
    config.ScreenWidth = 1920;
    config.ScreenHeight = 1080;
    config.SwapInterval = 1;
    Initialize(config);

    Camera3D camera;
    camera.origin = { 4, 4, 4 };
    camera.lookat = { 0, 0, 0 };
    camera.zUp = false;

    double e = GetElapsedTime();

    while (pr::NextFrame() == false) {
        if (IsImGuiUsingMouse() == false) {
            UpdateCameraBlenderLike(&camera);
        }

        ClearBackground(0.1f, 0.1f, 0.1f, 1);

        BeginCamera(camera);

        PushGraphicState();

        DrawGrid(GridAxis::XZ, 1.0f, 10, { 128, 128, 128 });
        DrawXYZAxis(1.0f);

        float unit = 1.0f;
        static glm::vec3 v0 = { -unit, -unit - 0.3f, 0.0f };
        static glm::vec3 v1 = { unit, -unit, 0.0f };
        static glm::vec3 v2 = { -unit, unit, 0.0f };

        static float scatter = 2.0f;

        ManipulatePosition(camera, &v0, 1);
        ManipulatePosition(camera, &v1, 1);
        ManipulatePosition(camera, &v2, 1);

        DrawText(v0, "v0");
        DrawText(v1, "v1");
        DrawText(v2, "v2");
        DrawLine(v0, v1, { 128, 128, 128 });
        DrawLine(v1, v2, { 128, 128, 128 });
        DrawLine(v2, v0, { 128, 128, 128 });

        // Random Point
        Xoshiro128StarStar rng;
        for (int i = 0; i < 100; i++)
        {
            glm::vec3 p = {
                glm::mix( -scatter, scatter, rng.uniformf() ),
                glm::mix( -scatter, scatter, rng.uniformf() ),
                glm::mix( -scatter, scatter, rng.uniformf() )
            };
            glm::vec3 uvw = closestBarycentricCoordinateOnTriangle(v0, v1, v2, p);
            glm::vec3 close = uvw.x * v1 + uvw.y * v2 + uvw.z * v0;
            DrawSphere(p, 0.025f, { 255, 255, 255 });
            DrawSphere(close, 0.025f, { 255, 0, 0 });
            DrawLine(p, close, { 255,0,0 });
        }

        // Single Point
        static glm::vec3 P = {1, 1, 0 };
        DrawSphere(P, 0.05f, { 255, 255, 255 });
        ManipulatePosition(camera, &P, 1);
        glm::vec3 uvw = closestBarycentricCoordinateOnTriangle( v0, v1, v2, P );

        glm::vec3 location
            = uvw.x * v1 + uvw.y * v2 + uvw.z * v0;
        DrawSphere(location, 0.075f, { 255, 0, 255 });

        PopGraphicState();
        EndCamera();

        BeginImGui();

        ImGui::SetNextWindowSize({ 500, 800 }, ImGuiCond_Once);
        ImGui::Begin("Panel");
        ImGui::Text("fps = %f", GetFrameRate());
        ImGui::SliderFloat("scatter", &scatter, 0, 2);

        ImGui::End();

        EndImGui();
    }

    pr::CleanUp();
}
