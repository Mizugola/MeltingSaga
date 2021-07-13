#include <Config/Templates/Scene.hpp>
#include <Debug/Render.hpp>
#include <Scene/Exceptions.hpp>
#include <Scene/Scene.hpp>
#include <Script/ViliLuaBridge.hpp>
#include <System/Window.hpp>
#include <Utils/MathUtils.hpp>
#include <Utils/StringUtils.hpp>

#include <vili/node.hpp>
#include <vili/parser.hpp>

namespace obe::Scene
{
    void Scene::_reorganizeLayers()
    {
        m_renderCache.clear();
        for (const auto& sprite : m_spriteArray)
        {
            m_renderCache.push_back(sprite.get());
        }
        if (m_tiles)
        {
            std::vector<Graphics::Renderable*> tileLayers = m_tiles->getRenderables();
            m_renderCache.insert(
                m_renderCache.end(), tileLayers.begin(), tileLayers.end());
        }

        std::sort(m_renderCache.begin(), m_renderCache.end(),
            [](const auto& renderable1, const auto& renderable2)
            {
                if (renderable1->getLayer() == renderable2->getLayer())
                {
                    return renderable1->getZDepth() > renderable2->getZDepth();
                }
                else
                {
                    return renderable1->getLayer() > renderable2->getLayer();
                }
            });
        m_sortRenderables = false;
    }

    void Scene::_rebuildIds()
    {
        m_spriteIds.clear();
        m_colliderIds.clear();
        m_gameObjectIds.clear();
        for (auto& item : m_spriteArray)
        {
            m_spriteIds.emplace(item->getId());
        }
        for (auto& item : m_colliderArray)
        {
            m_colliderIds.emplace(item->getId());
        }
        for (auto& item : m_gameObjectArray)
        {
            m_gameObjectIds.emplace(item->getId());
        }
    }

    Scene::Scene(Event::EventNamespace& eventNamespace, sol::state_view lua)
        : m_lua(lua)
        , e_scene(eventNamespace.createGroup("Scene"))

    {
        e_scene->add<Events::Scene::Loaded>();
    }

    void Scene::attachResourceManager(Engine::ResourceManager& resources)
    {
        m_resources = &resources;
    }

    Graphics::Sprite& Scene::createSprite(const std::string& id, bool addToSceneRoot)
    {
        std::string createId = id;
        if (createId.empty())
        {
            int i = 0;
            std::string testId = "sprite" + std::to_string(this->getSpriteAmount() + i);
            while (this->doesSpriteExists(testId))
            {
                testId = "sprite" + std::to_string(this->getSpriteAmount() + i++);
            }
            createId = testId;
        }
        if (!this->doesSpriteExists(createId))
        {
            std::unique_ptr<Graphics::Sprite> newSprite
                = std::make_unique<Graphics::Sprite>(createId);
            if (m_resources)
                newSprite->attachResourceManager(*m_resources);

            Graphics::Sprite* returnSprite = newSprite.get();
            m_spriteArray.push_back(move(newSprite));
            m_spriteIds.insert(createId);

            if (addToSceneRoot)
                m_sceneRoot.addChild(*returnSprite);

            this->reorganizeLayers();
            return *returnSprite;
        }
        else
        {
            Debug::Log->warn("<Scene> Sprite '{0}' already exists !", createId);
            return this->getSprite(createId);
        }
    }

    Collision::PolygonalCollider& Scene::createCollider(
        const std::string& id, bool addToSceneRoot)
    {
        std::string createId = id;
        if (createId.empty())
        {
            int i = 0;
            std::string testId
                = "collider" + std::to_string(this->getColliderAmount() + i);
            while (this->doesColliderExists(testId))
            {
                testId = "collider" + std::to_string(this->getColliderAmount() + i++);
            }
            createId = testId;
        }
        if (!this->doesColliderExists(createId))
        {
            m_colliderArray.push_back(
                std::make_unique<Collision::PolygonalCollider>(createId));
            m_colliderIds.insert(createId);
            if (addToSceneRoot)
                m_sceneRoot.addChild(*m_colliderArray.back());
            return *m_colliderArray.back().get();
        }
        else
        {
            Debug::Log->warn("<Scene> Collider '{0}' already exists !", createId);
            return this->getCollider(createId);
        }
    }

