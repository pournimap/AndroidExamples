//
// Created by malati on 18-09-2019.
//

#ifndef DISINTEGRATE_MODEL_H
#define DISINTEGRATE_MODEL_H


#pragma once

#include <android/log.h>
#include "Mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#define LOG_TAG "MPD: Model.h "
#define LOGINFO(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
class Model
{
public:
    /*  Model Data */
    std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    std::string mypath;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.

    Model(bool gamma = false) : gammaCorrection(gamma)
    {
        //constructor
    }

    void LoadMyModel(std::string const& path)
    {

        loadModel(path);

    }

    // draws the model, and thus all its meshes
    void Draw(GLuint shaderprogramobject)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shaderprogramobject);
    }

    std::string getPath(const std::string& path)
    {
        std::string(*pathBuilder)(std::string const&) = getPathBuilder();
        return (*pathBuilder)(path);
    }
private:
    typedef std::string(*Builder) (const std::string& path);




    Builder getPathBuilder()
    {

        return &getPathRelativeBinary;
    }

    static std::string getPathRelativeBinary(const std::string& path)
    {
        return path;
    }
    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;


       // std::string sphere = "s 0 0 0 10";

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        //const aiScene* scene =  aiImportFileFromMemory(sphere.c_str(), sphere.length(), 0, ".nff");
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            LOGINFO("ERROR::ASSIMP:: %s\n", importer.GetErrorString());
            return;
        }

        LOGINFO("ASSIMP:: importer.ReadFile Successful\n");

        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));
        LOGINFO("ASSIMP:: Directory-> %s\n", directory.c_str());

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);

    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // Walk through each of the mesh's vertices

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector[0] = mesh->mVertices[i].x;
            vector[1] = mesh->mVertices[i].y;
            vector[2] = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector[0] = mesh->mNormals[i].x;
            vector[1] = mesh->mNormals[i].y;
            vector[2] = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec[0] = mesh->mTextureCoords[0][i].x;
                vec[1] = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = vec2(0.0f, 0.0f);
            // tangent
            vector[0] = mesh->mTangents[i].x;
            vector[1] = mesh->mTangents[i].y;
            vector[2] = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector[0] = mesh->mBitangents[i].x;
            vector[1] = mesh->mBitangents[i].y;
            vector[2] = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);

        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        for(unsigned int m = 0; m < scene->mNumMaterials; m++)
        {
            int textureIndex = 0;
            aiString textureFilename;
            aiReturn isTexturePresent = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE,
                                                                        textureIndex,
                                                                        &textureFilename);
            if(isTexturePresent == AI_SUCCESS) {

                LOGINFO("ASSIMP:: isTexturePresent true \n");
            } else{
                LOGINFO("ASSIMP:: isTexturePresent false\n");
            }
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
      //  std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
      //  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
       // std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
       // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
       // std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
       // textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        /*fopen_s(&gpFile, "Log.txt", "a+");
        fprintf(gpFile, "ASSIMP:: returning from processMesh\n");
        fclose(gpFile);*/
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {

        LOGINFO("ASSIMP:: GetTexture  of typeName ->  %s, mat->GetTextureCount(type) = %d\n", typeName.c_str(), mat->GetTextureCount(type));

        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            LOGINFO("ASSIMP:: GetTexture  : %s ,from typeName -> %s\n", str.C_Str(), typeName.c_str());

            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            /*for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }*/
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }

        return textures;
    }
};

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;


    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        LOGINFO("Texture load at path: -> %s, filename = %s\n", path, filename.c_str());
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        LOGINFO("Texture failed to load at path: -> %s\n", path);
        //	std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif //DISINTEGRATE_MODEL_H
