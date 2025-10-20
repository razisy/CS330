///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp — Monitor restored; mug handle attached; inner lip added
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"
#include "ShaderManager.h"
#include "stb_image.h"
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace {
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
    m_loadedTextures = 0;
}
SceneManager::~SceneManager()
{
    m_pShaderManager = nullptr;
    delete m_basicMeshes;
    m_basicMeshes = nullptr;
}

/* ------------ textures ------------- */
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int w = 0, h = 0, ch = 0; GLuint tex = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img = stbi_load(filename, &w, &h, &ch, 0);
    if (!img) { std::cout << "Could not load image: " << filename << "\n"; return false; }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (ch == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    else if (ch == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    else { stbi_image_free(img); glBindTexture(GL_TEXTURE_2D, 0); return false; }

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_textureIDs[m_loadedTextures].ID = tex;
    m_textureIDs[m_loadedTextures].tag = tag;
    m_loadedTextures++;
    return true;
}
void SceneManager::BindGLTextures()
{
    for (int i = 0;i < m_loadedTextures;++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}
void SceneManager::DestroyGLTextures()
{
    for (int i = 0;i < m_loadedTextures;++i) glDeleteTextures(1, &m_textureIDs[i].ID);
    m_loadedTextures = 0;
}
int SceneManager::FindTextureID(std::string tag) const
{
    for (int i = 0;i < m_loadedTextures;++i) if (m_textureIDs[i].tag == tag) return (int)m_textureIDs[i].ID;
    return -1;
}
int SceneManager::FindTextureSlot(std::string tag) const
{
    for (int i = 0;i < m_loadedTextures;++i) if (m_textureIDs[i].tag == tag) return i;
    return -1;
}
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& m) const
{
    for (auto& x : m_objectMaterials) if (x.tag == tag) { m = x; return true; }
    return false;
}

/* ------------ shader helpers ------------- */
void SceneManager::SetTransformations(glm::vec3 s, float rx, float ry, float rz, glm::vec3 p)
{
    glm::mat4 S = glm::scale(s);
    glm::mat4 RX = glm::rotate(glm::radians(rx), glm::vec3(1, 0, 0));
    glm::mat4 RY = glm::rotate(glm::radians(ry), glm::vec3(0, 1, 0));
    glm::mat4 RZ = glm::rotate(glm::radians(rz), glm::vec3(0, 0, 1));
    glm::mat4 T = glm::translate(p);
    glm::mat4 M = T * RX * RY * RZ * S;
    m_pShaderManager->setMat4Value(g_ModelName, M);
}
void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    m_pShaderManager->setIntValue(g_UseTextureName, false);
    m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r, g, b, a));
}
void SceneManager::SetShaderTexture(std::string tag)
{
    m_pShaderManager->setIntValue(g_UseTextureName, true);
    int slot = FindTextureSlot(tag);
    m_pShaderManager->setSampler2DValue(g_TextureValueName, slot < 0 ? 0 : slot);
}
void SceneManager::SetTextureUVScale(float u, float v)
{
    m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}
void SceneManager::SetShaderMaterial(std::string) {}

/* ------------ scene ------------- */
void SceneManager::PrepareScene()
{
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadTorusMesh();

    CreateGLTexture("../../Utilities/textures/tilesf2.jpg", "texDesk");
    CreateGLTexture("../../Utilities/textures/stainless.jpg", "texMetal");
    CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "texGold");
    CreateGLTexture("../../Utilities/textures/knife_handle.jpg", "texKnife");

    BindGLTextures();
}

