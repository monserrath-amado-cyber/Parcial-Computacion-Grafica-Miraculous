#define USAR_IMGUI
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>

#ifdef USAR_IMGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

struct MeshData {
    unsigned int VAO = 0;
    unsigned int indexCount = 0;
    unsigned int textureID = 0;
};

struct StageMesh {
    unsigned int VAO = 0;
    unsigned int vertexCount = 0;
};

std::vector<MeshData> ladybugMeshes;
std::vector<MeshData> chatNoirMeshes;
std::vector<MeshData> aretesMeshes;
std::vector<MeshData> anilloMeshes;
std::vector<MeshData> pavoRealMeshes;
std::vector<MeshData> hockmothMeshes;

StageMesh plataformaMesh;
StageMesh fondoMesh;

unsigned int lightingShaderProgram = 0;
unsigned int backgroundShaderProgram = 0;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.35f, 4.3f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 450.0f;
float lastY = 450.0f;

bool menuMode = false;
bool showBezierLines = false;
bool pauseAnimation = false;
float animSpeedMultiplier = 1.0f;
float ladybugRotY = 0.0f;
float chatNoirRotY = 0.0f;
bool autoRotateCharacters = false;
glm::vec3 lightPos = glm::vec3(1.2f, 2.5f, 2.5f);
float specularStrengthUI = 0.5f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
        if (key == GLFW_KEY_M || key == GLFW_KEY_TAB) {
            menuMode = !menuMode;
            if (menuMode) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            }
        }
        if (key == GLFW_KEY_B) {
            showBezierLines = !showBezierLines;
        }
        if (key == GLFW_KEY_SPACE) {
            pauseAnimation = !pauseAnimation;
        }
        if (key == GLFW_KEY_R) {
            cameraPos = glm::vec3(0.0f, 0.35f, 4.3f);
            cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
            yaw = -90.0f;
            pitch = 0.0f;
            firstMouse = true;
        }
    }
}

static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (menuMode) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

static void processKeyboardMove(GLFWwindow* window, float deltaTime) {
    float speed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= speed * cameraFront;

    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += speed * right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= speed * right;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos += speed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos -= speed * cameraUp;
}

unsigned int createSolidColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    unsigned char pixel[4] = { r, g, b, a };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texID;
}

