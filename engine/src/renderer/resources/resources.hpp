#pragma once

#include <cassert>
#include <filesystem>
#include <memory>
#include <unordered_map>

class ResourceState {
public:
    static ResourceState* get() { return _instance; }

    static void init(const std::string& workingDirectory) {
        assert(_instance == nullptr);

        // executable is found inside parent directory bin. all resources are found in the parent directory of bin
        auto wd = std::filesystem::canonical(std::filesystem::path(workingDirectory)).parent_path().parent_path().parent_path();
        _instance = new ResourceState(std::move(wd));
    }

    std::filesystem::path getModelPath(const std::string& model = "") {
        return _workingDirectory / _modelsDirectory / model;
    }

    std::filesystem::path getShaderPath(const std::string& shader = "") {
        return _workingDirectory / _shadersDirectory / shader;
    }

    std::filesystem::path getSkyboxPath(const std::string& skybox = "") {
        return _workingDirectory / _skyboxesDirectory / skybox;
    }

    std::filesystem::path getTexturePath(const std::string& texture = "") {
        return _workingDirectory / _texturesDirectory / texture;
    }

    std::filesystem::path _workingDirectory;

private:
    explicit ResourceState(std::filesystem::path workingDirectory) : _workingDirectory{ std::move(workingDirectory) } {
    }

    inline static const std::filesystem::path _resourceDirectory = "resources";
    inline static const std::filesystem::path _modelsDirectory = _resourceDirectory / "models";
    inline static const std::filesystem::path _shadersDirectory = _resourceDirectory / "shaders";
    inline static const std::filesystem::path _skyboxesDirectory = _resourceDirectory / "skyboxes";
    inline static const std::filesystem::path _texturesDirectory = _resourceDirectory / "textures";

    static inline ResourceState* _instance = nullptr;
};