    std::size_t Scene::getColliderAmount() const
    {
        return m_colliderArray.size();
    }

    std::string Scene::getFilePath() const
    {
        return m_baseFolder;
    }

    void Scene::reload()
    {
        Debug::Log->debug("<Scene> Reloading Scene");
        m_futureLoad = m_levelFileName;
    }

    void Scene::reload(const OnSceneLoadCallback& callback)
    {
        Debug::Log->debug("<Scene> Reloading Scene");
        m_futureLoad = m_levelFileName;
        m_onLoadCallback = callback;
    }

    void Scene::loadFromFile(const std::string& path)
    {
        Debug::Log->debug("<Scene> Loading Scene from map file : '{0}'", path);
        this->clear();
        Debug::Log->debug("<Scene> Cleared Scene");

        m_levelFileName = path;
        vili::node sceneFile = vili::parser::from_file(
            System::Path(path).find(), Config::Templates::getSceneTemplates());
        this->load(sceneFile);
    }

    void Scene::setFutureLoadFromFile(const std::string& path)
    {
        m_futureLoad = path;
    }

    void Scene::setFutureLoadFromFile(
        const std::string& path, const OnSceneLoadCallback& callback)
    {
        m_futureLoad = path;
        m_onLoadCallback = callback;
    }

    void Scene::clear()
    {
        if (m_resources)
        {
            m_resources->clean();
        }
        for (auto it = m_gameObjectArray.rbegin(); it != m_gameObjectArray.rend(); ++it)
        {
            Script::GameObject* gameObject = it->get();
            if (!gameObject->isPermanent())
            {
                Debug::Log->debug("<Scene> Deleting GameObject {0}", gameObject->getId());
                gameObject->deleteObject();
            }
        }
        for (auto& gameObject : m_gameObjectArray) { }
        Debug::Log->debug("<Scene> Cleaning GameObject Array");
        m_gameObjectArray.erase(
            std::remove_if(m_gameObjectArray.begin(), m_gameObjectArray.end(),
                [](const std::unique_ptr<Script::GameObject>& ptr)
                { return (!ptr->isPermanent()); }),
            m_gameObjectArray.end());
        // Required for the next doesGameObjectExists
        this->_rebuildIds();
        Debug::Log->debug("<Scene> Cleaning Sprite Array");
        m_spriteArray.erase(std::remove_if(m_spriteArray.begin(), m_spriteArray.end(),
                                [this](const std::unique_ptr<Graphics::Sprite>& ptr)
                                {
                                    if (!ptr->getParentId().empty()
                                        && this->doesGameObjectExists(ptr->getParentId()))
                                        return false;
                                    return true;
                                }),
            m_spriteArray.end());
        Debug::Log->debug("<Scene> Cleaning Sprite Array");
        m_colliderArray.erase(
            std::remove_if(m_colliderArray.begin(), m_colliderArray.end(),
                [this](const std::unique_ptr<Collision::PolygonalCollider>& ptr)
                {
                    if (!ptr->getParentId().empty()
                        && this->doesGameObjectExists(ptr->getParentId()))
                        return false;
                    return true;
                }),
            m_colliderArray.end());
        Debug::Log->debug("<Scene> Clearing MapScript Array");
        m_scriptArray.clear();
        Debug::Log->debug("<Scene> Scene Cleared !");
        this->_rebuildIds();
        if (m_tiles)
            m_tiles->clear();
    }

