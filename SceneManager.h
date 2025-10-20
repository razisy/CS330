#pragma once
#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include <string>
#include <vector>

class SceneManager
{
public:
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();

    void PrepareScene();
    void RenderScene();

private:
    struct TEXTURE_INFO
    {
        std::string tag;
        unsigned int ID;
    };

    struct OBJECT_MATERIAL
    {
        float ambientStrength;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        std::string tag;
    };

    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;
    TEXTURE_INFO m_textureIDs[50];
    int m_loadedTextures;
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    bool CreateGLTexture(const char* filename, std::string tag);
    void BindGLTextures();
    void DestroyGLTextures();

    int FindTextureID(std::string tag) const;
    int FindTextureSlot(std::string tag) const;
    bool FindMaterial(std::string tag, OBJECT_MATERIAL& mat) const;

    void SetTransformations(glm::vec3 scale, float rx, float ry, float rz, glm::vec3 pos);
    void SetShaderColor(float r, float g, float b, float a);
    void SetShaderTexture(std::string tag);
    void SetTextureUVScale(float u, float v);
    void SetShaderMaterial(std::string materialTag);
};