void loadModelGLTF(const std::string& filepath, std::vector<MeshData>& outMeshes, bool centerPivot = false) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!ret) {
        std::cout << "[ERROR] Error al cargar el modelo glTF (" << filepath << "): " << err << std::endl;
        return;
    }

    if (model.meshes.empty()) {
        std::cout << "[ERROR] El modelo no contiene mallas (meshes): " << filepath << std::endl;
        return;
    }

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());
    bool hasBounds = false;

    if (centerPivot) {
        for (size_t i = 0; i < model.meshes.size(); i++) {
            const tinygltf::Mesh& mesh = model.meshes[i];
            if (mesh.primitives.empty()) continue;
            const tinygltf::Primitive& primitive = mesh.primitives[0];
            auto itPos = primitive.attributes.find("POSITION");
            if (itPos != primitive.attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[itPos->second];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const float* pos = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec3 p(pos[v * 3 + 0], pos[v * 3 + 1], pos[v * 3 + 2]);
                    minBounds = glm::min(minBounds, p);
                    maxBounds = glm::max(maxBounds, p);
                    hasBounds = true;
                }
            }
        }
    }

    glm::vec3 centerOffset = (centerPivot && hasBounds) ? (minBounds + maxBounds) * 0.5f : glm::vec3(0.0f);

    std::vector<unsigned int> loadedTextures(model.textures.size(), 0);
    for (size_t i = 0; i < model.textures.size(); i++) {
        const tinygltf::Texture& tex = model.textures[i];
        if (tex.source < 0) continue;

        const tinygltf::Image& img = model.images[tex.source];
        if (img.image.empty()) continue;

        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format = GL_RGBA;
        if (img.component == 3) format = GL_RGB;
        else if (img.component == 1) format = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        loadedTextures[i] = textureID;
    }

    for (size_t i = 0; i < model.meshes.size(); i++) {
        const tinygltf::Mesh& mesh = model.meshes[i];
        if (mesh.primitives.empty()) continue;

        const tinygltf::Primitive& primitive = mesh.primitives[0];

        const float* bufferPos = nullptr;
        const float* bufferNormal = nullptr;
        const float* bufferTexCoord = nullptr;
        int vtxCount = 0;

        auto itPos = primitive.attributes.find("POSITION");
        if (itPos != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[itPos->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            bufferPos = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
            vtxCount = accessor.count;
        }

        if (vtxCount == 0) continue;

        auto itNormal = primitive.attributes.find("NORMAL");
        if (itNormal != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[itNormal->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            bufferNormal = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
        }

        auto itTex = primitive.attributes.find("TEXCOORD_0");
        if (itTex != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[itTex->second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            bufferTexCoord = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
        }

        unsigned int VAO, VBO, EBO = 0;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        std::vector<float> vertexData;
        vertexData.resize(vtxCount * 8);
        for (int v = 0; v < vtxCount; v++) {
            vertexData[v * 8 + 0] = bufferPos[v * 3 + 0] - centerOffset.x;
            vertexData[v * 8 + 1] = bufferPos[v * 3 + 1] - centerOffset.y;
            vertexData[v * 8 + 2] = bufferPos[v * 3 + 2] - centerOffset.z;

            if (bufferNormal) {
                vertexData[v * 8 + 3] = bufferNormal[v * 3 + 0];
                vertexData[v * 8 + 4] = bufferNormal[v * 3 + 1];
                vertexData[v * 8 + 5] = bufferNormal[v * 3 + 2];
            }
            else {
                vertexData[v * 8 + 3] = 0.0f;
                vertexData[v * 8 + 4] = 1.0f;
                vertexData[v * 8 + 5] = 0.0f;
            }

            if (bufferTexCoord) {
                vertexData[v * 8 + 6] = bufferTexCoord[v * 2 + 0];
                vertexData[v * 8 + 7] = bufferTexCoord[v * 2 + 1];
            }
            else {
                vertexData[v * 8 + 6] = 0.0f;
                vertexData[v * 8 + 7] = 0.0f;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        unsigned int idxCount = 0;
        if (primitive.indices >= 0) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, accessor.count * sizeof(unsigned int),
                &(buffer.data[bufferView.byteOffset + accessor.byteOffset]), GL_STATIC_DRAW);
            idxCount = accessor.count;
        }

        glBindVertexArray(0);

        unsigned int assignedTex = 0;
        if (primitive.material >= 0 && primitive.material < model.materials.size()) {
            const tinygltf::Material& mat = model.materials[primitive.material];
            int texIdx = -1;

            if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                texIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
            }
            else if (mat.values.find("baseColorTexture") != mat.values.end()) {
                texIdx = mat.values.at("baseColorTexture").TextureIndex();
            }

            if (texIdx >= 0 && texIdx < loadedTextures.size()) {
                assignedTex = loadedTextures[texIdx];
            }
        }

        if (assignedTex == 0) {
            assignedTex = createSolidColorTexture(255, 255, 255);
        }

        outMeshes.push_back({ VAO, idxCount, assignedTex });
    }
}

StageMesh createPlatformMesh(float radius = 5.2f, int segments = 64) {
    std::vector<float> verts;
    const float pi = 3.14159265359f;

    for (int i = 0; i < segments; i++) {
        float a0 = (2.0f * pi * i) / segments;
        float a1 = (2.0f * pi * (i + 1)) / segments;

        float x0 = cos(a0) * radius;
        float z0 = sin(a0) * radius;
        float x1 = cos(a1) * radius;
        float z1 = sin(a1) * radius;

        verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.5f, 0.5f });
        verts.insert(verts.end(), { x1, 0.0f, z1,    0.0f, 1.0f, 0.0f,  0.5f + cos(a1) * 0.5f, 0.5f + sin(a1) * 0.5f });
        verts.insert(verts.end(), { x0, 0.0f, z0,    0.0f, 1.0f, 0.0f,  0.5f + cos(a0) * 0.5f, 0.5f + sin(a0) * 0.5f });
    }

    unsigned int VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return { VAO, static_cast<unsigned int>(verts.size() / 8) };
}