    vili::node Scene::dump() const
    {
        vili::node result = vili::object {};

        // Meta
        result["Meta"] = vili::object { { "name", m_levelName } };

        // View
        result["View"] = vili::object {};
        result["View"]["size"] = m_camera.getSize().y / 2;
        result["View"]["position"] = vili::object { { "x", m_cameraInitialPosition.x },
            { "y", m_cameraInitialPosition.y },
            { "unit", unitsToString(m_cameraInitialPosition.unit) } };
        result["View"]["referential"] = m_cameraInitialReferential.toString("{}");

        // Sprites
        if (!m_spriteArray.empty())
            result["Sprites"] = vili::object {};
        for (auto& sprite : m_spriteArray)
        {
            if (sprite->getParentId().empty())
            {
                result["Sprites"][sprite->getId()] = sprite->dump();
            }
        }

        // Collisions
        if (!m_colliderArray.empty())
            result["Collisions"] = vili::object {};
        for (auto& collider : m_colliderArray)
        {
            if (collider->getParentId().empty())
            {
                result["Collisions"][collider->getId()] = collider->dump();
            }
        }

        // GameObjects
        if (!m_gameObjectArray.empty())
            result["GameObjects"] = vili::object {};
        for (auto& gameObject : m_gameObjectArray)
        {
            result["GameObjects"][gameObject->getId()] = gameObject->dump();
        }

        // Scripts
        if (!m_scriptArray.empty())
        {
            result["Script"] = vili::object {};
            if (m_scriptArray.size() == 1)
            {
                result.at("Script")["source"] = m_scriptArray[0];
            }
            else
            {
                result.at("Script")["sources"]
                    = vili::array(m_scriptArray.begin(), m_scriptArray.end());
            }
        }
        return result;
    }

    void Scene::load(const vili::node& data)
    {
        if (data.contains("Meta"))
        {
            const vili::node& meta = data.at("Meta");
            m_levelName = meta.at("name");
        }
        else
            throw Exceptions::MissingSceneFileBlock(m_levelFileName, "Meta", EXC_INFO);

        if (data.contains("View"))
        {
            const vili::node& view = data.at("View");
            m_camera.setSize(view.at("size"));
            double x = 0.f;
            double y = 0.f;
            Transform::Units unit = Transform::Units::SceneUnits;
            if (view.contains("position"))
            {
                const vili::node& position = view.at("position");
                if (position.contains("x"))
                {
                    x = position["x"];
                }
                if (position.contains("y"))
                {
                    y = position["y"];
                }
                if (position.contains("unit"))
                {
                    unit = Transform::stringToUnits(position.at("unit"));
                }
            }
            m_cameraInitialPosition = Transform::UnitVector(x, y, unit);
            m_cameraInitialReferential = Transform::Referential::TopLeft;
            if (view.contains("referential"))
            {
                m_cameraInitialReferential
                    = Transform::Referential::FromString(view.at("referential"));
            }
            Debug::Log->debug("<Scene> Set Camera Position at : {0}, {1} using "
                              "Referential {2}",
                m_cameraInitialPosition.x, m_cameraInitialPosition.y,
                m_cameraInitialReferential.toString());
            m_camera.setPosition(m_cameraInitialPosition, m_cameraInitialReferential);
        }
        else
            throw Exceptions::MissingSceneFileBlock(m_levelFileName, "View", EXC_INFO);

        if (data.contains("Sprites"))
        {
            const vili::node& sprites = data.at("Sprites");
            m_spriteArray.reserve(sprites.size());
            m_spriteIds.reserve(sprites.size());
            for (auto [spriteId, sprite] : data.at("Sprites").items())
            {
                this->createSprite(spriteId).load(sprite);
            }
        }

        if (data.contains("Collisions"))
        {
            const vili::node& collisions = data.at("Collisions");
            m_colliderArray.reserve(collisions.size());
            m_colliderIds.reserve(collisions.size());
            for (auto [collisionId, collision] : data.at("Collisions").items())
            {
                this->createCollider(collisionId).load(collision);
            }
        }

        if (data.contains("GameObjects"))
        {
            const vili::node& gameObjects = data.at("GameObjects");
            m_gameObjectArray.reserve(gameObjects.size());
            m_gameObjectIds.reserve(gameObjects.size());
            for (auto [gameObjectId, gameObject] : gameObjects.items())
            {
                if (!this->doesGameObjectExists(gameObjectId))
                {
                    const std::string gameObjectType = gameObject.at("type");
                    Script::GameObject& newObject
                        = this->createGameObject(gameObjectType, gameObjectId);
                    if (gameObject.contains("Requires")
                        && newObject.doesHaveScriptEngine())
                    {
                        const vili::node& objectRequirements = gameObject.at("Requires");
                        Script::GameObjectDatabase::ApplyRequirements(
                            newObject.getEnvironment(), objectRequirements);
                    }
                }
                else if (!this->getGameObject(gameObjectId).isPermanent())
                {
                    throw Exceptions::GameObjectAlreadyExists(m_levelFileName,
                        this->getGameObject(gameObjectId).getType(), gameObjectId,
                        EXC_INFO);
                }
            }
        }

        if (data.contains("Script"))
        {
            const vili::node& script = data.at("Script");
            if (script.contains("source"))
            {
                std::string source = System::Path(script.at("source")).find();
                const sol::protected_function_result result
                    = m_lua.safe_script_file(source, &sol::script_pass_on_error);
                if (!result.valid())
                {
                    const auto errObj = result.get<sol::error>();
                    const std::string errMsg = errObj.what();
                    throw Exceptions::SceneScriptLoadingError(m_levelFileName, source,
                        Utils::String::replace(errMsg, "\n", "\n        "), EXC_INFO);
                }
                m_scriptArray.push_back(script.at("source"));
            }
            else if (script.contains("sources"))
            {
                for (const vili::node& scriptName : script.at("sources"))
                {
                    m_lua.safe_script_file(System::Path(scriptName).find());
                    m_scriptArray.push_back(scriptName);
                }
            }
        }

        if (data.contains("Tiles"))
        {
            m_tiles = std::make_unique<Tiles::TileScene>(*this);
            m_tiles->load(data.at("Tiles"));
        }

        this->reorganizeLayers();

        e_scene->trigger(Events::Scene::Loaded { m_levelFileName });
    }