void SceneManager::RenderScene()
{
    m_pShaderManager->setIntValue(g_UseLightingName, true);

    const float deskY = 0.0f;
    const float baseHeight = 0.12f;
    const float baseTopY = deskY + baseHeight;
    const float standHeight = 0.85f;
    const float panelGap = 0.10f;

    glm::vec3 s, p; float rx = 0, ry = 0, rz = 0;

    // --- floor (tiles)
    s = { 18.0f, 1.0f, 10.5f }; p = { 0.0f, deskY, 0.0f };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderTexture("texDesk"); SetTextureUVScale(4.5f, 3.0f);
    m_basicMeshes->DrawPlaneMesh();

    // ===== Monitor back (restored) =====
    // base
    s = { 2.6f, baseHeight, 1.05f }; p = { 0.0f, deskY + baseHeight * 0.5f, -0.60f };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderTexture("texKnife"); SetTextureUVScale(1.5f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // stand
    s = { 0.60f, standHeight, 0.32f }; p = { 0.0f, baseTopY + standHeight * 0.5f, -0.60f };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderTexture("texKnife"); SetTextureUVScale(0.6f, 2.0f);
    m_basicMeshes->DrawBoxMesh();

    // panel
    s = { 7.4f, 4.2f, 0.22f };
    p = { 0.0f, baseTopY + standHeight + panelGap + s.y * 0.5f, -0.65f };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderTexture("texMetal"); SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // keyboard case
    s = { 6.2f, 0.16f, 1.72f }; p = { 0.0f, deskY + s.y * 0.5f, 0.90f };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderTexture("texMetal"); SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // key rows (small dark boxes)
    const float keyW = 0.46f, keyH = 0.10f, keyD = 0.42f;
    for (int r = 0;r < 2;++r) {
        for (int c = 0;c < 6;++c) {
            s = { keyW,keyH,keyD };
            p = { -2.0f + c * 0.78f, deskY + 0.16f + keyH * 0.5f, 1.25f + r * 0.52f };
            SetTransformations(s, 0, 0, 0, p);
            SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);
            m_basicMeshes->DrawBoxMesh();
        }
    }

    // ===== Mug =====
    const float bodyR = 1.05f;      // cup cylinder X/Z scale
    const float bodyH = 1.45f;      // cup cylinder Y scale
    const glm::vec3 mugCenter = { 6.8f, deskY + bodyH * 0.5f, 0.70f };

    // body
    s = { bodyR, bodyH, bodyR };
    SetTransformations(s, 0, 0, 0, mugCenter);
    SetShaderTexture("texGold"); SetTextureUVScale(0.6f, 0.6f);
    m_basicMeshes->DrawCylinderMesh();

    // outer rim torus
    s = { 1.12f, 1.12f, 1.12f }; rx = 90.0f;
    p = { mugCenter.x, deskY + bodyH + 0.55f, mugCenter.z };
    SetTransformations(s, rx, 0, 0, p);
    SetShaderTexture("texGold");
    m_basicMeshes->DrawTorusMesh();

    // inner lip (slightly smaller torus inside for cup look)
    s = { 0.92f, 0.92f, 0.92f }; rx = 90.0f;
    p = { mugCenter.x, deskY + bodyH + 0.45f, mugCenter.z };
    SetTransformations(s, rx, 0, 0, p);
    SetShaderTexture("texGold");
    m_basicMeshes->DrawTorusMesh();

    // "liquid" disk near the top (thin cylinder, darker gold)
    s = { 0.90f, 0.05f, 0.90f };
    p = { mugCenter.x, deskY + bodyH + 0.10f, mugCenter.z };
    SetTransformations(s, 0, 0, 0, p);
    SetShaderColor(0.72f, 0.60f, 0.32f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // handle — attach with a little overlap into the body
    {
        glm::vec3 sH = { 0.40f, 0.40f, 0.40f };   // torus size
        float tubeR = 0.25f * sH.x;           // approx tube radius
        float overlap = 0.08f;                  // how much to push into cup
        float centerOffset = bodyR + tubeR - overlap; // ensures attachment
        ry = 90.0f;

        float handleY = deskY + bodyH * 0.70f;  // slightly below rim center
        p = { mugCenter.x + centerOffset, handleY, mugCenter.z };
        SetTransformations(sH, 0, ry, 0, p);
        SetShaderTexture("texGold"); SetTextureUVScale(0.6f, 0.6f);
        m_basicMeshes->DrawTorusMesh();

        // two short "connectors" (very thin cylinders) to visually bridge
        glm::vec3 cs = { 0.10f, 0.08f, 0.10f };  // tiny stubs
        // upper connector
        p = { mugCenter.x + bodyR - 0.01f, handleY + 0.18f, mugCenter.z };
        SetTransformations(cs, 0, 0, 0, p); SetShaderTexture("texGold");
        m_basicMeshes->DrawCylinderMesh();
        // lower connector
        p = { mugCenter.x + bodyR - 0.01f, handleY - 0.18f, mugCenter.z };
        SetTransformations(cs, 0, 0, 0, p); SetShaderTexture("texGold");
        m_basicMeshes->DrawCylinderMesh();
    }
}