StageMesh createBackdropMesh() {
    float quadVerts[] = {
        -22.0f, -12.0f, -16.0f,   0.0f, 0.0f,
         22.0f, -12.0f, -16.0f,   1.0f, 0.0f,
         22.0f,  16.0f, -16.0f,   1.0f, 1.0f,

        -22.0f, -12.0f, -16.0f,   0.0f, 0.0f,
         22.0f,  16.0f, -16.0f,   1.0f, 1.0f,
        -22.0f,  16.0f, -16.0f,   0.0f, 1.0f
    };

    unsigned int VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return { VAO, 6 };
}

glm::vec3 calcularBezier(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * p0;
    p += 3.0f * uu * t * p1;
    p += 3.0f * u * tt * p2;
    p += ttt * p3;
    return p;
}

void drawBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color, unsigned int whiteTex) {
    if (!showBezierLines) return;

    std::vector<float> lineVerts;
    int segments = 48;
    for (int i = 0; i <= segments; i++) {
        float t = static_cast<float>(i) / segments;
        glm::vec3 pt = calcularBezier(t, p0, p1, p2, p3);
        lineVerts.insert(lineVerts.end(), { pt.x, pt.y, pt.z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f });
    }

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(float), lineVerts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniform3f(glGetUniformLocation(lightingShaderProgram, "uTint"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(lightingShaderProgram, "uSpecularStrength"), 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    glUniform1i(glGetUniformLocation(lightingShaderProgram, "uTex"), 0);

    glLineWidth(2.5f);
    glDrawArrays(GL_LINE_STRIP, 0, segments + 1);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void drawGLTFModel(const std::vector<MeshData>& meshes, const glm::mat4& modelMat,
    const glm::vec3& tint = glm::vec3(1.0f), float specStrength = 0.5f) {
    if (meshes.empty() || lightingShaderProgram == 0) return;

    glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniform3f(glGetUniformLocation(lightingShaderProgram, "uTint"), tint.r, tint.g, tint.b);
    glUniform1f(glGetUniformLocation(lightingShaderProgram, "uSpecularStrength"), specStrength);

    for (const auto& mesh : meshes) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        glUniform1i(glGetUniformLocation(lightingShaderProgram, "uTex"), 0);

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void drawAnimatedMiraculous(const std::vector<MeshData>& meshes, float animTime,
    float speedFactor, float phaseOffset,
    glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
    glm::vec3 scaleVec) {
    if (meshes.empty()) return;

    float t = glm::mod(animTime * speedFactor + phaseOffset, 1.0f);
    glm::vec3 pos = calcularBezier(t, p0, p1, p2, p3);
    pos.y += sin(animTime * 3.0f + phaseOffset * 6.28f) * 0.015f;

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, pos);
    modelMat = glm::rotate(modelMat, animTime * 1.5f + phaseOffset * 3.14f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, animTime * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::scale(modelMat, scaleVec);

    drawGLTFModel(meshes, modelMat, glm::vec3(1.0f), specularStrengthUI);
}

std::string loadShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[ERROR] No se pudo abrir el shader: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int compileFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vsCode = loadShaderFile(vertPath);
    std::string fsCode = loadShaderFile(fragPath);
    const char* vsSrc = vsCode.c_str();
    const char* fsSrc = fsCode.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, NULL);
    glCompileShader(fs);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

unsigned int createLightingShader() {
    return compileFromFiles("shaders/lighting.vert", "shaders/lighting.frag");
}

unsigned int createBackgroundShader() {
    return compileFromFiles("shaders/background.vert", "shaders/background.frag");
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(900, 900, "Parcial Final - Miraculous 3D", NULL, NULL);
    if (window == NULL) {
        std::cout << "Fallo al crear la ventana de GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Fallo al inicializar GLAD" << std::endl;
        return -1;
    }

#ifdef USAR_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
#endif

    glEnable(GL_DEPTH_TEST);
    lightingShaderProgram = createLightingShader();
    backgroundShaderProgram = createBackgroundShader();

    unsigned int whiteTex = createSolidColorTexture(255, 255, 255);
    plataformaMesh = createPlatformMesh(5.2f, 64);
    fondoMesh = createBackdropMesh();

    loadModelGLTF("lady_bug_-_miraculous/scene.gltf", ladybugMeshes, false);
    loadModelGLTF("chat_noir_-_miraculous/scene.gltf", chatNoirMeshes, false);

    loadModelGLTF("aretes/scene.gltf", aretesMeshes, true);
    loadModelGLTF("anillo cat noir/scene.gltf", anilloMeshes, true);
    loadModelGLTF("pavo real/scene.gltf", pavoRealMeshes, true);
    loadModelGLTF("hockmoth/scene.gltf", hockmothMeshes, true);

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    float animTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (!pauseAnimation) {
            animTime += deltaTime * animSpeedMultiplier;
        }
        if (autoRotateCharacters) {
            ladybugRotY += deltaTime * 1.2f;
            chatNoirRotY -= deltaTime * 1.2f;
        }

        processKeyboardMove(window, deltaTime);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;

        glClearColor(0.08f, 0.06f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projMat = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        if (backgroundShaderProgram != 0 && fondoMesh.VAO != 0) {
            glUseProgram(backgroundShaderProgram);
            glUniformMatrix4fv(glGetUniformLocation(backgroundShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMat));
            glUniformMatrix4fv(glGetUniformLocation(backgroundShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projMat));
            glBindVertexArray(fondoMesh.VAO);
            glDrawArrays(GL_TRIANGLES, 0, fondoMesh.vertexCount);
            glBindVertexArray(0);
        }

        if (lightingShaderProgram != 0) {
            glUseProgram(lightingShaderProgram);
            glDisable(GL_CULL_FACE);

            glUniform3f(glGetUniformLocation(lightingShaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(glGetUniformLocation(lightingShaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
            glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMat));
            glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projMat));

            if (plataformaMesh.VAO != 0) {
                glm::mat4 modelPlataforma = glm::mat4(1.0f);
                modelPlataforma = glm::translate(modelPlataforma, glm::vec3(0.0f, -0.66f, -2.0f));
                glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelPlataforma));
                glUniform3f(glGetUniformLocation(lightingShaderProgram, "uTint"), 0.22f, 0.12f, 0.34f);
                glUniform1f(glGetUniformLocation(lightingShaderProgram, "uSpecularStrength"), 0.35f);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, whiteTex);
                glUniform1i(glGetUniformLocation(lightingShaderProgram, "uTex"), 0);

                glBindVertexArray(plataformaMesh.VAO);
                glDrawArrays(GL_TRIANGLES, 0, plataformaMesh.vertexCount);
                glBindVertexArray(0);
            }

            if (!ladybugMeshes.empty()) {
                glm::mat4 modelLadybug = glm::mat4(1.0f);
                modelLadybug = glm::translate(modelLadybug, glm::vec3(0.55f, -0.50f, -2.0f));
                modelLadybug = glm::rotate(modelLadybug, ladybugRotY, glm::vec3(0.0f, 1.0f, 0.0f));
                modelLadybug = glm::scale(modelLadybug, glm::vec3(0.5f, 0.5f, 0.5f));
                drawGLTFModel(ladybugMeshes, modelLadybug, glm::vec3(1.0f), specularStrengthUI);
            }

            if (!chatNoirMeshes.empty()) {
                glm::mat4 modelChatNoir = glm::mat4(1.0f);
                modelChatNoir = glm::translate(modelChatNoir, glm::vec3(-0.55f, -0.56f, -2.0f));
                modelChatNoir = glm::rotate(modelChatNoir, chatNoirRotY, glm::vec3(0.0f, 1.0f, 0.0f));
                modelChatNoir = glm::scale(modelChatNoir, glm::vec3(0.028f, 0.028f, 0.028f));
                drawGLTFModel(chatNoirMeshes, modelChatNoir, glm::vec3(1.0f), specularStrengthUI);
            }

            glm::vec3 aretesP0(1.65f, 0.20f, -1.75f), aretesP1(2.15f, 0.60f, -1.65f),
                aretesP2(2.45f, -0.10f, -1.85f), aretesP3(1.65f, 0.20f, -1.75f);
            drawBezierCurve(aretesP0, aretesP1, aretesP2, aretesP3, glm::vec3(1.0f, 0.2f, 0.2f), whiteTex);
            drawAnimatedMiraculous(aretesMeshes, animTime, 0.4f, 0.0f,
                aretesP0, aretesP1, aretesP2, aretesP3, glm::vec3(0.035f));

            glm::vec3 anilloP0(-1.65f, 0.20f, -1.75f), anilloP1(-2.15f, 0.60f, -1.65f),
                anilloP2(-2.45f, -0.10f, -1.85f), anilloP3(-1.65f, 0.20f, -1.75f);
            drawBezierCurve(anilloP0, anilloP1, anilloP2, anilloP3, glm::vec3(0.2f, 1.0f, 0.4f), whiteTex);
            drawAnimatedMiraculous(anilloMeshes, animTime, 0.35f, 0.25f,
                anilloP0, anilloP1, anilloP2, anilloP3, glm::vec3(0.016f));

            glm::vec3 pavoP0(-2.35f, 1.20f, -1.85f), pavoP1(-1.95f, 1.55f, -1.75f),
                pavoP2(-1.55f, 1.20f, -1.85f), pavoP3(-2.35f, 1.20f, -1.85f);
            drawBezierCurve(pavoP0, pavoP1, pavoP2, pavoP3, glm::vec3(0.2f, 0.6f, 1.0f), whiteTex);
            drawAnimatedMiraculous(pavoRealMeshes, animTime, 0.32f, 0.0f,
                pavoP0, pavoP1, pavoP2, pavoP3, glm::vec3(0.14f));

            glm::vec3 mariP0(1.55f, 1.20f, -1.85f), mariP1(1.95f, 1.55f, -1.75f),
                mariP2(2.35f, 1.20f, -1.85f), mariP3(1.55f, 1.20f, -1.85f);
            drawBezierCurve(mariP0, mariP1, mariP2, mariP3, glm::vec3(0.8f, 0.3f, 1.0f), whiteTex);
            drawAnimatedMiraculous(hockmothMeshes, animTime, 0.32f, 0.50f,
                mariP0, mariP1, mariP2, mariP3, glm::vec3(0.048f));

            glUseProgram(0);
        }

#ifdef USAR_IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Panel de Control - Miraculous 3D");
        ImGui::Text("Tecla [M] o [TAB]: Activar/Desactivar Mouse");
        ImGui::Text("WASD + QE: Mover Camara | [R]: Reset Camara");
        ImGui::Separator();

        ImGui::Text("Iluminacion Phong:");
        ImGui::SliderFloat3("Posicion Luz", &lightPos.x, -5.0f, 5.0f);
        ImGui::SliderFloat("Brillo Especular", &specularStrengthUI, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::Text("Rotacion de Personajes:");
        ImGui::SliderAngle("Rotacion Ladybug", &ladybugRotY);
        ImGui::SliderAngle("Rotacion Chat Noir", &chatNoirRotY);
        ImGui::Checkbox("Auto-rotar Personajes", &autoRotateCharacters);

        ImGui::Separator();
        ImGui::Text("Curvas de Bezier:");
        ImGui::Checkbox("Mostrar Lineas Bezier [B]", &showBezierLines);
        ImGui::Checkbox("Pausar Animacion [Espacio]", &pauseAnimation);
        ImGui::SliderFloat("Velocidad Miraculous", &animSpeedMultiplier, 0.1f, 3.0f);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef USAR_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