    void Scene::update()
    {
        if (!m_futureLoad.empty())
        {
            const std::string futureLoadBuffer = std::move(m_futureLoad);
            const std::string currentScene = m_levelFileName;
            this->loadFromFile(futureLoadBuffer);
            if (m_onLoadCallback)
            {
                sol::protected_function_result result
                    = m_onLoadCallback(futureLoadBuffer);
                if (!result.valid())
                {
                    const auto error = result.get<sol::error>();
                    const std::string errMsg = "\n        \""
                        + Utils::String::replace(error.what(), "\n", "\n        ") + "\"";
                    throw Exceptions::SceneOnLoadCallbackError(
                        currentScene, futureLoadBuffer, errMsg, EXC_INFO);
                }
            }
        }
        if (m_updateState)
        {
            const size_t arraySize = m_gameObjectArray.size();
            for (size_t i = 0; i < arraySize; i++)
            {
                Script::GameObject& gameObject = *m_gameObjectArray[i];
                if (!gameObject.deletable)
                    gameObject.update();
            }
            m_gameObjectArray.erase(
                std::remove_if(m_gameObjectArray.begin(), m_gameObjectArray.end(),
                    [this](const std::unique_ptr<Script::GameObject>& ptr)
                    {
                        if (ptr->deletable)
                        {
                            Debug::Log->debug(
                                "<Scene> Removing GameObject {}", ptr->getId());
                            if (ptr->m_sprite)
                                this->removeSprite(ptr->getSprite().getId());
                            if (ptr->m_collider)
                                this->removeCollider(ptr->getCollider().getId());
                            return true;
                        }
                        return false;
                    }),
                m_gameObjectArray.end());
            if (m_tiles)
                m_tiles->update();
        }
    }

    void Scene::draw(Graphics::RenderTarget surface)
    {
        /*for (auto it = m_spriteArray.begin(); it != m_spriteArray.end(); ++it)
        {
            if (it->get()->m_layerChanged)
            {
                for (auto it2 = it; it2 != m_spriteArray.end(); ++it2)
                {
                    it2->get()->m_layerChanged = false;
                }
                this->reorganizeLayers();
                break;
            }
        }*/

        this->_reorganizeLayers();
        if (m_renderOptions.sprites)
        {
            for (const auto& renderable : m_renderCache)
            {
                if (renderable->isVisible())
                {
                    renderable->draw(surface, m_camera);
                }
            }
        }

        // m_tiles->draw(surface, m_camera);

        if (m_renderOptions.collisions)
        {
            for (const auto& collider : m_colliderArray)
            {
                Debug::Render::drawPolygon(surface, *collider.get(), true, true, false,
                    false, m_camera.getPosition());
            }
        }

        if (m_renderOptions.sceneNodes)
        {
            for (auto& gameObject : m_gameObjectArray)
            {
                sf::CircleShape sceneNodeCircle;
                SceneNode& sceneNode = gameObject->getSceneNode();
                const Transform::UnitVector sceneNodePosition
                    = sceneNode.getPosition().to<Transform::Units::ViewPixels>();
                sceneNodeCircle.setPosition(
                    sceneNodePosition.x - 3, sceneNodePosition.y - 3);
                if (sceneNode.isSelected())
                    sceneNodeCircle.setFillColor(sf::Color::Green);
                else
                    sceneNodeCircle.setFillColor(sf::Color::Red);
                sceneNodeCircle.setOutlineColor(sf::Color::Black);
                sceneNodeCircle.setOutlineThickness(2);
                sceneNodeCircle.setRadius(6);
                surface.draw(sceneNodeCircle);
            }
        }
    }

    std::string Scene::getLevelName() const
    {
        return m_levelName;
    }

    void Scene::setLevelName(const std::string& newName)
    {
        m_levelName = newName;
    }

    std::vector<Collision::PolygonalCollider*> Scene::getAllColliders() const
    {
        std::vector<Collision::PolygonalCollider*> allColliders;
        for (const auto& collider : m_colliderArray)
            allColliders.push_back(collider.get());
        return allColliders;
    }

    Camera& Scene::getCamera()
    {
        return m_camera;
    }

    void Scene::setUpdateState(bool state)
    {
        m_updateState = state;
        for (auto& gameObject : m_gameObjectArray)
        {
            gameObject->setState(state);
        }
    }

    Script::GameObject& Scene::getGameObject(const std::string& id)
    {
        for (auto& gameObject : m_gameObjectArray)
        {
            if (gameObject->getId() == id && !gameObject->deletable)
                return *gameObject;
        }
        std::vector<std::string> objectIds;
        objectIds.reserve(m_gameObjectArray.size());
        for (const auto& object : m_gameObjectArray)
        {
            objectIds.push_back(object->getId());
        }
        throw Exceptions::UnknownGameObject(m_levelFileName, id, objectIds, EXC_INFO);
    }

    bool Scene::doesGameObjectExists(const std::string& id)
    {
        return m_gameObjectIds.find(id) != m_gameObjectIds.end();
    }

    void Scene::removeGameObject(const std::string& id)
    {
        m_gameObjectArray.erase(
            std::remove_if(m_gameObjectArray.begin(), m_gameObjectArray.end(),
                [&id](const std::unique_ptr<Script::GameObject>& ptr)
                { return (ptr->getId() == id); }),
            m_gameObjectArray.end());
        m_gameObjectIds.erase(id);
    }

    std::vector<Script::GameObject*> Scene::getAllGameObjects(
        const std::string& objectType)
    {
        std::vector<Script::GameObject*> returnVec;
        for (auto& gameObject : m_gameObjectArray)
        {
            if (!gameObject->deletable
                && (objectType.empty() || objectType == gameObject->getType()))
                returnVec.push_back(gameObject.get());
        }
        return returnVec;
    }

    Script::GameObject& Scene::createGameObject(
        const std::string& obj, const std::string& id)
    {
        std::string useId = id;
        if (useId.empty())
        {
            while (useId.empty() || this->doesGameObjectExists(useId))
            {
                useId = obj + "_"
                    + Utils::String::getRandomKey(
                        Utils::String::Alphabet + Utils::String::Numbers, 8);
            }
        }
        else if (this->doesGameObjectExists(useId))
        {
            throw Exceptions::GameObjectAlreadyExists(
                m_levelFileName, this->getGameObject(useId).getType(), useId, EXC_INFO);
        }

        std::unique_ptr<Script::GameObject> newGameObject
            = std::make_unique<Script::GameObject>(m_lua, obj, useId);
        vili::node gameObjectData
            = Script::GameObjectDatabase::GetDefinitionForGameObject(obj);
        newGameObject->loadGameObject(*this, gameObjectData, m_resources);

        if (newGameObject->doesHaveSprite())
        {
            // const Transform::UnitVector zero(0, 0);
            // newGameObject->getSprite().setPosition(zero);
            newGameObject->getSprite().setParentId(useId);
        }

        if (newGameObject->doesHaveCollider())
        {
            newGameObject->getCollider().setParentId(useId);
        }

        m_gameObjectArray.push_back(move(newGameObject));
        m_gameObjectIds.insert(useId);

        return *m_gameObjectArray.back();
    }

    std::size_t Scene::getGameObjectAmount() const
    {
        return m_gameObjectArray.size();
    }

    void Scene::reorganizeLayers()
    {
        m_sortRenderables = true;
    }

    std::size_t Scene::getSpriteAmount() const
    {
        return m_spriteArray.size();
    }

    std::vector<Graphics::Sprite*> Scene::getAllSprites()
    {
        std::vector<Graphics::Sprite*> allSprites;
        allSprites.reserve(m_spriteArray.size());
        for (auto& sprite : m_spriteArray)
            allSprites.push_back(sprite.get());
        return allSprites;
    }

    std::vector<Graphics::Sprite*> Scene::getSpritesByLayer(const int layer)
    {
        std::vector<Graphics::Sprite*> returnLayer;

        for (const auto& sprite : m_spriteArray)
        {
            if (sprite->getLayer() == layer)
                returnLayer.push_back(sprite.get());
        }

        return returnLayer;
    }

    Graphics::Sprite* Scene::getSpriteByPosition(
        const Transform::UnitVector& position, const int layer)
    {
        std::vector<Transform::Referential> rectPts = { Transform::Referential::TopLeft,
            Transform::Referential::TopRight, Transform::Referential::BottomRight,
            Transform::Referential::BottomLeft };
        const Transform::UnitVector zeroOffset(0, 0);

        std::vector<Graphics::Sprite*> spritesOnLayer = this->getSpritesByLayer(layer);
        Transform::UnitVector camera
            = -(m_camera.getPosition().to<Transform::Units::ScenePixels>());
        for (const auto& sprite : spritesOnLayer)
        {
            Collision::PolygonalCollider positionCollider("positionCollider");
            positionCollider.addPoint(
                sprite->getPositionTransformer()(position, camera, layer));
            Collision::PolygonalCollider sprCollider("sprCollider");
            for (Transform::Referential& ref : rectPts)
            {
                sprCollider.addPoint(sprite->getPosition(ref));
            }
            if (sprCollider.doesCollide(positionCollider, zeroOffset, true))
            {
                return sprite;
            }
        }
        return nullptr;
    }

    Graphics::Sprite& Scene::getSprite(const std::string& id)
    {
        for (int i = 0; i < m_spriteArray.size(); i++)
        {
            if (m_spriteArray[i]->getId() == id)
                return *m_spriteArray[i];
        }
        std::vector<std::string> spritesIds;
        spritesIds.reserve(m_spriteArray.size());
        for (const auto& sprite : m_spriteArray)
        {
            spritesIds.push_back(sprite->getId());
        }
        throw Exceptions::UnknownSprite(m_levelFileName, id, spritesIds, EXC_INFO);
    }

    bool Scene::doesSpriteExists(const std::string& id)
    {
        return m_spriteIds.find(id) != m_spriteIds.end();
    }

    void Scene::removeSprite(const std::string& id)
    {
        Debug::Log->debug("<Scene> Removing Sprite {0}", id);
        m_spriteArray.erase(std::remove_if(m_spriteArray.begin(), m_spriteArray.end(),
                                [&id](std::unique_ptr<Graphics::Sprite>& Sprite)
                                { return (Sprite->getId() == id); }),
            m_spriteArray.end());
        m_spriteIds.erase(id);
    }

    SceneNode* Scene::getSceneNodeByPosition(const Transform::UnitVector& position) const
    {
        for (auto& gameObject : m_gameObjectArray)
        {
            const Transform::UnitVector sceneNodePosition
                = gameObject->getSceneNode().getPosition();
            const Transform::UnitVector pVec
                = position.to<Transform::Units::SceneUnits>();
            const Transform::UnitVector pTolerance
                = Transform::UnitVector(6, 6, Transform::Units::ScenePixels)
                      .to<Transform::Units::SceneUnits>();

            if (Utils::Math::isBetween(pVec.x, sceneNodePosition.x - pTolerance.x,
                    sceneNodePosition.x + pTolerance.x))
            {
                if (Utils::Math::isBetween(pVec.y, sceneNodePosition.y - pTolerance.x,
                        sceneNodePosition.y + pTolerance.y))
                    return &gameObject->getSceneNode();
            }
        }
        return nullptr;
    }

    bool Scene::hasTiles() const
    {
        return static_cast<bool>(m_tiles);
    }

    const Tiles::TileScene& Scene::getTiles() const
    {
        return *m_tiles;
    }

    SceneRenderOptions Scene::getRenderOptions() const
    {
        return m_renderOptions;
    }

    void Scene::setRenderOptions(SceneRenderOptions options)
    {
        m_renderOptions = options;
    }

    std::pair<Collision::PolygonalCollider*, int> Scene::getColliderPointByPosition(
        const Transform::UnitVector& position)
    {
        const Transform::UnitVector pPos = position.to<Transform::Units::ScenePixels>();
        const Transform::UnitVector pTolerance
            = Transform::UnitVector(6, 6, Transform::Units::ScenePixels);
        for (const auto& collider : m_colliderArray)
        {
            // TODO: Fix here
            if (auto point = collider->getPointAroundPosition(pPos, pTolerance);
                point.has_value())
            {
                return std::make_pair(collider.get(), point.value()->index);
            }
        }
        return std::pair<Collision::PolygonalCollider*, int>(nullptr, 0);
    }

    Collision::PolygonalCollider* Scene::getColliderByCentroidPosition(
        const Transform::UnitVector& position)
    {
        const Transform::UnitVector pPos = position.to<Transform::Units::ScenePixels>();
        const Transform::UnitVector pTolerance
            = Transform::UnitVector(6, 6, Transform::Units::ScenePixels);
        for (const auto& collider : m_colliderArray)
        {
            if (collider->isCentroidAroundPosition(pPos, pTolerance))
                return collider.get();
        }
        return nullptr;
    }

    Collision::PolygonalCollider& Scene::getCollider(const std::string& id)
    {
        for (const auto& collider : m_colliderArray)
        {
            if (id == collider->getId())
            {
                return *collider;
            }
        }
        std::vector<std::string> collidersIds;
        collidersIds.reserve(m_colliderArray.size());
        for (const auto& collider : m_colliderArray)
        {
            collidersIds.push_back(collider->getId());
        }
        throw Exceptions::UnknownCollider(m_levelFileName, id, collidersIds, EXC_INFO);
    }

    bool Scene::doesColliderExists(const std::string& id)
    {
        return m_colliderIds.find(id) != m_colliderIds.end();
    }

    void Scene::removeCollider(const std::string& id)
    {
        m_colliderArray.erase(
            std::remove_if(m_colliderArray.begin(), m_colliderArray.end(),
                [&id](std::unique_ptr<Collision::PolygonalCollider>& collider)
                { return (collider->getId() == id); }),
            m_colliderArray.end());
        m_colliderIds.erase(id);
    }

    SceneNode& Scene::getSceneRootNode()
    {
        return m_sceneRoot;
    }

    std::string Scene::getLevelFile() const
    {
        return m_levelFileName;
    }

    // Proxy functions
    sol::table sceneGetGameObjectProxy(Scene* self, const std::string& id)
    {
        return self->getGameObject(id).access();
    }

    sol::function sceneCreateGameObjectProxy(
        Scene* self, const std::string& obj, const std::string& id)
    {
        return self->createGameObject(obj, id).getConstructor();
    }

    sol::nested<std::vector<sol::table>> sceneGetAllGameObjectsProxy(
        Scene* self, const std::string& objectType)
    {
        std::vector<sol::table> gameObjects;

        for (const auto& gameObject : self->getAllGameObjects(objectType))
        {
            gameObjects.push_back(gameObject->access());
        }
        return gameObjects;
    }
} // namespace obe::Scene